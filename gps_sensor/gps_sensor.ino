#include <TinyGPS++.h>

// The TinyGPS++ object
TinyGPSPlus gps;

void setup() {
  Serial.begin(115200); // Set Serial Monitor to 115200
  Serial2.begin(9600, SERIAL_8N1, 16, 17); // GPS Connection
  
  Serial.println("--- GPS DECODING TEST ---");
  Serial.println("Waiting for Satellite Fix (Take the antenna to a window)...");
}

void loop() {
  // Feed the raw $GPGGA data into the library
  while (Serial2.available() > 0) {
    gps.encode(Serial2.read());
  }

  // If the library successfully decoded a location
  if (gps.location.isUpdated()) {
    Serial.print("LATITUDE:  "); Serial.println(gps.location.lat(), 6);
    Serial.print("LONGITUDE: "); Serial.println(gps.location.lng(), 6);
    Serial.print("SATELLITES: "); Serial.println(gps.satellites.value());
    Serial.println("---------------------------------");
  }

  // If no data is processed after 5 seconds, something is wrong
  if (millis() > 5000 && gps.charsProcessed() < 10) {
    Serial.println("Error: No GPS characters received. Check connections.");
    delay(2000);
  }
}