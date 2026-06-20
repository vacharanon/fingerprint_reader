const { contextBridge, ipcRenderer } = require("electron");

/**
 * Expose a minimal, typed API to the renderer process via the context bridge.
 * This keeps nodeIntegration disabled while still allowing IPC calls.
 */
contextBridge.exposeInMainWorld("electronAPI", {
  /**
   * Show the macOS Touch ID prompt.
   * @param {string} reason  Text shown in the system dialog.
   * @returns {Promise<{success: boolean, error?: string}>}
   */
  promptTouchID: (reason) => ipcRenderer.invoke("touch-id-prompt", reason),

  /**
   * Send a fingerprint slot ID to the backend (enroll or verify).
   * @param {number} id  Fingerprint slot ID (1–127)
   * @returns {Promise<{success: boolean, status: number, message: string}>}
   */
  sendFingerprint: (id) => ipcRenderer.invoke("fingerprint-send", id),
});
