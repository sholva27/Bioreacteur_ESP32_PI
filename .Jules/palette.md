## 2026-05-19 - [Embedded Dashboard Character Encoding]
**Learning:** Embedded web dashboards in MCU firmware (like ESP32) often serve HTML as raw strings. Without an explicit `<meta charset="UTF-8">` tag, scientific symbols (e.g., °C, µ) common in biofermenter telemetry can render as broken characters (e.g., Â°C, Âµ) depending on the browser's default interpretation of the binary stream.
**Action:** Always include `<meta charset="UTF-8">` in the `<head>` of any embedded HTML dashboard to ensure precise rendering of scientific units.

## 2026-05-19 - [Non-blocking Feedback for Hardware Operations]
**Learning:** For hardware-linked operations with known durations (like a 5-second feeding pulse), synchronizing the UI button state (disabled with 'Feeding...' text) with the hardware timer provides a much more intuitive user experience than a simple 'Success' alert. It prevents redundant commands and manages user expectations of the hardware's state.
**Action:** Use `setTimeout` in JavaScript to match UI button disabled states with hardware-defined operation durations (e.g., `FEEDING_DURATION_MS`).
