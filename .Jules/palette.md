## 2025-05-14 - Non-blocking Async Feedback and Error Recovery

**Learning:** In hardware-control dashboards, using asynchronous UI feedback (e.g., changing button text to "Saving...") without proper error handling can lead to "UI Deadlocks." If a network request fails and the code doesn't catch the error to reset the button state, the control becomes permanently disabled, which is a safety risk for critical systems like biofermenters.

**Action:** Always include `.catch()` blocks in fetch chains that manage button states. The catch block must revert the button to an enabled, interactive state (e.g., showing "Error!") to ensure the user retains control even after a failed operation.

## 2025-05-14 - Embedded Dashboard Metadata and Symbols

**Learning:** Embedded web dashboards often serve scientific data containing special symbols (like °C or μ). Without explicit UTF-8 declaration and language attributes, these symbols may fail to render correctly in some browsers, and screen readers may struggle with context.

**Action:** Always include `<meta charset="UTF-8">` and `<html lang="en">` in embedded HTML headers to ensure reliable symbol rendering and accessibility compliance.
