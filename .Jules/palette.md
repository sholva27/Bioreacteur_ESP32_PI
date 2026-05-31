# Palette's Journal

## 2026-05-31 - Initializing Journal
**Learning:** Starting the UX improvement process for the Probiotic Biofermenter project.
**Action:** Identified opportunities in button feedback, accessibility (labels/aria), and keyboard navigation.

## 2026-05-31 - Encoding scientific symbols in embedded web views
**Learning:** Embedded web dashboards served as raw strings from microcontrollers (like ESP32) can suffer from character encoding issues (e.g., '°C' appearing as 'Â°C') if the UTF-8 charset is not explicitly declared.
**Action:** Always include <meta charset="UTF-8"> in the <head> of embedded HTML headers to ensure scientific units (μ, °) render correctly across all browsers.
