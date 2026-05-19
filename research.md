# Microbial Growth Modeling

This project allows for high-resolution monitoring of probiotic cultures, enabling the fitting of data to established mathematical models.

## Primary Growth Models

### 1. Gompertz Model (Modified)
The Gompertz model is widely used to describe the growth of microorganisms. It is particularly good at capturing the asymmetrical sigmoid curve.

**Equation:**
`y(t) = A * exp(-exp( (mu_max * e / A) * (lambda - t) + 1))`

- **A**: Asymptotic value (carrying capacity).
- **mu_max**: Maximum specific growth rate.
- **lambda**: Lag time duration.

### 2. Baranyi-Roberts Model
Considered more "biologically correct" than Gompertz, as it explicitly models the adjustment period (lag phase) and the transition to the stationary phase.

## Kinetic Parameters in the Firmware

The firmware estimates the **Specific Growth Rate (µ)** in real-time using:
`µ = (ln(OD_t) - ln(OD_t-dt)) / dt`

This value represents the instantaneous rate of biomass increase. During the **exponential phase**, this value will peak and stabilize at `mu_max`.

## Research Applications

### 1. NADH Fluorescence and Metabolic Vigor
Monitoring the intracellular redox state provides the most advanced layer of biological feedback in this system:
- **Biochemical Basis**: Reduced Nicotinamide Adenine Dinucleotide (NADH) is a key electron carrier in metabolism. It naturally fluoresces when excited at ~340nm, emitting light at ~460nm. NAD+ (the oxidized form) does not.
- **Metabolic Activity Indicator**: The intensity of the 460nm emission is a direct proxy for the concentration of NADH. A high NADH level typically indicates active glycolysis and strong metabolic "vigor".
- **Phase Detection**: Changes in fluorescence often signal the transition between growth phases (e.g., shifts from exponential to stationary) much faster than OD measurements, as the internal chemistry reacts before total biomass stabilizes.
- **Correction for Riboflavin (Vitamin B2)**: Lactic Acid Bacteria (LAB) media (like MRS or whey) often contains Riboflavin. This molecule fluoresces at **520nm** when excited by UV/Blue light.
    - **Strategy**: By monitoring both 460nm (NADH) and 520nm (Riboflavin), the system can distinguish between biological activity and background media noise. If the 520nm signal rises proportionally with the 460nm signal, it suggests media interference rather than metabolic flux.

### 2. Headspace Pressure and Gas Production
Many probiotic bacteria (e.g., *Lactobacillus*, *Bifidobacterium*) produce $CO_2$ or other gases as metabolic byproducts. In a sealed or semi-sealed bioreactor:
- **Metabolic Proxy**: The rate of pressure increase in the headspace (measured via **XGZP6847A**) can be directly correlated with the glucose consumption rate and biomass production.
- **Fermentation Signature**: The "Pressure Curve" often precedes the OD log-phase, providing an early warning of metabolic initiation.
- **Gas Evolution Rate (GER)**: By calculating $dP/dt$, researchers can estimate the metabolic flux of the culture in real-time, independent of optical turbidity.

### 3. Turbidity vs. OD600
For high-resolution growth tracking, understanding the difference between Transmission and Scattering is vital:
- **OD600 (Transmission)**: Measures the amount of light that *fails* to pass through the culture. It is highly accurate for low to medium densities but "saturates" (flattens out) when the culture becomes too thick for light to penetrate.
- **Turbidity (Scattering/Nephelometry)**: Measures the light that is *scattered* at an angle (usually 90°). This method is often superior for high-density probiotic cultures as it remains linear over a wider range of biomass concentrations.
- **Combined Approach**: Using the Laser/Photodiode path for early-stage growth (lag and early-log) and a dedicated Turbidity sensor for late-log and stationary phases provides a complete picture of the culture's lifecycle.

### 4. Optical Redox Monitoring
By adding redox-active dyes like **Resazurin** or **Methylene Blue** to the media, the metabolic activity (respiration) of the culture can be tracked optically.
- **Resazurin Assay**: Using an RGB sensor (like TCS34725) to monitor the color change from blue (inactive) to pink (metabolically active). This provides a more sensitive measurement of health than OD alone.
- **Spectral Fingerprinting**: Using a multi-channel spectral sensor (AS7262) to monitor subtle changes in the light absorption at different wavelengths (450nm - 650nm), which can be correlated with nutrient depletion or metabolic byproduct accumulation.

### 5. Fed-batch and Chemostat Strategies
The automated nutrient pump can be programmed for specific feeding profiles:
- **Constant Feed**: Maintains a steady supply of substrate.
- **Exponential Feed**: Matches the growth rate (µ) to maintain a constant substrate concentration.
- **Chemostat**: By adding an overflow/effluent pump, the system can maintain a continuous culture at a specific steady state.

### 6. Metabolic Heat Correlation
Since the system uses a PWM-controlled heater (MK2Y), the **duty cycle** required to maintain a constant temperature can be correlated with the metabolic heat produced by the culture. This provides an indirect measurement of metabolic activity without additional sensors.

### 7. Co-culture Modeling
Study the interaction between different probiotic strains by monitoring the composite OD curve and fitting it to multi-species competition models (e.g., Lotka-Volterra).

### 8. Metabolic Optimization
Correlate µ with different feeding strategies and pH levels to find the "sweet spot" for biomass production.

### 9. Lag Phase Analysis
Study the effect of inoculum size, age, and media composition on the lag time (`lambda`).

### 10. Harvest Optimization
Use the real-time mu value to trigger harvesting exactly at the transition to the stationary phase for peak bacteriocin yield.

## Pedagogical Use
Students can use the exported CSV data to perform non-linear regression in tools like Python (SciPy), MATLAB, or R (nls package) to solve for A, mu_max, and lambda.
