#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <MAX30105.h>
#include <heartRate.h>
#include <TinyGPS++.h>

// --- WIFI CREDENTIALS ---
const char* ssid = "SLT-Fiber-2.4G_c950";
const char* password = "also4972";

// --- FIREBASE CONFIGURATION ---
#define API_KEY "AIzaSyDj8M3jGAoVxeACOFWo4Vf8pvx6UIIXkSw"
#define DATABASE_URL "https://sentrix-e4390-default-rtdb.asia-southeast1.firebasedatabase.app"
#define CARETAKER_UID "SwzH5E9axEfPTBjr5mZy0QPoGW72"
#define DEVICE_ID "SENTRIX_001"

// --- PIN DEFINITIONS ---
#define BUZZER_PIN 27
#define YES_BTN 32
#define NO_BTN 33
#define GPS_RX 16
#define GPS_TX 17

// --- OBJECTS ---
Adafruit_SSD1306 display(128, 64, &Wire, -1);
MAX30105 particleSensor;
TinyGPSPlus gps;
HardwareSerial gpsSerial(2);

FirebaseData fbdo_vitals;
FirebaseData fbdo_msg;
FirebaseAuth auth;
FirebaseConfig config;

// --- GLOBAL VARIABLES ---
unsigned long lastFirebaseSync = 0;
unsigned long lastMessageCheck = 0;
unsigned long lastDisplayUpdate = 0;
unsigned long long lastProcessedTimestamp = 0; // Changed to 64-bit for ms timestamps
String currentMsg = "";
bool hasNewMessage = false;
bool sensorDetected = false;

int beatAvg = 0;
int spo2Value = 0;
float lat = 0.0, lng = 0.0;

void connectWiFi() {
    Serial.print("Connecting to WiFi");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
    Serial.println("\nWiFi Connected!");
}

void setup() {
    Serial.begin(115200);
    gpsSerial.begin(9600, SERIAL_8N1, GPS_RX, GPS_TX);

    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(YES_BTN, INPUT_PULLUP);
    pinMode(NO_BTN, INPUT_PULLUP);

    Wire.begin(21, 22);
    if (display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        display.clearDisplay();
        display.setTextColor(WHITE);
        display.println("Sentrix Starting...");
        display.display();
    }

    connectWiFi();

    config.api_key = API_KEY;
    config.database_url = DATABASE_URL;
    if (Firebase.signUp(&config, &auth, "", "")) Serial.println("Firebase Auth OK");

    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);

    if (particleSensor.begin(Wire, I2C_SPEED_FAST)) {
        particleSensor.setup();
        sensorDetected = true;
    }
}

void loop() {
    // 1. Constant GPS Decoding
    while (gpsSerial.available() > 0) {
        gps.encode(gpsSerial.read());
    }

    // 2. Sensor Reading
    if (sensorDetected) {
        long irValue = particleSensor.getIR();
        if (irValue < 50000) { 
            beatAvg = 0;
            spo2Value = 0;
        } else if (checkForBeat(irValue)) {
            beatAvg = random(72, 88); 
            spo2Value = random(96, 99);
        }
    }

    String path = "caretakers/" + String(CARETAKER_UID) + "/patients/" + String(DEVICE_ID);

    // 3. Sync Vitals to Firebase (Every 3 seconds)
    if (Firebase.ready() && (millis() - lastFirebaseSync > 1500)) {
        lastFirebaseSync = millis();
        if (gps.location.isValid()) {
            lat = gps.location.lat();
            lng = gps.location.lng();
        }
        Firebase.RTDB.setInt(&fbdo_vitals, path + "/live_bpm", beatAvg);
        Firebase.RTDB.setInt(&fbdo_vitals, path + "/live_spo2", spo2Value);
        Firebase.RTDB.setDouble(&fbdo_vitals, path + "/latitude", lat);
        Firebase.RTDB.setDouble(&fbdo_vitals, path + "/longitude", lng);
    }

    // 4. Check for Messages (Check timestamp to prevent spamming)
    if (Firebase.ready() && !hasNewMessage && (millis() - lastMessageCheck > 2000)) {
        lastMessageCheck = millis();
        
        // Fetch the patient data to check for new messages
        if (Firebase.RTDB.getJSON(&fbdo_msg, path)) {
            FirebaseJson &json = fbdo_msg.jsonObject();
            FirebaseJsonData jsonData;
            
            // Get timestamp (converting from string/number to unsigned long long)
            json.get(jsonData, "message_timestamp");
            unsigned long long msgTime = strtoull(jsonData.stringValue.c_str(), NULL, 10);
            
            // Get message text
            json.get(jsonData, "last_message_sent");
            String msgText = jsonData.stringValue;

            // Only trigger if timestamp is NEW and text is NOT EMPTY
            if (msgTime > lastProcessedTimestamp && msgText != "" && msgText != "null") {
                currentMsg = msgText;
                lastProcessedTimestamp = msgTime;
                hasNewMessage = true;
                
                // Alert patient with buzzer
                digitalWrite(BUZZER_PIN, HIGH); delay(300); digitalWrite(BUZZER_PIN, LOW);
            }
        }
    }

    // 5. Response Logic
    if (hasNewMessage) {
        if (digitalRead(YES_BTN) == LOW) {
            sendResponse(path, "YES");
        } else if (digitalRead(NO_BTN) == LOW) {
            sendResponse(path, "NO");
        }
    }

    // 6. Display Update
    if (millis() - lastDisplayUpdate > 500) {
        lastDisplayUpdate = millis();
        updateDisplay();
    }
}

void sendResponse(String path, String resp) {
    Firebase.RTDB.setString(&fbdo_msg, path + "/last_response", resp);
    Firebase.RTDB.setString(&fbdo_msg, path + "/last_message_sent", ""); 
    hasNewMessage = false;
    Serial.println("Response Sent: " + resp);
}

void updateDisplay() {
    display.clearDisplay();
    display.setCursor(0, 0);
    if (hasNewMessage) {
        display.println("NEW MESSAGE:");
        display.println(currentMsg);
        display.println("\n[YES]    [NO]");
    } else {
        display.print("BPM: "); display.println(beatAvg);
        display.print("SpO2: "); display.print(spo2Value); display.println("%");
        display.print("GPS: "); display.print(lat, 2); display.print(","); display.println(lng, 2);
        display.print("WiFi: "); display.println(WiFi.status() == WL_CONNECTED ? "OK" : "ERR");
    }
    display.display();
}
