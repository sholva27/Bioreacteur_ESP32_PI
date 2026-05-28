## 2025-05-14 - Embedded Dashboard UX & Accessibility

**Learning:** In embedded web interfaces (like ESP32 dashboards), asynchronous operations can often be interrupted by network instability or hardware resets. Implementing a robust feedback pattern requires matching `.catch()` blocks to ensure buttons don't stay in a disabled "Saving..." state indefinitely. Additionally, multi-step hardware procedures (e.g., pH calibration) benefit from a mix of non-blocking button feedback for the network request and blocking `alert()` calls for critical user instructions between physical steps.

**Action:** Always ensure fetch chains have `.catch()` handlers that revert button states. Preserve instructional `alert()` calls in wizards where the user must perform physical tasks (like moving a probe) before the next software step. Use explicit `id` and `for` associations for all form inputs to improve both screen-reader accessibility and touch-hit areas on mobile devices.
