import { HOST, API_KEY } from "../secrets";

/**
 * Notify the backend that a fingerprint ID was successfully authenticated.
 * Mirrors the JSON payload used by the Arduino/ESP32 firmware:
 *   { "api_key": "...", "id": <int> }
 *
 * @param {number} id  Fingerprint slot ID (1-127)
 * @returns {Promise<{success: boolean, status: number, message: string}>}
 */
export async function sendFingerprintId(id) {
  const response = await fetch(HOST, {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify({ api_key: API_KEY, id }),
  });
  return {
    success: response.ok,
    status: response.status,
    message: response.ok ? "OK" : `HTTP ${response.status}`,
  };
}

/**
 * Register a mobile device enrollment for the given fingerprint slot ID.
 * Sends the same payload as sendFingerprintId so the backend can record
 * that this slot was enrolled from a mobile device.
 *
 * @param {number} id  Fingerprint slot ID (1-127)
 * @returns {Promise<{success: boolean, status: number, message: string}>}
 */
export async function enrollFingerprintId(id) {
  return sendFingerprintId(id);
}
