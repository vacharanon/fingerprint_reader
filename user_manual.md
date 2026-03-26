# User Manual — Fingerprint Reader System

> For the ESP32 full-featured sketch (`esp32/enroll_read/`).
> Last updated: March 2026.

---

## Table of Contents

1. [Getting Started](#getting-started)
2. [Power On & Boot Sequence](#power-on--boot-sequence)
3. [Mode Selection](#mode-selection)
4. [Mode A — Reader](#mode-a--reader)
5. [Mode B — Enroll](#mode-b--enroll)
6. [Mode C — Delete](#mode-c--delete)
7. [Mode D — Status](#mode-d--status)
8. [OTA Firmware Update](#ota-firmware-update)
9. [LED Indicators](#led-indicators)
10. [OLED Messages Reference](#oled-messages-reference)
11. [Troubleshooting](#troubleshooting)
12. [FAQ](#faq)

---

## Getting Started

### What You Need

- ESP32 DevKit V1 with the firmware already uploaded
- AS608 fingerprint sensor connected via UART (GPIO4 RX / GPIO25 TX)
- SSD1306 OLED display (128x32, I2C address `0x3C`)
- 4x4 keypad via PCF8574 I2C expander (address `0x20`)
- Red LED (GPIO12) and Green LED (GPIO13)
- A configured `secrets.h` file with WiFi credentials and backend API details (see `secrets.h.example`)

### First-Time Setup

1. Copy `secrets.h.example` to `secrets.h` in the `esp32/enroll_read/` folder
2. Fill in your WiFi SSID, password, API key, and backend URL
3. Upload the sketch via USB using Arduino IDE (select your ESP32 board)
4. Power the device — it will display **"PALO IT"** on the OLED during boot

---

## Power On & Boot Sequence

When the device powers on, the following sequence occurs:

| Step | OLED Display | LED State | Duration |
|------|-------------|-----------|----------|
| 1 | **PALO IT** | Red ON, Green OFF | ~1 second |
| 2 | **WiFi...** | Red blinks | Up to 15 seconds |
| 3 | Mode selection menu | — | 3 seconds (or until keypress) |
| 4 | Selected mode name (e.g., **READER**) | Depends on mode | Continues to main loop |

If WiFi fails to connect within 15 seconds, the device continues without WiFi. Reader mode (which requires WiFi to POST data) will retry connection later.

---

## Mode Selection

After boot, the OLED displays:

```
MODE
A) Reader B) Enroll
C) Delete D) Status
Autoselect in 3
```

Press a key on the keypad within **3 seconds**:

| Key | Mode | Description |
|-----|------|-------------|
| **A** | Reader | Scan fingerprints and send matched IDs to the backend |
| **B** | Enroll | Register a new fingerprint under a specific ID |
| **C** | Delete | Remove a stored fingerprint by ID |
| **D** | Status | Show how many fingerprints are stored |

If no key is pressed, the device defaults to **Mode A (Reader)**.

---

## Mode A — Reader

This is the primary operating mode for daily use.

### How to Use

1. The OLED shows **"Ready"** and the green LED is ON
2. Place your finger on the sensor
3. If the fingerprint matches a stored template:
   - OLED shows **ID=\<number\>** and the confidence score
   - The device sends the ID to the backend via HTTPS POST
   - OLED shows **"Sending"** → then **"OK"** on success
4. Remove your finger — the device returns to **"Ready"**

### What Happens on Match

- The matched fingerprint ID and confidence score are displayed
- An HTTPS POST is sent to the backend with JSON: `{"id": <number>}`
- The API key is sent in the `Authorization: Bearer` header

### What If No Match

- The OLED displays **"No Match"** and the red LED flashes rapidly for 2 seconds
- The device then returns to **"Ready"** and continues scanning

---

## Mode B — Enroll

Use this mode to register a new fingerprint.

### How to Use

1. OLED shows **"Type in the ID then #"**
2. Use the keypad to enter an ID number (1–128), then press **#**
   - Example: Press `1`, `5`, `#` to enroll as ID 15
3. OLED shows **"Scan #1"** — place your finger on the sensor
4. Wait for the scan to complete, then **remove your finger** when OLED shows **"Remove"**
5. OLED shows **"Scan #2"** — place the **same finger** again
6. If both scans match, OLED shows **"DONE"** — the fingerprint is now stored

### Rules

- Valid IDs: **1 to 128** (ID 0 is not allowed)
- Maximum 3 digits can be entered on the keypad
- If you enter an invalid ID (0 or > 128), the OLED shows **"Invalid"**
- If the two scans don't match, the OLED shows **"Fingerprints did not match"** and the LED blinks red

### Tips

- Keep your finger flat and centered on the sensor for both scans
- Wait for the **"Remove"** prompt before lifting your finger after Scan #1
- If enrollment fails, you can try again immediately — the mode loops back to the ID prompt

---

## Mode C — Delete

Use this mode to remove a stored fingerprint.

### How to Use

1. OLED shows **"Type in the ID to delete then #"**
2. Enter the ID number (1–128) on the keypad, then press **#**
3. If successful, OLED shows **"Deleted"**

### Rules

- Same ID range as enrollment: **1 to 128**
- Deleting a non-existent ID may still show "Deleted" (the sensor clears the slot regardless)
- If the ID is invalid, OLED shows **"Invalid"**

---

## Mode D — Status

Displays the number of enrolled fingerprints.

### What You See

```
Templates:
<count>/128
```

This shows how many fingerprint slots are used out of the 128 available. The display stays for 3 seconds, then the mode loops and checks again.

---

## OTA Firmware Update

The device supports Over-The-Air firmware updates when connected to WiFi.

### Prerequisites

- The device must be on the same WiFi network as your computer
- WiFi must have connected successfully at boot
- Arduino IDE must have the ESP32 board package installed

### How to Update

1. In Arduino IDE, go to **Tools → Port**
2. Look for a network port named **fingerprint-reader** (the OTA hostname)
3. Select it and click **Upload** as usual
4. The OLED will show:
   - **"OTA Update"** when the update starts
   - **"Updating XX%"** with a progress percentage
   - **"OTA Done!"** when complete
5. The device reboots automatically after a successful update

### During OTA

- All normal operations (scanning, enrollment, etc.) are **paused**
- The device resumes normal operation after reboot
- If the update fails, an error is shown on the OLED (e.g., "OTA Auth Fail") and the device continues running the previous firmware

---

## LED Indicators

| Red LED | Green LED | Meaning |
|---------|-----------|---------|
| OFF | **ON** | Ready — waiting for fingerprint (Reader mode) |
| **ON** | OFF | Processing — scanning, enrolling, or sending data |
| **Blinking rapidly** | OFF | Error — check the OLED for details (blinks for ~3 seconds) |
| **Blinking** (during boot) | OFF | Connecting to WiFi |

---

## OLED Messages Reference

| Message | Meaning | Action Required |
|---------|---------|-----------------|
| **PALO IT** | Boot splash screen | Wait |
| **WiFi...** | Connecting to WiFi | Wait (up to 15 seconds) |
| **MODE** | Mode selection screen | Press A / B / C / D |
| **READER** / **ENROLL** / **DELETE** / **STATUS** | Selected mode confirmation | None |
| **Ready** | Waiting for fingerprint scan | Place finger on sensor |
| **ID=\<n\>** + Confidence | Fingerprint matched | Data is being sent |
| **Sending** | HTTP POST in progress | Wait |
| **No Match** | Scanned fingerprint not recognized | Try again or enroll first |
| **OK** | Backend acknowledged the scan | Fingerprint recorded successfully |
| **Type in the ID then #** | Awaiting enroll/delete ID input | Enter ID on keypad |
| **Scan #1** / **Scan #2** | Enrollment scan step | Place finger on sensor |
| **Remove** | Lift finger between enrollment scans | Remove finger |
| **DONE** | Enrollment or deletion complete | Proceed with next action |
| **Deleted** | Fingerprint template removed | Proceed with next action |
| **Invalid** | ID out of range (0 or > 128) | Enter a valid ID (1–128) |
| **Templates: \<n\>/128** | Number of stored fingerprints | Informational |
| **Reconnecting** | WiFi connection lost, retrying | Wait |
| **OTA Update** | Firmware update started | Do not power off |
| **Updating XX%** | Firmware upload progress | Do not power off |
| **OTA Done!** | Update complete, rebooting | Wait for reboot |
| **Communication error** | Sensor UART problem | See Troubleshooting |
| **Image too messy** | Poor fingerprint image quality | Clean finger/sensor, retry |
| **Fingerprints did not match** | Enrollment scans are different | Try enrollment again |
| **Could not find fingerprint features** | Sensor can't extract data | Reposition finger, retry |
| **Could not store in that location** | Storage slot problem | Try a different ID |
| **Error writing to flash** | Sensor flash memory failure | See Troubleshooting |
| **Connection refused** / **Not connected** / **Connection lost** | HTTP/WiFi failure | Check WiFi and backend server |
| **Read timeout** | Backend did not respond in time | Check backend server |

---

## Troubleshooting

### Device does not power on

- Check the USB cable — use a data-capable cable, not a charge-only cable
- Try a different USB port or power supply (5V, at least 500mA)
- Verify the ESP32 is properly seated on the breadboard

### OLED shows "SSD1306 allocation failed" (Serial Monitor)

- The OLED display is not detected on I2C
- Check wiring: SDA → GPIO21, SCL → GPIO22, VCC → 3.3V, GND → GND
- Run the `i2c_scanner/` sketch to verify the OLED appears at address `0x3C`
- Ensure no loose connections on the I2C bus

### "Did not find fingerprint sensor" at boot

- The AS608 sensor is not responding on UART
- Check wiring: Sensor TX → ESP32 GPIO4, Sensor RX → ESP32 GPIO25
- Ensure the sensor VCC is connected to **3.3V** (not 5V on ESP32)
- Verify the baud rate is 57600 (factory default for AS608)
- Try powering off and on — the sensor may need a cold reset

### Keypad not responding

- Check I2C wiring to the PCF8574 module (shared bus with OLED)
- Run `i2c_scanner/` to verify address `0x20`
- PCF8574 and PCF8574A have different addresses — `0x20` is for PCF8574 (non-A variant)
- If using PCF8574A, its address starts at `0x38` — update `KEYPAD_ADDRESS` in the code

### WiFi fails to connect

- OLED shows **"WiFi..."** and then the device continues without WiFi after 15 seconds
- Verify SSID and password in `secrets.h` (case-sensitive)
- Ensure the WiFi network is 2.4 GHz — ESP32 does not support 5 GHz
- Move the device closer to the router
- Check if the router has MAC filtering enabled — whitelist the ESP32's MAC address (shown in Serial Monitor during connection)

### WiFi connects but "Connection refused" or HTTP errors

- Verify the `HOST` URL in `secrets.h` is correct and reachable from the WiFi network
- Ensure the backend server is running and accepting HTTPS connections
- Check that the API key is valid
- Open Serial Monitor (115200 baud) to see the HTTP status code for more detail
- Common HTTP codes: `401` = bad API key, `404` = wrong endpoint URL, `500` = backend error

### "Reconnecting" appears during Reader mode

- WiFi connection was lost — the device is attempting to reconnect
- If reconnection fails, it returns to scanning but cannot send data
- Ensure the WiFi network is stable; consider moving the device closer to the router

### Fingerprint scan always fails ("no match")

- The finger may not be enrolled — use Mode B to enroll first
- Ensure the finger is clean and dry
- Place the same part of the finger that was used during enrollment
- Clean the sensor window with a soft, dry cloth
- Re-enroll the fingerprint with a fresh set of scans if problems persist

### Enrollment "Image too messy" or "Could not find features"

- The finger may be too wet, too dry, or dirty
- Press firmly but not too hard — moderate pressure works best
- Center the finger on the sensor window
- If the problem persists, try a different finger

### "Error writing to flash" during enrollment

- The sensor's internal flash memory may be failing
- Try deleting some templates (Mode C) and re-enrolling
- If the error persists, the sensor may need replacement

### OTA update not appearing in Arduino IDE

- Ensure the ESP32 and your computer are on the **same WiFi network and subnet**
- Verify WiFi connected at boot (check Serial Monitor for IP address)
- In Arduino IDE: Tools → Port — look for **fingerprint-reader** under network ports
- If not visible, restart the IDE or try: `python3 -m esptool --chip esp32 chip_id` to confirm the device is on the network
- Firewall or antivirus may block mDNS discovery — temporarily disable and retry

### OTA update starts but fails

- Ensure the firmware binary is not too large for the OTA partition
- Keep the device powered and on WiFi during the entire update
- Do not power off or reset during "Updating XX%" phase
- Error messages on OLED:
  - **OTA Auth Fail** — authentication mismatch (no password is set by default)
  - **OTA Begin Fail** — not enough flash space; check partition scheme
  - **OTA Connect Fail** — network interrupt during transfer
  - **OTA Recv Fail** — corrupted data during transfer; retry
  - **OTA End Fail** — final write failed; retry

### Serial Monitor shows garbled text

- Set baud rate to **115200** in the Serial Monitor
- Ensure the correct COM port is selected
- On the Arduino combo board (Uno R3), check DIP switch positions — they must route Serial to the correct chip

---

## FAQ

**Q: How many fingerprints can be stored?**
A: The AS608 sensor stores up to **128 templates** (IDs 1–128).

**Q: Can I enroll the same finger under multiple IDs?**
A: Yes. Each enrollment creates an independent template in the selected slot.

**Q: What happens if I enroll a new finger to an already-used ID?**
A: The old template is overwritten with the new one. No confirmation is asked.

**Q: Can I use the system without WiFi?**
A: Modes B (Enroll), C (Delete), and D (Status) work fully offline. Mode A (Reader) requires WiFi to send data to the backend, but fingerprint matching itself works offline — the scan result just won't be transmitted.

**Q: How do I factory-reset all fingerprints?**
A: There is no factory-reset function in the current firmware. Delete fingerprints one at a time using Mode C. Alternatively, use the Adafruit Fingerprint library's `finger.emptyDatabase()` function by modifying the sketch.

**Q: Can I change the mode without rebooting?**
A: No. The mode is selected once during boot. Press the **RST** button on the ESP32 to reboot and choose a different mode.

**Q: What confidence score is considered a good match?**
A: The AS608 sensor returns a confidence value (typically 0–300+). Values above **50** generally indicate a reliable match. The default library threshold is set at the sensor level. Higher values mean better match quality.

**Q: How do I find the device's IP address for OTA?**
A: Open the Serial Monitor at 115200 baud during boot. The IP address is printed after WiFi connects (e.g., `IP: 192.168.1.42`).
