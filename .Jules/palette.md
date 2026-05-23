## 2024-05-23 - [Asynchronous Button Feedback Pattern]
**Learning:** For embedded web dashboards where hardware actions (pumps, calibration) are slow, intrusive `alert()` blocks freeze the UI and degrade UX. Using an inline 'Saving...' -> 'Saved!'/'Error!' pattern with `disabled` states provides clear, non-blocking feedback.
**Action:** Use the `provideBtnFeedback` pattern and ensure all setting-update functions return Promises to allow correct synchronization of success/failure states.

## 2024-05-23 - [Scientific Symbol Rendering]
**Learning:** Embedded dashboards often omit the `<meta charset="UTF-8">` tag, causing scientific symbols like '°C' and 'μ' to render as mojibake (e.g., 'Â°C').
**Action:** Always include `<meta charset="UTF-8">` in the `<head>` of C++ embedded HTML strings.
