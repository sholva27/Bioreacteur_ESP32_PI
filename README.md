# ESP32-S3 Probiotic Biofermenter

An advanced, open-source automated fermentation system optimized for probiotic culture growth. This project leverages the ESP32-S3 to provide high-precision monitoring and control of pH, Optical Density (OD), and feeding cycles. Designed for **academic research**, **pedagogical demonstrations**, and **advanced bio-production**.

## Features

- **High-Precision Sensing**: Utilizes the ADS1115 16-bit ADC for stable and accurate pH and OD readings.
- **Automated pH Control**: Dual-pump system for acid and base titration to maintain optimal pH levels.
- **Automated Feeding**: Programmable nutrient delivery cycles based on time or OD thresholds.
- **OD Monitoring**: Supports both 650nm Laser and 600nm LED configurations for biomass estimation.
- **Data Logging**: Integrated DS3231 RTC and SPIFFS persistent storage for timestamped historical data.
- **Remote Dashboard**: Asynchronous web server providing real-time telemetry and non-blocking remote control.
- **Failsafe Mechanisms**: Heartbeat monitoring, sensor health checks, and automated emergency pump shutdown.
- **OTA Updates**: Built-in support for Over-The-Air firmware updates via Wi-Fi.

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
2. **Growth Modeling**: See [research.md](research.md) for details on kinetic parameters.
3. **Firmware Upload**:
   - Install the ESP32 board support in Arduino IDE.
   - Install required libraries via Library Manager.
   - Rename `firmware/include/secrets.h.example` to `secrets.h` and configure your WiFi credentials.
   - Upload to ESP32-S3.
4. **Web Interface**: Once connected to WiFi, navigate to the ESP32's IP address in your browser.

## Optimizations and Future Work

### Implemented Optimizations
- **Time-Proportional Titration**: Uses a 10-second duty cycle for pumps where the 'ON' time is proportional to the deviation from the pH target.
- **Async Processing**: All networking and control logic is non-blocking.
- **Persistent Telemetry**: Data is logged every minute to internal SPIFFS.
- **OTA Support**: Firmware can be updated wirelessly using the Arduino IDE or PlatformIO.

### Suggested Future Improvements (Inspired by Open-Source Community)
Drawing from projects like **Pioreactor**, **Chi.Bio**, and **BREAD**, the following upgrades are proposed to reach professional laboratory standards:

1.  **Multi-Wavelength Optical Density (Chi.Bio Inspired)**: Implement dual or triple-wavelength OD sensing (e.g., 600nm for biomass and 750nm for debris compensation) using spectral sensors.
2.  **Automated Calibration Routines (Pioreactor Inspired)**: Develop a software wizard in the web UI for multi-point pH calibration and peristaltic pump volume calibration.
3.  **Modular Hardware "Slices" (BREAD Inspired)**: Redesign the PCB to support pluggable modules (Slices) for different sensors, allowing for easy expansion without rewiring.
4.  **Metabolic Activity Detection (Respiration)**: Integration of CO2/O2 exhaust gas analysis to estimate real-time metabolic flux and oxygen uptake rate (OUR).
5.  **Bacteriocin Monitoring & Harvesting**: Automated sampling and fraction collection systems using a 4th or 5th peristaltic pump for studying antimicrobial peptides.
6.  **MQTT/SCADA Integration**: Full support for MQTT protocol to allow integration into SCADA systems for industrial-scale monitoring.
7.  **Oxygen Control (DO)**: Active regulation of Dissolved Oxygen through variable-speed aeration or O2 injection.

## Inspirational Open-Source Projects

For further inspiration and alternative architectures, check out these excellent open-source bioreactor projects:

- **[Pioreactor](https://github.com/Pioreactor/pioreactor)**: A highly polished, Raspberry Pi-based bioreactor platform with a rich Python API and extensive plugin ecosystem.
- **[BREAD (Broadly Reconfigurable and Expandable Automation Device)](https://github.com/FEASTorg/BREADS)**: A modular, ESP32-based hardware framework designed for low-cost laboratory automation and SCADA-like functionality.
- **[EvoluStat](https://github.com/onid-no-more/EvoluStat)**: An open-source automated continuous culture system (stat) for microbial evolution experiments.
- **[Chi.Bio](https://github.com/harrison-steel/Chi.Bio)**: An open-lab-hardware platform for biological characterization and manipulation, featuring advanced optical and thermal control.
- **[NinjaPBR](https://github.com/roblehmann/NinjaPBR)**: An open-source photobioreactor designed for studying microalgae and cyanobacteria growth.
- **[IRNAS Modular Bioreactor](https://github.com/IRNAS/newharvest-modular-bioreactor-v2)**: A modular system for muscle tissue engineering, emphasizing low-cost and ease of replication.

## License

This project is licensed under the MIT License.
