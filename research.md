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
- **Metabolic Optimization**: Correlate µ with different feeding strategies.
- **Lag Phase Analysis**: Study the effect of inoculum size and age on `lambda`.
- **Harvest Optimization**: Use the real-time mu value to trigger harvesting exactly at the transition to the stationary phase for peak bacteriocin yield.

## Pedagogical Use
Students can use the exported CSV data to perform non-linear regression in tools like Python (SciPy), MATLAB, or R (nls package) to solve for A, mu_max, and lambda.
