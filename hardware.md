# Hardware Specification

## Bill of Materials (BOM)

| Component | Description | Quantity |
|-----------|-------------|----------|
| ESP32-S3-WROOM-1 | Main Microcontroller | 1 |
| ADS1115 | 16-bit 4-Channel I2C ADC | 1 |
| DS3231 | High Precision I2C RTC | 1 |
| pH Probe + Module | Industrial Grade BNC Probe | 1 |
| Photodiode | Silicon Photodiode for OD | 1 |
| 650nm Laser / 600nm LED | Light Source for OD | 1 |
| L298N / MOSFET Module | Peristaltic Pump Driver | 3 |
| Peristaltic Pump | 12V 0-100mL/min | 3 |
| Buck Converter | 12V to 5V (for sensors) | 1 |
| LDO Regulator | 5V to 3.3V (for ESP32) | 1 |

## Pinout Mapping (ESP32-S3)

| Peripheral | ESP32-S3 Pin | ADC Channel |
|------------|--------------|-------------|
| I2C SDA | GPIO 8 | - |
| I2C SCL | GPIO 9 | - |
| Pump Acid (PWM) | GPIO 10 | - |
| Pump Base (PWM) | GPIO 11 | - |
| Pump Nutrients (PWM) | GPIO 12 | - |
| Laser/LED Control | GPIO 13 | - |
| Status LED | GPIO 21 | - |

## Sensor Configuration

### ADS1115 I2C Address: 0x48

- **Channel 0**: pH Probe Signal (0-5V scaled if necessary)
- **Channel 1**: OD Photodiode Signal
- **Channel 2**: System Voltage Monitor
- **Channel 3**: Reserved

## Circuit Notes

1. **Isolation**: It is highly recommended to use an I2C isolator (like ADUM1250) for the pH probe to avoid ground loops if the system is connected to mains-powered equipment.
2. **OD Path**: The Laser/LED and Photodiode should be mounted opposite each other across a 10mm clear flow cell or tube.
3. **Power**: Ensure the 12V power supply can handle the peak current of 3 pumps simultaneously (~2-3A).
