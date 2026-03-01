#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h" // This comes with the SparkFun library

MAX30105 particleSensor;

const byte RATE_SIZE = 4; // How many beats to average
byte rates[RATE_SIZE]; 
byte rateSpot = 0;
long lastBeat = 0; 
float beatsPerMinute;
int beatAvg;

void setup() {
  Serial.begin(115200);
  Serial.println("--- Vitals Monitor Starting ---");

  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println("MAX30102 not found!");
    while (1);
  }

  // Optimized settings for BPM and SpO2
  particleSensor.setup(60, 4, 2, 100, 411, 4096); 
}

void loop() {
  long irValue = particleSensor.getIR();
  long redValue = particleSensor.getRed();

  // 1. Detect a Heartbeat
  if (checkForBeat(irValue) == true) {
    long delta = millis() - lastBeat;
    lastBeat = millis();
    beatsPerMinute = 60 / (delta / 1000.0);

    if (beatsPerMinute < 255 && beatsPerMinute > 20) {
      rates[rateSpot++] = (byte)beatsPerMinute;
      rateSpot %= RATE_SIZE;
      beatAvg = 0;
      for (byte x = 0 ; x < RATE_SIZE ; x++) beatAvg += rates[x];
      beatAvg /= RATE_SIZE;
    }
  }

  // 2. Simple SpO2 Calculation (Ratio of Red to IR)
  float ratio = (float)redValue / (float)irValue;
  float spo2 = 104.0 - (17.0 * ratio);

  // 3. Print Results
  if (irValue < 50000) {
    Serial.println("Finger not detected. Please place finger.");
  } else {
    Serial.print("Avg BPM: ");
    Serial.print(beatAvg);
    Serial.print(" | SpO2: ");
    Serial.print(spo2, 1);
    Serial.println("%");
  }
}