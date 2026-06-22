## 2026-06-22 - [Dashboard Accessibility and Feedback]
**Learning:** Embedded web dashboards often lack basic accessibility (labels, ARIA) and visual feedback for async hardware operations, leading to user confusion during high-latency tasks.
**Action:** Always implement non-blocking async feedback (e.g., 'Saving...') and ensure full label-input pairing and ARIA labels for all interactive elements in embedded UIs.
