#include <Wire.h>
#include "MAX30105.h"
#include "spo2_algorithm.h"

MAX30105 particleSensor;

// Buffers for MAX30102 readings
#define BUFFER_SIZE 100
uint32_t irBuffer[BUFFER_SIZE];
uint32_t redBuffer[BUFFER_SIZE];

// SpO2 and Heart Rate variables
int32_t spo2;
int8_t validSPO2;
int32_t heartRate;
int8_t validHeartRate;

// Average BPM
float totalBPM = 0;
int bpmCount = 0;
float avgBPM = 0;

// SpO2 running average
#define SPO2_BUFFER_SIZE 5
int spo2Buffer[SPO2_BUFFER_SIZE] = {0};
int spo2Index = 0;
int spo2Sum = 0;
float avgSPO2 = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("Initializing MAX30102...");

  Wire.begin(21, 22); // SDA, SCL

  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println("MAX30102 not found. Check wiring.");
    while (1);
  }

  particleSensor.setup();
  particleSensor.setPulseAmplitudeRed(0x1F);
  particleSensor.setPulseAmplitudeIR(0x1F);

  Serial.println("Place your finger on the sensor...");
}

void loop() {
  // Fill the buffers
  for (byte i = 0; i < BUFFER_SIZE; i++) {
    while (!particleSensor.available()) particleSensor.check();

    redBuffer[i] = particleSensor.getRed();
    irBuffer[i] = particleSensor.getIR();
    particleSensor.nextSample();
  }

  // Calculate Heart Rate & SpO2
  maxim_heart_rate_and_oxygen_saturation(
      irBuffer,
      BUFFER_SIZE,
      redBuffer,
      &spo2,
      &validSPO2,
      &heartRate,
      &validHeartRate
  );

  // Average BPM
  if (validHeartRate && heartRate > 30 && heartRate < 220) {
    totalBPM += heartRate;
    bpmCount++;
    avgBPM = totalBPM / bpmCount;
  }

  // SpO2 running average
  if (validSPO2) {
    spo2Sum -= spo2Buffer[spo2Index]; // remove oldest
    spo2Buffer[spo2Index] = spo2;     // add new
    spo2Sum += spo2;
    spo2Index = (spo2Index + 1) % SPO2_BUFFER_SIZE;
    avgSPO2 = (float)spo2Sum / SPO2_BUFFER_SIZE;
  }

  // Print results
  Serial.print("Heart Rate: ");
  if (validHeartRate) Serial.print(heartRate);
  else Serial.print("Invalid");

  Serial.print(" bpm | Avg BPM: ");
  Serial.print(avgBPM);

  Serial.print(" | SpO2: ");
  if (validSPO2) Serial.print(avgSPO2, 1);
  else Serial.print("Invalid");

  Serial.println(" %");

  delay(1000);
}