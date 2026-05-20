## 2025-05-15 - Async Feedback Pattern for Embedded Dashboards
**Learning:** For embedded systems where frontend frameworks are absent, a robust UX pattern for async actions (like settings updates) involves: 1) Changing button text to "Saving...", 2) Disabling the button to prevent double-submissions, 3) Using a 2-second "Saved!" success state, and 4) Always including a .catch() block to revert the UI state on network failure.
**Action:** Apply this pattern to all POST/GET interactive elements in C++ header-based web dashboards to replace disruptive alert() calls.
