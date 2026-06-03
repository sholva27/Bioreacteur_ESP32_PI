## 2024-06-03 - Inline Asynchronous Feedback Pattern
**Learning:** In embedded web dashboards where network latency (WiFi/ESP32) can be unpredictable, using disruptive `alert()` calls for confirmation breaks user flow. Replacing them with inline "Saving..."/"Saved!" button states provides immediate, non-blocking feedback while maintaining context.
**Action:** Implement the `provideBtnFeedback` helper in dashboard headers to manage button text transitions and disabled states during fetch operations, ensuring a consistent and accessible feedback loop.
