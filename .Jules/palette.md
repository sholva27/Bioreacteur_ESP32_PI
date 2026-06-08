## 2026-06-08 - Embedded Dashboard Async Feedback Pattern
**Learning:** In resource-constrained embedded web servers (like ESP32), traditional form submissions or page reloads are disruptive and slow. Users often lose context or encounter "hanging" browser states. Implementing a client-side non-blocking feedback pattern (e.g., changing button text to "Saving..." -> "Saved!") significantly improves the perceived responsiveness of hardware-linked operations without increasing server load.

**Action:** Always implement a `provideBtnFeedback` helper for async fetch operations in embedded dashboards to provide immediate visual confirmation and prevent redundant clicks during slow hardware responses.
