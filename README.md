# Sentrix Patient Monitoring System (Smart Healthcare Wristband)

![Project Status](https://img.shields.io/badge/status-active-success)
![Platform](https://img.shields.io/badge/platform-IoT%20%7C%20Mobile%20%7C%20Web-blue)
![Hardware](https://img.shields.io/badge/hardware-ESP32-orange)

##  Project Overview

The **Smart Healthcare Wristband System** is an IoT-based patient monitoring solution designed to help caretakers supervise elderly and mentally ill patients in real-time. The system consists of a wearable wristband device (powered by **ESP32**) and a connected mobile/web application that allows remote monitoring, location tracking, and basic communication between the caretaker and the patient.

The system acts as a central hub, ensuring that essential health vitals and location data are synchronized and accessible during emergencies.

##  Key Features

- **Real-Time Health Monitoring**: Live tracking of patient vitals including **Heart Rate (BPM)** and **Oxygen Levels (SpO2)**.
- **Location Tracking & Geofencing**:
  - View patient's live location on a map.
  - Set a **500-meter safety zone** (geofence).
  - Receive automatic alerts if the patient exits the safe zone.
- **Caretaker Dashboard**:
  - Secure login and registration.
  - QR Code scanner to link physical wristbands to digital patient profiles.
  - Manage multiple patients and view their specific medical conditions.
- **Two-Way Communication**:
  - Caretakers can send predefined "Yes/No" checks (e.g., "Are you ok?").
  - Patients respond via physical buttons on the wristband.

## 🛠️ System Architecture

### Hardware
- **Microcontroller**: ESP32 (Wi-Fi enabled)
- **Sensors**: Heart rate/Pulse ox sensor, GPS module.
- **Interface**: OLED/LCD screen for messages, physical buttons for response.

### Database Structure
The system uses a real-time database (likely Firebase) to store data in three main nodes:

1.  **Caretakers**: Stores user profiles (UID, Name, Email).
2.  **Devices**: Registry of hardware IDs (e.g., `ESP_32`).
3.  **Patients**: The core data node linking patients to caretakers and devices.
    *   **Vitals**: `bpm`, `spo2`
    *   **Location**: `lat`, `lng`
    *   **Geofence**: `home_lat`, `home_lng`, `radius`
    *   **Communication**: Status of current messages/responses.

##  Database Schema Example

```json
{
  "patients": {
    "patient_id": 1,
    "name": "Rukshan",
    "condition": "Hypertension",
    "vitals": {
      "bpm": 78,
      "spo2": "85%"
    },
    "location": {
      "lat": 0,
      "lng": 0
    },
    "geofence": {
      "radius": 500
    }
  }
}
```

##  Getting Started

### Prerequisites
- **Hardware**: ESP32 development board, required sensors.
- **Software**: 
  - Arduino IDE (for firmware).
  - Mobile App framework (Flutter/React Native/Android Native) or Web framework.
  - Firebase Account (for Realtime Database).

### Installation
1. **Clone the repository**:
   ```bash
   git clone https://github.com/your-username/sentrix-patient-monitoring.git
   ```
2. **Firmware Setup**:
   - Open the firmware folder in Arduino IDE.
   - Install necessary libraries (FirebaseESP32, TinyGPS++, etc.).
   - Upload code to the ESP32.
3. **App Setup**:
   - Navigate to the app directory.
   - Install dependencies (e.g., `npm install` or `flutter pub get`).
   - Configure your Firebase API keys.
   - Run the application.

## Contributing
Contributions are welcome! Please read `CONTRIBUTING.md` for details on our code of conduct and the process for submitting pull requests.
