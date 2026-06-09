## 2026-06-09 - Accessibility & Character Encoding
**Learning:** Embedded web dashboards served as raw strings in C++ often suffer from encoding issues if the charset is not explicitly defined, causing scientific symbols to corrupt.
**Action:** Always include <meta charset="UTF-8"> and <html lang="en"> in embedded HTML headers.

## 2026-06-09 - Non-blocking Feedback
**Learning:** Simple button feedback (e.g., 'Saving...') is preferred over blocking alerts for standard settings updates to maintain a smooth user experience.
**Action:** Implement light-weight feedback on interactive buttons for async operations.
