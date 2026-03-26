# Fingerprint Reader PoC

Proof-of-concept of using cheap Chinese Arduino Uno R3 with Wifi (Atmega328P + ESP8266).
The one with 8 DIP switches.

These codes demonstrate Serial communication between Atmega328P MCU and ESP8266.

---

## Mobile App – Enroll via Apple Touch ID / Android Fingerprint

The `mobile/` directory contains a **React Native (Expo)** app that lets you
enroll and verify fingerprints using your phone's built-in biometric sensor:

| Platform | Supported sensors |
|----------|-------------------|
| iOS      | Touch ID, Face ID |
| Android  | Fingerprint, Face Unlock (device-dependent) |

### How it works

The mobile app mirrors the same API contract used by the Arduino/ESP32 firmware.
After a successful biometric prompt the app sends:

```json
{ "api_key": "<your key>", "id": <slot 1-127> }
```

to the same backend endpoint, so the server sees a mobile verification exactly
like a hardware sensor scan.

### Setup

#### 1. Install dependencies

```bash
cd mobile
npm install
```

#### 2. Configure the API endpoint

```bash
cp mobile/secrets.example.js mobile/secrets.js
```

Edit `mobile/secrets.js` and fill in your `HOST` URL and `API_KEY`:

```js
export const HOST = "https://your-api-host/fingerprint-endpoint";
export const API_KEY = "your-api-key-here";
```

> `secrets.js` is listed in `.gitignore` and will never be committed.

#### 3. Run on device

```bash
# iOS simulator / physical device (requires macOS + Xcode)
cd mobile && npx expo start --ios

# Android emulator / physical device
cd mobile && npx expo start --android
```

Or start Expo Dev Server and scan the QR code with the **Expo Go** app:

```bash
cd mobile && npx expo start
```

### App screens

| Screen | Description |
|--------|-------------|
| **Home** | Choose between Enroll and Verify |
| **Enroll** | Enter a slot ID (1–127), authenticate with Touch ID/Fingerprint, and register the enrollment with the backend |
| **Verify** | Enter your slot ID, authenticate with Touch ID/Fingerprint, and notify the backend of a successful verification |

### iOS – required permission

The `NSFaceIDUsageDescription` key is already set in `mobile/app.json`.
When building a production `.ipa` you do **not** need to add it manually.

### Android – required permissions

`USE_BIOMETRIC` and `USE_FINGERPRINT` are already declared in `mobile/app.json`.

---
