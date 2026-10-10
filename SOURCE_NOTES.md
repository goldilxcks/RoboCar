# Source notes

This repository was organized from the supplied Arduino/ESP32 project files.

Changes made while organizing:

- removed IDE log files from the repository layout;
- kept the main robot, camera, line-tracking and servo source code;
- replaced hard-coded Wi-Fi credentials with safe placeholders;
- added README documentation and a `.gitignore`;
- kept `esp_http_server.h` under `reference/` because it is normally provided by the ESP32 framework.

No attempt was made to redesign the original robot-control logic.
