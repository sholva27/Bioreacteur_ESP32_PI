# ESP32-S3 Probiotic Biofermenter

An advanced, open-source automated fermentation system optimized for probiotic culture growth. This project leverages the ESP32-S3 to provide high-precision monitoring and control of pH, Optical Density (OD), and feeding cycles.

## Features

- **High-Precision Sensing**: Utilizes the ADS1115 16-bit ADC for stable and accurate pH and OD readings.
- **Automated pH Control**: Dual-pump system for acid and base titration to maintain optimal pH levels.
- **Automated Feeding**: Programmable nutrient delivery cycles based on time or OD thresholds.
- **OD Monitoring**: Supports both 650nm Laser and 600nm LED configurations for biomass estimation.
- **Data Logging**: Integrated DS3231 RTC and SPIFFS persistent storage for timestamped historical data.
- **Remote Dashboard**: Asynchronous web server providing real-time telemetry and non-blocking remote control.
- **Failsafe Mechanisms**: Heartbeat monitoring, sensor health checks, and automated emergency pump shutdown.

## Hardware Components

- **MCU**: ESP32-S3-WROOM-1
- **ADC**: ADS1115 (16-bit)
- **RTC**: DS3231
- **pH Probe**: Industrial BNC probe with signal conditioner
- **OD Sensor**: 650nm Laser / 600nm LED + Photodiode
- **Actuators**: 12V Peristaltic pumps (Acid, Base, Nutrients)
- **Power**: 12V DC Input (regulated to 5V and 3.3V)

## Software Stack

- **Framework**: Arduino ESP32 Core
- **Libraries**:
  - `ESPAsyncWebServer`
  - `Adafruit_ADS1X15`
  - `RTClib`
  - `ArduinoJson`
- **Frontend**: HTML5/CSS3/JavaScript (Vanilla)

## Setup and Installation

1. **Hardware Assembly**: Refer to [hardware.md](hardware.md) for wiring diagrams and BOM.
2. **Firmware Upload**:
   - Install the ESP32 board support in Arduino IDE.
   - Install required libraries via Library Manager.
   - Rename `firmware/include/secrets.h.example` to `secrets.h` and configure your WiFi credentials.
   - Upload to ESP32-S3.
3. **Web Interface**: Once connected to WiFi, navigate to the ESP32's IP address in your browser.

## License

This project is licensed under the MIT License.
