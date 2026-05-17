## 2026-05-17 - UTF-8 for Scientific Dashboards
**Learning:** Embedded web dashboards for scientific instruments often display units like degree Celsius (°C) or specific growth rate (µ). Without an explicit `<meta charset="UTF-8">` tag, these symbols may render incorrectly depending on the browser's default encoding, leading to a "broken" feel.
**Action:** Always include `<meta charset="UTF-8">` as the first tag in the `<head>` of embedded HTML headers to ensure scientific symbols render consistently across all devices.
