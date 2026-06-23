## 2025-05-14 - Non-blocking Asynchronous Feedback Pattern
**Learning:** In resource-constrained embedded dashboards, replacing blocking `alert()` calls with inline button state changes ("Saving...", "Saved!") significantly improves the perceived responsiveness and prevents UI blocking during network latency.
**Action:** Use the `provideBtnFeedback` pattern for all asynchronous hardware operations to ensure consistent user feedback and prevent double-submissions by disabling buttons during transit.
