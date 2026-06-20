# Fingerprint Reader PoC

Proof-of-concept using a low-cost Arduino Uno R3 with Wi-Fi (ATmega328P + ESP8266, the 8-DIP-switch variant).

Demonstrates serial communication between the ATmega328P MCU and ESP8266, and extends enrollment/verification to a macOS desktop app via Touch ID.

---

## Repository structure

```
fingerprint_reader/
├── esp32/          — ESP32 firmware (AS608 sensor + Wi-Fi HTTP)
├── enroll/         — Arduino enrollment sketch
├── enroll_read/    — Arduino enrollment + read sketch
├── reader/         — Arduino reader sketch
├── esp8266_send/   — ESP8266 HTTP send helper
├── i2c_scanner/    — I²C bus scanner utility
└── desktop/        — macOS Electron app (Touch ID enrollment/verification)
```

---

## Hardware – Arduino / ESP32

These sketches demonstrate serial communication between the ATmega328P MCU and an ESP8266, and fingerprint enrollment/verification with the **AS608** optical sensor.

### Requirements

- Arduino Uno R3 with ESP8266 Wi-Fi module (8-DIP-switch board), **or** ESP32
- AS608 fingerprint sensor
- Arduino IDE ≥ 1.8 with the [Adafruit Fingerprint Sensor Library](https://github.com/adafruit/Adafruit-Fingerprint-Sensor-Library)

### Configuration

Open `esp32/secrets.h` (copy from `secrets.h.example` if present) and set your Wi-Fi credentials and backend URL.

---

## Desktop App – macOS Touch ID

The `desktop/` directory contains an **Electron** app that lets you enroll and verify fingerprints on any Mac with Touch ID, using `systemPreferences.promptTouchID()`.

### How it works

After a successful Touch ID prompt the app POSTs the same payload used by the Arduino/ESP32 firmware:

```json
{ "api_key": "<your key>", "id": <slot 1-127> }
```

The backend handles desktop and hardware requests identically.

### Requirements

- macOS with Touch ID hardware
- Node.js ≥ 18

### Setup

#### 1. Install dependencies

```bash
cd desktop
npm install
```

#### 2. Configure the backend endpoint

```bash
cp desktop/secrets.example.js desktop/secrets.js
```

Edit `desktop/secrets.js` and fill in your values:

```js
module.exports = {
  HOST:    "http://YOUR_BACKEND_HOST/fingerprint",
  API_KEY: "YOUR_API_KEY",
};
```

> `secrets.js` is listed in `.gitignore` and will never be committed.

#### 3. Run the app

```bash
cd desktop
npm start
```

### App screens

| Screen | Description |
|--------|-------------|
| **Home** | Navigate to Enroll or Verify |
| **Enroll** | Enter a slot ID (1–127), approve the Touch ID prompt, and register the slot with the backend |
| **Verify** | Enter your slot ID, approve the Touch ID prompt, and confirm verification with the backend |

### Project layout

```
desktop/
├── main.js               — Electron main process; Touch ID + HTTP to backend
├── preload.js            — contextBridge (contextIsolation: true, nodeIntegration: false)
├── package.json          — dependencies and npm start script
├── secrets.example.js    — template for secrets.js
└── renderer/
    ├── index.html        — single-page app shell
    ├── app.js            — screen navigation and backend integration
    └── styles.css        — macOS-native styling (system font, hiddenInset title bar)
```

---
