# MAX30102 Oximeter Module

This module reads **Heart Rate** and **SpO2** using the MAX30102 sensor on ESP32.

## Required Libraries

Make sure to install these libraries in Arduino IDE before uploading:

1. **SparkFun MAX3010x Pulse and Proximity Sensor Library**  
   - For reading MAX30102 sensor data  
   - Install via Arduino IDE → Sketch → Include Library → Manage Libraries → Search “SparkFun MAX3010x”

2. **Wire** (usually pre-installed with Arduino IDE)  
   - For I2C communication with ESP32

3. **spo2_algorithm.h** (included in the SparkFun MAX3010x library)  
   - Calculates Heart Rate and SpO2 values

---

## How to Use

1. Connect MAX30102 to ESP32:

| MAX30102 | ESP32 |
|----------|-------|
| VIN      | 3.3V  |
| GND      | GND   |
| SDA      | GPIO 21 |
| SCL      | GPIO 22 |

2. Open `oximeter_sensor.ino` in Arduino IDE  
3. Upload code to ESP32  
4. Open Serial Monitor at 115200 baud  
5. Place finger on the sensor and read Heart Rate & SpO2

---

## Author

**Name:** darkKnight8008  
**Email:** akumatekina96@gmail.com
