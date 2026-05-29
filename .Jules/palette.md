## 2026-05-29 - [Scientific Symbol Rendering & Async Feedback]
**Learning:** Missing `<meta charset="UTF-8">` in embedded C++ dashboards causes corruption of scientific symbols like '°C' and 'µ' (rendered as 'Â°C', 'Âµ'). Additionally, using `alert()` for simple 'Save' confirmations is disruptive to the user flow.
**Action:** Always include the UTF-8 meta tag in embedded HTML headers. Use a non-blocking feedback pattern (e.g., 'Saving...' -> 'Saved!') for simple async operations to maintain a smooth user experience.
