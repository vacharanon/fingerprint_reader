# Fingerprint Reader PoC — Improvement Tasks

> Prioritized backlog as of March 2026. Updated: March 20, 2026.
>
> **Progress: 13/20 tasks completed**

---

## P0 — Security (Must Fix)

- [x] **SEC-01** Create `secrets.h.example` template so contributors know required variables without exposing real values
- [ ] **SEC-02** Replace `client.setInsecure()` with `client.setCACert(root_ca)` using a proper PEM root CA certificate in `esp32/enroll_read/`
  - Currently flagged with a TODO comment; needs a real cert before any deployment
- [x] **SEC-03** Move API key from JSON request body to `Authorization` header across `esp8266_send/` and `esp32/enroll_read/`
- [ ] **SEC-04** Add anti-replay protection — AS608 is vulnerable to printed fingerprint attacks; evaluate capacitive sensors for production hardware

---

## P1 — Reliability

- [x] **REL-01** Add 10-second timeout to all blocking `while (Serial.available() == 0)` waits in `reader/`, `esp32/reader/`, and `enroll_read/`
- [ ] **REL-02** Add WiFi reconnect logic to `esp8266_send/` — currently only `setAutoReconnect(true)` is set but `WiFi.status()` is never checked in `loop()`
  - The ESP32 version already handles this; backport the same pattern
- [x] **REL-03** Cap keypad input buffer to 3 characters (IDs 1–128) in `enroll_read/` and `esp32/enroll_read/` to prevent memory issues
- [ ] **REL-04** Handle and log HTTP response body in `esp8266_send/` — currently only status code is returned, making debugging difficult

---

## P2 — Code Quality & Maintainability

- [x] **CQ-01** Standardize baud rate — `enroll/` was using 9600 while everything else uses 115200
- [x] **CQ-02** Remove ~50 lines of dead commented-out code in `reader/reader.ino` (duplicate `getFingerprintID()` function, stale debug lines)
- [x] **CQ-03** Extract OLED helper functions (`oledShow(text, size)`) — the same 5-line OLED pattern is repeated 15+ times per sketch
- [ ] **CQ-04** Create a shared local Arduino library (`lib/FingerprintHelpers/`) for common OLED, fingerprint, and LED utility functions
- [x] **CQ-05** Replace magic numbers (delay values `2000`/`3000`, pin numbers, thresholds) with named `#define` constants

---

## P3 — Feature Enhancements

| ID | Feature | Value | Effort |
|---|---|---|---|
| **FE-01** | ~~Display confidence score on OLED after fingerprint match~~ | Done in `esp32/enroll_read/` | Low |
| **FE-02** | ~~OTA firmware updates via ArduinoOTA on ESP32~~ | Done in `esp32/enroll_read/` | Medium |
| **FE-03** | Simple web dashboard on ESP32 (list enrolled IDs, delete remotely, check status) | Remote management capability | Medium |
| **FE-04** | Audit log — store scan events (ID, timestamp, success/fail) in SPIFFS/LittleFS on ESP32 | Traceability and accountability | Medium |

---

## P4 — Developer Experience

- [x] **DX-01** Add a wiring diagram image (Fritzing or hand-drawn PNG) to the repo
  - See Gemini prompt below
- [x] **DX-02** ~~Migrate to PlatformIO~~ Stay with Arduino IDE + VS Code — install recommended extensions instead
  - See VS Code extension list below
- [x] **DX-03** Add a GitHub Actions CI pipeline that compiles all sketches on push using `arduino-cli` to catch build regressions

---

## DX-01 — Gemini Image Generation Prompt

Copy-paste the following prompt into **Gemini** (with image generation enabled) to produce the wiring diagram:

> **Prompt:**
>
> Generate a clean, professional electronics wiring diagram (NOT a schematic — a physical wiring/breadboard-style diagram like Fritzing) for the following ESP32 project. Use distinct wire colors for each bus. Label every pin. White background, high resolution, landscape orientation.
>
> **Components (draw each as a recognizable module/breakout board):**
> 1. ESP32 DevKit V1 (30-pin) — central MCU
> 2. AS608 optical fingerprint sensor module (4 pins: VCC, GND, TX, RX)
> 3. SSD1306 0.91" OLED display 128x32 pixels (4 pins: VCC, GND, SCL, SDA)
> 4. 4x4 membrane matrix keypad connected via PCF8574 I2C I/O expander module (4 pins exposed: VCC, GND, SCL, SDA)
> 5. Red 5mm LED with 220Ω resistor
> 6. Green 5mm LED with 220Ω resistor
>
> **Wiring (use these exact connections):**
>
> *Power (red wires):*
> - AS608 VCC → ESP32 3.3V
> - SSD1306 VCC → ESP32 3.3V
> - PCF8574 VCC → ESP32 3.3V
> - Both LED 220Ω resistors → ESP32 GPIO (see below)
>
> *Ground (black wires):*
> - AS608 GND → ESP32 GND
> - SSD1306 GND → ESP32 GND
> - PCF8574 GND → ESP32 GND
> - Both LED cathodes → ESP32 GND
>
> *I2C bus (blue wires — shared bus, 2 devices):*
> - SSD1306 SCL → ESP32 GPIO22 (SCL)
> - SSD1306 SDA → ESP32 GPIO21 (SDA)
> - PCF8574 SCL → ESP32 GPIO22 (SCL) (same bus)
> - PCF8574 SDA → ESP32 GPIO21 (SDA) (same bus)
> - Label: "I2C Bus — OLED @ 0x3C, Keypad @ 0x20"
>
> *UART (green wires):*
> - AS608 TX → ESP32 GPIO4 (Serial2 RX)
> - AS608 RX → ESP32 GPIO25 (Serial2 TX)
> - Label: "UART2 @ 57600 baud"
>
> *LEDs (orange wires):*
> - Red LED anode → 220Ω resistor → ESP32 GPIO12
> - Green LED anode → 220Ω resistor → ESP32 GPIO13
>
> **Style requirements:**
> - Use a breadboard or flat-layout style (not a circuit schematic)
> - Each component should be visually distinct and labeled with its name
> - Add a title: "ESP32 Fingerprint Reader — Wiring Diagram"
> - Add a legend box in the corner showing wire color meanings
> - Make text readable at 1920x1080 resolution

