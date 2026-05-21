# Hardware Specification

## Bill of Materials (BOM)

| Component | Description | Quantity |
|-----------|-------------|----------|
| ESP32-S3-WROOM-1 | Main Microcontroller | 1 |
| ADS1115 | 16-bit 4-Channel I2C ADC | 1 |
| DS3231 | High Precision I2C RTC | 1 |
| TTP223B | Capacitive Touch Button Module | 1 |
| AS7262 | 6-Channel Visible Spectral Sensor | 1 |
| TCS34725 | RGB Color Sensor with IR Filter | 1 |
| pH Probe + Module | Industrial Grade BNC Probe | 1 |
| Photodiode | Silicon Photodiode for OD | 1 |
| 650nm Laser / 600nm LED | Light Source for OD | 1 |
| L298N / MOSFET Module | Peristaltic Pump Driver | 3 |
| Peristaltic Pump | 12V 0-100mL/min | 3 |
| 5V 1A Buck Regulator | DC5.5-32V Switching Regulator for logic/sensors | 1 |
| LDO Regulator | 5V to 3.3V (for ESP32) | 1 |
| 12V 20A PSU | Main Power Supply (e.g. Mean Well) | 1 |
| Fuse Holder | 12V Blade type | 2 |
| Fuses | 20A (main), 15A (heater), 5A (logic) | 1 set |
| 1N4007 Diode | Flyback protection for pumps | 4 |
| Thermal Fuse | 70°C Thermal Cutoff for safety | 1 |
| LM358 Op-Amp | Dual Op-Amp (Optional for DIY sensors) | 1 |
| RC522 RFID | Mifare Card Reader (Optional - Traceability) | 1 |
| AT24C256 | External I2C EEPROM (Optional - Settings Backup) | 1 |
| ILI9488 TFT | 4" SPI Touch Display (Optional - Local UI) | 1 |
| Turbidity Sensor | Water Turbidity Module (Optional - OD complement) | 1 |
| ADS1118 | 16-bit ADC with Temp Sensor (Alternative ADC) | 1 |
| EC11 I2C Encoder | Rotary Encoder for local menu navigation | 1 |
| 20A MOS Driver | High-current MOSFET for Heater control | 1 |
| XGZP6847A | Gas Pressure Sensor (0-40kPa) | 1 |
| UV Sensor Brick | Ultraviolet Intensity Sensor (200-370nm) | 1 |
| UV-A LED (340nm) | NADH Excitation Light Source | 1 |
| AS7341 Spectral Sensor | 11-Channel High Precision Visible Light Sensor | 1 |
| 460nm Bandpass Filter | Optical Filter for NADH Emission | 1 |
| IRLZ44N MOSFET | Logic-level MOSFET for UV LED Driver | 1 |
| 200Ω 1W Resistor | Current limiting for 340nm LED @ 12V | 1 |
| Quartz Window | UV-transparent interface for vessel | 1 |

## Pinout Mapping (ESP32-S3)

| Peripheral | ESP32-S3 Pin | ADC Channel |
|------------|--------------|-------------|
| I2C SDA | GPIO 8 | - |
| I2C SCL | GPIO 9 | - |
| Pump Acid (PWM) | GPIO 10 | - |
| Pump Base (PWM) | GPIO 11 | - |
| Pump Nutrients (PWM) | GPIO 12 | - |
| Laser/LED Control | GPIO 13 | - |
| OneWire (Temp) | GPIO 14 | - |
| Heater (PWM/MOSFET) | GPIO 15 | - |
| Stirrer (PWM) | GPIO 16 | - |
| Touch Button (TTP223B) | GPIO 17 | - |
| Status LED | GPIO 21 | - |
| Fluorescence LED | GPIO 18 | - |

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

## Heating and Stirring Solutions

