# API References

> Backend API used by the Fingerprint Reader system to report matched fingerprint IDs.

---

## Base URL

Configured via `HOST` in `secrets.h`:

```
https://your-backend-api.example.com/endpoint
```

---

## Authentication

| Architecture | Method | Details |
|---|---|---|
| **ESP32** (`esp32/enroll_read/`) | Bearer token in `Authorization` header | `Authorization: Bearer <API_KEY>` |
| **ESP8266** (`esp8266_send/`) | API key in request body (legacy) | `"api_key": "<API_KEY>"` |

> **Note:** The ESP8266 approach is legacy. SEC-03 in [task.md](task.md) tracks migrating all sketches to use the `Authorization` header.

---

## Endpoints

### POST — Report Fingerprint Match

Sent when the reader successfully matches a fingerprint against a stored template.

**URL:** `{HOST}` (the full URL including path is defined in `secrets.h`)

**Method:** `POST`

**Headers:**

| Header | Value | Required |
|---|---|---|
| `Content-Type` | `application/json` | Yes |
| `Authorization` | `Bearer <API_KEY>` | Yes (ESP32) |

**Request Body:**

```json
{
  "id": 15
}
```

| Field | Type | Description |
|---|---|---|
| `id` | `integer` | Matched fingerprint template ID (1–128) |

**Legacy Request Body (ESP8266 only):**

```json
{
  "api_key": "YOUR_API_KEY",
  "id": 15
}
```

| Field | Type | Description |
|---|---|---|
| `api_key` | `string` | API key for authentication (legacy — moved to header on ESP32) |
| `id` | `integer` | Matched fingerprint template ID (1–128) |

#### Success Response

| Status Code | Meaning |
|---|---|
| `200 OK` | The fingerprint ID was received and processed |

The device displays **"OK"** on the OLED and returns to the ready state.

#### Error Responses

Any non-200 status code is treated as an error. The device displays the HTTP status code on the OLED and blinks the red LED.

| Status Code | Typical Meaning |
|---|---|
| `401 Unauthorized` | Invalid or missing API key |
| `400 Bad Request` | Malformed JSON or missing `id` field |
| `500 Internal Server Error` | Backend processing failure |

---

## Device-Side Error Codes

The ESP32 `HTTPClient` library may return negative error codes before an HTTP response is received:

| Code | Constant | Meaning |
|---|---|---|
| `-1` | `HTTPC_ERROR_CONNECTION_REFUSED` | Connection refused by server |
| `-2` | `HTTPC_ERROR_SEND_HEADER_FAILED` | Failed to send request headers |
| `-3` | `HTTPC_ERROR_SEND_PAYLOAD_FAILED` | Failed to send request body |
| `-4` | `HTTPC_ERROR_NOT_CONNECTED` | Not connected to server |
| `-5` | `HTTPC_ERROR_CONNECTION_LOST` | Connection lost during request |
| `-6` | `HTTPC_ERROR_NO_STREAM` | No response stream available |
| `-7` | `HTTPC_ERROR_NO_HTTP_SERVER` | No HTTP server at the target |
| `-8` | `HTTPC_ERROR_TOO_LESS_RAM` | Insufficient memory |
| `-9` | `HTTPC_ERROR_ENCODING` | Encoding error |
| `-10` | `HTTPC_ERROR_STREAM_WRITE` | Stream write error |
| `-11` | `HTTPC_ERROR_READ_TIMEOUT` | Read timeout |

---

## Dual-Chip Serial Protocol (Arduino ↔ ESP8266)

When using the Arduino Uno + ESP8266 combo board, the two chips communicate over UART at **115200 baud**. The ESP8266 acts as a WiFi bridge.

| Direction | Message | Description |
|---|---|---|
| ESP8266 → Arduino | `[ESP8266]READY\n` | WiFi connected, ready to receive IDs |
| Arduino → ESP8266 | `[F_ID]<id>\n` | Fingerprint matched — send this ID to backend |
| ESP8266 → Arduino | `[ESP8266]<httpCode>\n` | HTTP response status code from backend |

---

## TLS / SSL Configuration

| Architecture | TLS Method | Notes |
|---|---|---|
| **ESP32** | `client.setInsecure()` (PoC) | Should use `client.setCACert(root_ca)` for production (see SEC-02) |
| **ESP8266** | `client.setFingerprint(fingerprint)` | SHA-1 fingerprint of backend's SSL certificate defined in `secrets.h` |

---

## Example: cURL

```bash
# ESP32-style (Bearer token)
curl -X POST https://your-backend-api.example.com/endpoint \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer YOUR_API_KEY" \
  -d '{"id": 15}'

# ESP8266-style (legacy — API key in body)
curl -X POST https://your-backend-api.example.com/endpoint \
  -H "Content-Type: application/json" \
  -d '{"api_key": "YOUR_API_KEY", "id": 15}'
```
