## 2026-06-01 - Asynchronous Feedback Pattern
**Learning:** For embedded systems where network latency can be unpredictable, replacing blocking `alert()` calls with non-blocking button feedback (Saving... -> Saved!) provides a much smoother user experience and keeps the dashboard responsive.
**Action:** Use the `provideBtnFeedback` pattern for all hardware-triggering buttons to ensure clear, accessible status updates without interrupting the user's flow.
