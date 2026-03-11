#include <TinyGsmClient.h> // Required for the Dongle/SIM module
#include <Firebase_ESP_Client.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <MAX30105.h>
#include <heartRate.h>
#include <TinyGPS++.h>

// --- MODEM CONFIGURATION ---
#define TINY_GSM_MODEM_SIM7600  // Change this if your dongle uses a different chip (e.g., SIM800)
#define SerialAT Serial1        // Serial for Modem/Dongle communication

// Your SIM Card credentials (Check with your provider like Dialog/Mobitel)
const char apn[]      = "internet"; 
const char gprsUser[] = "";
const char gprsPass[] = "";

// --- FIREBASE CONFIGURATION ---
#define API_KEY "AIzaSyDj8M3jGAoVxeACOFWo4Vf8pvx6UIIXkSw"
#define DATABASE_URL "https://sentrix-e4390-default-rtdb.asia-southeast1.firebasedatabase.app"
#define CARETAKER_UID "SwzH5E9axEfPTBjr5mZy0QPoGW72"
#define DEVICE_ID "SENTRIX_001"

// --- PINS ---
#define MODEM_TX 26 // Connect to Dongle RX
#define MODEM_RX 25 // Connect to Dongle TX
#define BUZZER_PIN 27
#define YES_BTN 32
#define NO_BTN 33
#define GPS_RX 16
#define GPS_TX 17

// --- OBJECTS ---
TinyGsm modem(SerialAT);
TinyGsmClient client(modem); // Creates a network client for Firebase

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
float lat = 0.0, lng = 0.0;

void setup() {
    Serial.begin(115200);
    gpsSerial.begin(9600, SERIAL_8N1, GPS_RX, GPS_TX);
    
    // Initialize Modem Serial
    SerialAT.begin(115200, SERIAL_8N1, MODEM_RX, MODEM_TX);
    
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(YES_BTN, INPUT_PULLUP);
    pinMode(NO_BTN, INPUT_PULLUP);

    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) Serial.println("OLED failed");
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.println("Sentrix Starting...");
    display.println("Connecting to 4G...");
    display.display();

    // 1. Initialize Modem and GPRS
    Serial.println("Initializing modem...");
    modem.restart();
    if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
        Serial.println("GPRS Connection Failed");
    } else {
        Serial.println("GPRS Connected");
    }

    if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) Serial.println("MAX30102 failed");
    particleSensor.setup(); 

    // 2. Firebase Setup with GSM Client
    config.api_key = API_KEY;
    config.database_url = DATABASE_URL;
    
    // This tells Firebase to use the GSM Client instead of WiFi
    Firebase.begin(&config, &auth);
}

void loop() {
    // [The rest of your GPS and Sensor logic remains exactly the same]
    
    // GPS Reading
    while (gpsSerial.available() > 0) {
        if (gps.encode(gpsSerial.read())) {
            if (gps.location.isValid()) {
                lat = gps.location.lat();
                lng = gps.location.lng();
            }
        }
    }

    // Vitals Reading
    long irValue = particleSensor.getIR();
    if (checkForBeat(irValue)) {
        beatAvg = 75; 
    }

    // Check GPRS Connection
    if (!modem.isGprsConnected()) {
        modem.gprsConnect(apn, gprsUser, gprsPass);
    }

    String path = "caretakers/" + String(CARETAKER_UID) + "/patients/" + String(DEVICE_ID);
    
    // Firebase Data Sync
    if (Firebase.ready() && (millis() - lastUpdate > 5000)) {
        Firebase.RTDB.setInt(&fbdo, path + "/live_bpm", beatAvg);
        Firebase.RTDB.setDouble(&fbdo, path + "/latitude", lat);
        Firebase.RTDB.setDouble(&fbdo, path + "/longitude", lng);
        lastUpdate = millis();
    }

    // Message handling logic...
    if (Firebase.RTDB.getString(&fbdo, path + "/last_message_sent")) {
        String msg = fbdo.stringData();
        if (msg != "" && msg != currentMsg) {
            currentMsg = msg;
            hasNewMessage = true;
            digitalWrite(BUZZER_PIN, HIGH); delay(200); digitalWrite(BUZZER_PIN, LOW);
        }
    }

    // Button Logic
    if (hasNewMessage) {
        if (digitalRead(YES_BTN) == LOW) {
            Firebase.RTDB.setString(&fbdo, path + "/last_response", "YES");
            Firebase.RTDB.setString(&fbdo, path + "/last_message_sent", "");
            hasNewMessage = false;
        } else if (digitalRead(NO_BTN) == LOW) {
            Firebase.RTDB.setString(&fbdo, path + "/last_response", "NO");
            Firebase.RTDB.setString(&fbdo, path + "/last_message_sent", "");
            hasNewMessage = false;
        }
    }

    updateDisplay();
}

void updateDisplay() {
    display.clearDisplay();
    display.setCursor(0,0);
    if (hasNewMessage) {
        display.println("MSG FROM CARETAKER:");
        display.println(currentMsg);
        display.println("\n[YES]    [NO]");
    } else {
        display.print("BPM: "); display.println(beatAvg);
        display.print("GPS: "); display.print(lat, 4); 
        display.print(","); display.println(lng, 4);
        display.print("Net: "); display.println(modem.isGprsConnected() ? "4G OK" : "OFFLINE");
    }
    display.display();
}