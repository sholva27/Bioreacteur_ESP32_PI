# Palette Journal - Critical UX/Accessibility Learnings

## 2026-06-17 - Asynchronous Interactive Feedback for Hardware Operations
**Learning:** In embedded web dashboards (like ESP32), users often experience "silent" failures or uncertainty because hardware actions (like running a pump or saving to flash/EEPROM) take time. Synchronous alerts are disruptive, while no feedback leads to multiple clicks and potential race conditions.
**Action:** Implement a non-blocking `provideBtnFeedback` pattern that disables the button, shows an interim state (e.g., "Saving..."), and confirms success (e.g., "Saved!") before reverting. This provides clear intent and prevents duplicate hardware commands.
