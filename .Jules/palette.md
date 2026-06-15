## 2026-06-15 - Async UI Feedback in Embedded Environments
**Learning:** In resource-constrained embedded web dashboards (like ESP32), traditional blocking alerts interrupt the user flow and don't reflect the true status of asynchronous hardware operations. Using a chained Promise-based feedback pattern (Saving... -> Saved!) improves the perceived responsiveness of the system.
**Action:** Always implement non-blocking visual feedback for hardware-linked actions and ensure the UI state (e.g., button text) is synchronized with the resolution of the underlying network fetch request.

## 2026-06-15 - Accessibility in Scientific Dashboards
**Learning:** Scientific dashboards often rely on icon-less buttons and raw data displays. Explicit label-input associations (`for` attributes) and ARIA live regions (`role="alert"`) are critical for accessibility, especially when alerting users to critical sensor failures or hardware failsafes.
**Action:** Use semantic HTML and ARIA attributes even in minimal embedded web interfaces to ensure they are usable with assistive technologies.