---

## DX-02 — Recommended VS Code Extensions for Arduino/ESP32

Install these extensions in VS Code to improve developer experience without migrating away from Arduino IDE:

| Extension | ID | Purpose |
|---|---|---|
| **Arduino** | `vsciot-vscode.vscode-arduino` | Board selection, upload, serial monitor, IntelliSense for .ino files |
| **C/C++** | `ms-vscode.cpptools` | IntelliSense, code navigation, auto-complete for Arduino/ESP32 headers |
| **Serial Monitor** | `ms-vscode.vscode-serial-monitor` | Built-in serial monitor — no need to switch to Arduino IDE |
| **ESP-IDF** (optional) | `espressif.esp-idf-extension` | Advanced ESP32 support: flash, monitor, menuconfig, partition editor |
| **PlatformIO IDE** (optional) | `platformio.platformio-ide` | If you ever want PlatformIO — works alongside Arduino extension |
| **GitLens** | `eamodio.gitlens` | Git blame, history, and diff inline — useful for tracking wiring changes |
| **Todo Tree** | `gruntfuggly.todo-tree` | Highlights TODO/FIXME/WARNING comments across all sketches |
| **Hex Editor** | `ms-vscode.hexeditor` | Inspect binary firmware files |

### Quick Install (run in terminal)

```bash
code --install-extension vsciot-vscode.vscode-arduino \
     --install-extension ms-vscode.cpptools \
     --install-extension ms-vscode.vscode-serial-monitor \
     --install-extension eamodio.gitlens \
     --install-extension gruntfuggly.todo-tree
```

### Recommended `c_cpp_properties.json` for IntelliSense

Create `.vscode/c_cpp_properties.json` in the repo root to get proper autocomplete for Arduino and ESP32 headers:

```json
{
  "configurations": [
    {
      "name": "ESP32",
      "includePath": [
        "${workspaceFolder}/**",
        "~/Library/Arduino15/packages/esp32/hardware/esp32/*/cores/esp32",
        "~/Library/Arduino15/packages/esp32/hardware/esp32/*/libraries/**",
        "~/Documents/Arduino/libraries/**"
      ],
      "defines": ["ESP32", "ARDUINO=10819"],
      "compilerPath": "",
      "cStandard": "c11",
      "cppStandard": "c++17",
      "intelliSenseMode": "gcc-arm"
    }
  ],
  "version": 4
}
```

---

## Suggested Execution Order (remaining 7 items)

| # | Task | Effort | Depends On | Notes |
|---|------|--------|------------|-------|
| 1 | **SEC-02** — Replace `client.setInsecure()` with `client.setCACert(root_ca)` | Low | Backend team provides PEM root CA cert | Highest security risk remaining — MITM-vulnerable until fixed |
| 2 | **REL-02** — Add WiFi reconnect logic to `esp8266_send/` | Low | — | Backport `WiFi.status()` check pattern already in ESP32 sketch |
| 3 | **REL-04** — Parse and log HTTP response body in `esp8266_send/` | Low | — | Currently only status code is captured; aids debugging |
| 4 | **CQ-04** — Extract shared Arduino library (`lib/FingerprintHelpers/`) | Medium | — | Reduces duplication of OLED, LED, and fingerprint helpers across 8 sketches |
| 5 | **FE-04** — Audit log in SPIFFS/LittleFS on ESP32 | Medium | — | Store timestamped scan events (ID, result, confidence); prerequisite data for FE-03 dashboard |
| 6 | **FE-03** — Web dashboard on ESP32 (AsyncWebServer) | Medium | FE-04 | List enrolled IDs, view audit log, delete remotely; depends on stored data from FE-04 |
| 7 | **SEC-04** — Evaluate anti-replay / liveness detection | Research | — | Hardware decision — compare capacitive vs optical sensors; no code change until hardware selected |