### 1. Heating
For maintaining culture temperature (typically 30-45°C), the following are recommended:
- **MK2Y 200mm Round Heatbed (12V)**: Ideal for supporting the fermentation vessel. It provides a large, even heating surface.
- **MOSFET Driver**: Because the MK2Y draws ~10-12A at 12V, a high-current **External MOSFET Module** (often sold for 3D printers) or an **IRLB3034** with a heatsink is required. **Do not use small MOSFET modules.**

### 2. Stirring (Magnetic)
- **DC Motor + Magnets**: A standard 12V DC motor with a 3D-printed magnet holder.
- **Magnetic Stir Bar**: Placed inside the bioreactor vessel.
- **L298N or MOSFET**: Used to control the motor speed via PWM.

### Wiring Note
The MK2Y heatbed alone can draw ~10A. Combined with pumps and motors, ensure your 12V power supply is rated for at least **15A-20A**. Use thick gauge wire (14-16 AWG) for the heatbed power paths to prevent voltage drops and overheating.

## Temperature Sensor Options

Maintaining a stable temperature is critical for probiotic growth. While the **DS18B20** is the standard digital choice, here are other alternatives:

| Sensor | Type | Pros | Cons |
|--------|------|------|------|
| **DS18B20** | Digital (OneWire) | Waterproof versions common, No ADC needed, Multi-drop | Slower response time (~750ms for 12-bit) |
| **PT100/PT1000** | RTD (Analog) | Extremely precise, industry standard | Requires a specialized amplifier (e.g., MAX31865) |
| **NTC Thermistor** | Analog (Resistor) | Fast response, extremely cheap | Non-linear, requires calibration and voltage divider |
| **BME280 / SHT3x** | Digital (I2C) | Very high accuracy for air temp/humidity | Not naturally waterproof; hard to use in liquids |

### Recommendations
- **For Bioreactors**: The **Waterproof DS18B20** remains the best balance of cost and performance. It is easy to sanitize and simple to wire.
- **For High Precision Research**: A **PT100** with a MAX31865 breakout is recommended if you need 0.1°C accuracy and high stability.
- **For Fast Response**: Use a small **10k NTC Thermistor** if you need to detect rapid temperature changes, though it requires more complex calibration.

## Optical Metabolic Sensing Options

To detect metabolic activity beyond biomass (OD), specialized light sensors can monitor changes in chemical indicators or metabolic byproducts:

| Sensor | Purpose | Pros | Cons |
|--------|---------|------|------|
| **AS7262** | Spectral Fingerprinting | Monitors 6 wavelengths simultaneously | Higher I2C overhead |
| **TCS34725** | Colorimetric Assays | Detects color changes in redox dyes | Sensitive to ambient light interference |

### Practical Applications
- **Redox Dyes**: Using a **TCS34725** to monitor the transition of **Resazurin** (blue/non-fluorescent) to **Resorufin** (pink/highly fluorescent) as a direct proxy for microbial respiration.
- **Spectral Shift**: Using an **AS7262** to detect shifts in the absorption spectrum of the growth media caused by the secretion of specific metabolites or bacteriocins.

## DIY Signal Conditioning (LM358)

Integrating an **LM358 Dual Operational Amplifier** is highly recommended if you are building your own sensor interfaces rather than using pre-integrated modules (like the OPT101).

### 1. Transimpedance Amplifier (TIA) for OD
If using a raw photodiode (e.g., **BPW34**), the photocurrent produced is in the nanoampere/microampere range.
- **Role**: The LM358 can be configured as a TIA to convert this tiny current into a 0-3.3V or 0-5V voltage signal for the ADS1115.
- **Benefit**: Allows the use of extremely cheap photodiodes while maintaining linear response.

### 2. Unity Gain Buffer for pH
pH probes have an input impedance in the range of $10^{12} \Omega$.
- **Role**: One half of the LM358 can act as a voltage follower (buffer). This prevents the ADS1115 from "loading" the pH probe, which would cause inaccurate and drifting readings.
- **Note**: For better precision, a rail-to-rail op-amp with lower input bias current (like the **TL072** or **OPA2134**) is preferred, but the LM358 is a functional starting point available in most local kits.

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
- **Channel 2**: Headspace Pressure Signal (XGZP6847A)
- **Channel 3**: UV Intensity Signal

