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

## Light Source Options for OD600

While "OD600" strictly refers to 600nm (Amber/Orange light), different sources can be used depending on precision requirements and availability:

| Source | Wavelength | Type | Pros | Cons |
|--------|------------|------|------|------|
| **Standard Laser** | 650nm | Red Laser | Extremely cheap, pre-collimated, high intensity | Not exactly 600nm; absorption spectra may differ |
| **Precision Laser** | 635nm | Red Laser | Closer to 600nm than standard lasers | Slightly higher cost than 650nm |
| **Amber LED** | 590-610nm | LED | Matches the standard 600nm wavelength perfectly | Requires aperture/lens for collimation; lower intensity |
| **RGB LED** | Variable | LED | Flexible; can be tuned to Orange (R+G) | Low monochromaticity; spectral drift with temperature |

### Recommendations
- **For Standard Use (OD600 Proxy)**: The **650nm Laser Module** you mentioned is perfectly acceptable for tracking growth curves, as long as the same source is used for the entire experiment.
- **For Scientific Accuracy**: A **600nm Amber LED** (e.g., Kingbright or Osram) is preferred. To use it effectively, mount it inside a black 3D-printed tube with a small aperture (1-2mm) to mimic a beam.
- **For Research**: If comparing results with published literature, using a source closer to 600nm (like a 600nm LED) ensures that the extinction coefficients match standard values.

## Sensor Options for OD600

While the **OPT101** (Photodiode + Integrated Amp) is an excellent choice for its simplicity and linear response, here are other accessible and cost-effective alternatives:

| Sensor | Type | Pros | Cons |
|--------|------|------|------|
| **TSL2591** | Digital (I2C) | Very high dynamic range (600M:1), No ADC needed | Fixed gain steps can be tricky for precise OD |
| **AS7262** | Spectral (I2C) | 6-channel detection, can target 600nm specifically | Slightly more expensive, 16-bit resolution |
| **BPW34 + Op-Amp** | Analog | Extremely cheap, customizable gain | Requires external Transimpedance Amp (TIA) circuit |
| **TEMT6000** | Analog | Very common, cheap | Senses visible light, less sensitive at 600nm than BPW34 |

### Recommendations for Research/Pedagogy
- **For Simplicity**: Stick with **OPT101** as it minimizes external circuitry.
- **For Accuracy**: Use **AS7262** as it allows you to subtract background noise from other wavelengths.
- **For Low Cost**: A **BPW34** photodiode with a precision op-amp (e.g., AD8605 or even a basic TL072) provides the most flexibility for DIY builds.

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
