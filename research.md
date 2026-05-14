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

### 1. Optical Redox Monitoring
By adding redox-active dyes like **Resazurin** or **Methylene Blue** to the media, the metabolic activity (respiration) of the culture can be tracked optically.
- **Resazurin Assay**: Using an RGB sensor (like TCS34725) to monitor the color change from blue (inactive) to pink (metabolically active). This provides a more sensitive measurement of health than OD alone.
- **Spectral Fingerprinting**: Using a multi-channel spectral sensor (AS7262) to monitor subtle changes in the light absorption at different wavelengths (450nm - 650nm), which can be correlated with nutrient depletion or metabolic byproduct accumulation.

### 2. Fed-batch and Chemostat Strategies
The automated nutrient pump can be programmed for specific feeding profiles:
- **Constant Feed**: Maintains a steady supply of substrate.
- **Exponential Feed**: Matches the growth rate (µ) to maintain a constant substrate concentration.
- **Chemostat**: By adding an overflow/effluent pump, the system can maintain a continuous culture at a specific steady state.

### 2. Metabolic Heat Correlation
Since the system uses a PWM-controlled heater (MK2Y), the **duty cycle** required to maintain a constant temperature can be correlated with the metabolic heat produced by the culture. This provides an indirect measurement of metabolic activity without additional sensors.

### 3. Co-culture Modeling
Study the interaction between different probiotic strains by monitoring the composite OD curve and fitting it to multi-species competition models (e.g., Lotka-Volterra).

### 4. Metabolic Optimization
Correlate µ with different feeding strategies and pH levels to find the "sweet spot" for biomass production.

### 5. Lag Phase Analysis
Study the effect of inoculum size, age, and media composition on the lag time (`lambda`).

### 6. Harvest Optimization
Use the real-time mu value to trigger harvesting exactly at the transition to the stationary phase for peak bacteriocin yield.

## Pedagogical Use
Students can use the exported CSV data to perform non-linear regression in tools like Python (SciPy), MATLAB, or R (nls package) to solve for A, mu_max, and lambda.