## Circuit Notes

1. **Isolation**: It is highly recommended to use an I2C isolator (like ADUM1250) for the pH probe to avoid ground loops if the system is connected to mains-powered equipment.
2. **OD Path**: The Laser/LED and Photodiode should be mounted opposite each other across a 10mm clear flow cell or tube.
3. **Power**: Ensure the 12V power supply can handle the peak current of 3 pumps simultaneously (~2-3A).

## Power and Protection Recommendations

### 1. Power Supply (PSU) Selection
The system requires a stable **12V DC** source. Given that the MK2Y heatbed draws ~10A and pumps/stirrer draw another 2-3A, a **20A (240W)** power supply is highly recommended.
- **Recommended Model**: **Mean Well LRS-250-12** or similar industrial-grade enclosed switching PSU.
- **Logic Power**: Use a high-efficiency **5V 1A Switching Regulator (DC 5.5-32V input)** to step down the 12V supply for sensors and the ESP32. This is significantly better than linear regulators (7805) which would waste energy and generate heat during the long fermentation cycles.
- **Noise Mitigation**: High-precision sensors (pH and OD) are sensitive to ripple. Ensure the switching regulator for the 5V rail has adequate output capacitance (e.g., 470uF Low-ESR) to stabilize the readings.

### 2. Electrical Protection
Protecting the sensitive ESP32-S3 and high-current actuators is critical for long-term fermentation (which can run for days).
- **Main Fusing**: Install a **20A Blade Fuse** or Circuit Breaker on the 12V positive rail immediately after the PSU.
- **Branch Protection**:
    - **Heater**: 15A fuse.
    - **Pumps & ESP32**: 5A fuse.
- **Reverse Polarity**: Use a high-current Schottky diode (e.g., MBR20100) or a P-Channel MOSFET protection circuit if the PSU is not already protected.

### 3. Inductive Load Protection (Flyback Diodes)
Peristaltic pumps and magnetic stirrers are inductive loads. When turned off, they generate high-voltage spikes (back-EMF).
- **Action**: Solder a **1N4007** or **UF4007** diode in parallel with each pump and the DC stirrer motor, with the cathode (stripe) towards the positive terminal. This protects your MOSFET/L298N drivers from burning out.

### 4. Thermal and Environmental Safety
- **Thermal Fuse**: For absolute safety, mount a **70°C Thermal Cutoff (TCO)** fuse in series with the heater power line, physically attached to the MK2Y heatbed. If software control fails, the TCO will permanently break the circuit.
- **Grounding**: If using a metal enclosure, ensure the PSU ground (V-) and the chassis are properly earthed (PE) to prevent static buildup and EMI.

## Advanced & Optional Modules

Integrating these components can transition the biofermenter into an industrial-grade or highly autonomous research tool.

### 1. RFID RC522 (Batch Traceability)
- **Use Case**: Attach RFID tags to fermentation vessels.
- **Benefit**: Scanning a vessel can automatically load its specific calibration curve (e.g., if different vessels have slightly different optical paths) and associate log files with a unique Batch ID.

### 2. AT24C256 EEPROM (Industrial Persistence)
- **Use Case**: **State Recovery** for long-duration fermentation. Store high-frequency data like current pump total volumes, elapsed cycle time, and log indices.
- **Benefit**:
    - **Flash Protection**: Prevents wearing out the ESP32's internal Flash memory (which has limited write cycles compared to EEPROM).
    - **Power Failure Resilience**: Allows the system to resume an experiment exactly where it left off after a power cut, without needing to re-initialize calibration or lose cumulative feeding data.

### 3. Turbidity Sensor (Complementary OD)
- **Use Case**: Measuring very high density or large-particle cultures.
- **Benefit**: Unlike the OD600 laser path (transmission), many turbidity sensors use light scattering (Nephelometry). This is less prone to saturation at high biomass levels and can detect suspended solids that might interfere with pure optical density.

