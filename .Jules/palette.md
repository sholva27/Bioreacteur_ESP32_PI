## 2025-05-14 - [Embedded UI Interaction Feedback]
**Learning:** In embedded systems where actions (like pump operations) have a fixed physical duration, the UI should mirror this state by disabling the trigger button and showing a progress indicator (e.g., "Feeding...") for the exact duration. This prevents redundant commands and provides clear system state visibility.
**Action:** Synchronize UI button disable-durations with firmware constants (like `FEEDING_DURATION_MS`) and provide clear visual feedback for long-running hardware tasks.
