## 2025-05-15 - Rigid API Mapping in Embedded Dashboards
**Learning:** In embedded web dashboards (like those on ESP32), the backend JSON parser is often extremely rigid and relies on hardcoded keys. Refactoring JavaScript to use dynamic key generation (e.g., kebab-to-camelCase conversion) can easily break the API contract if the generated keys do not exactly match the backend's expected schema (e.g., `targetPh` vs `phTarget`).
**Action:** Always maintain explicit key mapping for API payloads in embedded environments unless the backend schema is explicitly documented as flexible.
