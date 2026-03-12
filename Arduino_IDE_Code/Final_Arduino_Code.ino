#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <MAX30105.h>
#include <heartRate.h>
#include <TinyGPS++.h>

// DONGLE CREDENTIALS
const char* ssid = "4G-UFI-XX";
const char* password = "1234567890";

// FIREBASE
#define API_KEY "AIzaSyDj8M3jGAoVxeACOFWo4Vf8pvx6UIIXkSw"
#define DATABASE_URL "https://sentrix-e4390-default-rtdb.asia-southeast1.firebasedatabase.app"
#define CARETAKER_UID "SwzH5E9axEfPTBjr5mZy0QPoGW72"
#define DEVICE_ID "SENTRIX_001"

// PINS
#define BUZZER_PIN 27
#define YES_BTN 32
#define NO_BTN 33
#define GPS_RX 16
#define GPS_TX 17

// OBJECTS
Adafruit_SSD1306 display(128, 64, &Wire, -1);
MAX30105 particleSensor;
TinyGPSPlus gps;
HardwareSerial gpsSerial(2);

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

long lastUpdate = 0;
String currentMsg = "";
bool hasNewMessage = false;

byte beatAvg = 0;
float lat = 0.0;
float lng = 0.0;

void connectWiFi()
{
  Serial.print("Connecting to WiFi");

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected!");
  Serial.println(WiFi.localIP());
}

void setup()
{
  Serial.begin(115200);

  gpsSerial.begin(9600, SERIAL_8N1, GPS_RX, GPS_TX);

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(YES_BTN, INPUT_PULLUP);
  pinMode(NO_BTN, INPUT_PULLUP);

  Wire.begin(21,22);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  {
    Serial.println("OLED Failed");
  }

  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("Sentrix Starting...");
  display.println("Connecting WiFi...");
  display.display();

  connectWiFi();

  // Firebase Setup
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  // Sensor Start
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST))
  {
    Serial.println("MAX30102 not found");
  }

  particleSensor.setup();
}

void loop()
{

  // GPS
  while (gpsSerial.available() > 0)
  {
    if (gps.encode(gpsSerial.read()))
    {
      if (gps.location.isValid())
      {
        lat = gps.location.lat();
        lng = gps.location.lng();
      }
    }
  }

  // Heart Rate
  long irValue = particleSensor.getIR();

  if (checkForBeat(irValue))
  {
    beatAvg = random(70,85); // Demo BPM
  }

  String path = "caretakers/" + String(CARETAKER_UID) + "/patients/" + String(DEVICE_ID);

  // Send data to Firebase
  if (Firebase.ready() && millis() - lastUpdate > 5000)
  {
    Firebase.RTDB.setInt(&fbdo, path + "/live_bpm", beatAvg);
    Firebase.RTDB.setDouble(&fbdo, path + "/latitude", lat);
    Firebase.RTDB.setDouble(&fbdo, path + "/longitude", lng);

    lastUpdate = millis();
  }

  // Check messages
  if (Firebase.RTDB.getString(&fbdo, path + "/last_message_sent"))
  {
    String msg = fbdo.stringData();

    if (msg != "" && msg != currentMsg)
    {
      currentMsg = msg;
      hasNewMessage = true;

      digitalWrite(BUZZER_PIN, HIGH);
      delay(200);
      digitalWrite(BUZZER_PIN, LOW);
    }
  }

  // Button Logic
  if (hasNewMessage)
  {
    if (digitalRead(YES_BTN) == LOW)
    {
      Firebase.RTDB.setString(&fbdo, path + "/last_response", "YES");
      Firebase.RTDB.setString(&fbdo, path + "/last_message_sent", "");
      hasNewMessage = false;
    }

    if (digitalRead(NO_BTN) == LOW)
    {
      Firebase.RTDB.setString(&fbdo, path + "/last_response", "NO");
      Firebase.RTDB.setString(&fbdo, path + "/last_message_sent", "");
      hasNewMessage = false;
    }
  }

  updateDisplay();
}

void updateDisplay()
{
  display.clearDisplay();
  display.setCursor(0,0);

  if(hasNewMessage)
  {
    display.println("MSG FROM CARETAKER");
    display.println(currentMsg);
    display.println("");
    display.println("[YES]    [NO]");
  }
  else
  {
    display.print("BPM: ");
    display.println(beatAvg);

    display.print("LAT: ");
    display.println(lat,4);

    display.print("LNG: ");
    display.println(lng,4);

    display.print("WiFi: ");

    if(WiFi.status()==WL_CONNECTED)
      display.println("OK");
    else
      display.println("OFF");
  }

  display.display();
}
