## 2026-05-24 - [Interactive Feedback in Embedded Dashboards]
**Learning:** In resource-constrained embedded web interfaces, using blocking 'alert()' calls disrupts the user experience and is inaccessible. Implementing a non-blocking asynchronous feedback pattern (e.g., 'Saving...' -> 'Saved!') within buttons ensures a smoother interaction and allows the user to remain in the context of their work.
**Action:** Always prioritize in-place UI feedback over modal alerts for asynchronous operations like settings updates or hardware triggers.

## 2026-05-24 - [Accessibility in Scientific Dashboards]
**Learning:** Scientific and industrial dashboards often rely on dense grids of inputs and buttons. Missing 'label[for]' and 'aria-live' regions makes these interfaces nearly impossible to navigate with screen readers. Explicitly associating labels and using assertive live regions for errors is critical for safety-critical hardware interfaces.
**Action:** Ensure every input has a matching label and that hardware failsafe notifications use 'role="alert"' and 'aria-live="assertive"'.