### 4. ADS1118 (High-Precision + ATC)
- **Use Case**: Precision pH monitoring with Automatic Temperature Compensation.
- **Benefit**: The **ADS1118** includes an integrated temperature sensor. Since pH electrode slope varies with temperature (Nernst equation), the ADS1118 can use its internal temperature read to correct the pH value in real-time at the hardware interface level.

### 5. ILI9488 4" TFT (Local HMI)
- **Use Case**: Standalone monitoring without a smartphone/PC.
- **Benefit**: The ESP32-S3 is powerful enough to drive this 480x320 display. It allows for local graphing of pH/OD and manual pump control via the touchscreen, essential for lab environments where Wi-Fi might be restricted or unreliable.

### 6. EC11 I2C Rotary Encoder
- **Use Case**: Tactile navigation of the local TFT menu.
- **Benefit**: While touchscreens are convenient, a rotary encoder is often more reliable in wet lab environments or when wearing gloves. Using the I2C version saves GPIO pins on the ESP32-S3.

### 7. 20A MOS Driver Module
- **Use Case**: Driving the MK2Y 12V 10A heater.
- **Benefit**: These modules are designed for 3D printer heatbeds. A **20A rated MOSFET** (like those based on the D4184) ensures very low Rds(on), meaning the module stays cool while delivering the ~120W required for the heater.

### 8. XGZP6847A Gas Pressure Sensor
- **Use Case**: Monitoring fermentation gas production in the vessel headspace.
- **Benefit**: Many probiotic strains produce gases (primarily $CO_2$) during growth. Measuring pressure changes (0-40kPa range) provides a sensitive, non-invasive proxy for metabolic rate, complementary to pH and OD sensing.

### 9. NADH Fluorescence Setup (Metabolic Vigor)
- **Excitation**: 340nm UV LED.
    - **Driver**: Must use an external power source (12V) and a logic-level MOSFET (e.g., **IRLZ44N**) via GPIO 18.
    - **Current Limiting**: Use a **200Ω 1W** resistor for a 12V supply to achieve ~40mA ($R = (V_{alim} - V_f) / I$).
- **Emission Sensing**: **AS7341** spectral sensor.
- **Optical Geometry**:
    - **Front-Face Reflection**: Place the LED and sensor side-by-side (front-face) rather than at 90°. This is critical for turbid Lactic Acid Bacteria (LAB) cultures where light cannot penetrate deeply.
    - **Quartz Window**: Use quartz optics/windows; standard glass or plastic often blocks UV light at 340nm.
- **Benefit**: Monitoring NADH fluorescence (340nm excitation / 460nm emission) provides a direct look into the intracellular redox state. This allows the system to distinguish between "alive but dormant" and "metabolically active" bacteria.

### 10. UV Sensor Brick (200-370nm)
- **Use Case**: **Excitation Monitoring** and **Sterilization Safety**.
- **Utility**:
    - **NADH Verification**: Measures the actual intensity of the UV-A excitation light to normalize fluorescence readings (correcting for LED aging or power fluctuations).
    - **Sterilization**: If a UV-C lamp is used for vessel sterilization before inoculation, this sensor verifies that the lamp is functional and provides the correct dosage (within its 200nm-370nm response range).

## Local Procurement (Algeria)

For users in Algeria, most components can be sourced from:

### [Dzduino](https://www.dzduino.com)
- **ESP32-S3**: Look for "ESP32-S3 Touch LCD" or "ESP32-S3-CAM".
- **ADS1115**: Search for "Module ADC 16 Bits".
- **DS18B20**: Search for "Capteur de température étanche".
- **L298N**: Search for "Module de commande moteur".
- **MK2A**: Search for "Plaque chauffante".

### [SESDZ](https://www.sesdz.com)
- **Power Control**: Look for "Sonoff" or "Tense" modules for robust heater switching.
- **Enclosures**: Search for "Boîtier de jonction" for waterproof housing.
- **Connectors**: Search for "Connecteur d'alimentation DC".
