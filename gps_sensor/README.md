# GPS Sensor Module

This module reads **GPS location** using the TinyGPS++ library on ESP32.

## Required Libraries

1. **TinyGPS++**  
   - For decoding GPS NMEA sentences  
   - Install via Arduino IDE → Sketch → Include Library → Manage Libraries → Search "TinyGPS++"

2. **Wire** (pre-installed with Arduino IDE)


1. Connect GPS module to ESP32:

 GPS Pin       ESP32 Pin 
________________________
 TX       RX2 (GPIO 16) 
 RX       TX2 (GPIO 17)
 VCC      3.3V / 5V 
 GND      GND 

2. Open `gps_sensor.ino` in Arduino IDE  
3. Upload to ESP32  
4. Open Serial Monitor at 115200 baud  
5. Wait for satellite fix and read latitude/longitude
