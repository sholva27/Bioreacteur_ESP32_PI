## 2025-05-14 - Button Feedback Pattern
**Learning:** Using `alert()` for success feedback is disruptive in embedded web dashboards. A non-blocking inline feedback pattern (e.g., changing button text to 'Saving...' then 'Saved!') provides a smoother experience without interrupting the user's flow.
**Action:** Replace modal alerts with temporary button text changes for asynchronous operations.

## 2025-05-14 - Form Accessibility in C++ Headers
**Learning:** When embedding HTML in C++ headers, it's easy to overlook basic accessibility like linking labels to inputs.
**Action:** Ensure every `<label>` has a `for` attribute matching the input `id`.
