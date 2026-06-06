## 2025-05-14 - Character Encoding and Accessibility in Embedded Dashboards
**Learning:** Embedded web dashboards served as C++ raw strings (like in ESP32 projects) often miss critical boilerplate such as `<meta charset="UTF-8">` and `<html lang="en">`. This leads to the corruption of scientific symbols (e.g., °C, µ, h⁻¹) and prevents screen readers from correctly identifying the document language.
**Action:** Always verify the presence of standard HTML5 meta tags and language attributes in embedded firmware dashboards to ensure scientific precision and fundamental accessibility.
