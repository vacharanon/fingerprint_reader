const { app, BrowserWindow, ipcMain, systemPreferences } = require("electron");
const path = require("path");

let config;
try {
  config = require("./secrets");
} catch {
  config = { HOST: "", API_KEY: "" };
}
const { HOST, API_KEY } = config;

function createWindow() {
  const win = new BrowserWindow({
    width: 480,
    height: 620,
    resizable: false,
    titleBarStyle: "hiddenInset",
    webPreferences: {
      preload: path.join(__dirname, "preload.js"),
      contextIsolation: true,
      nodeIntegration: false,
    },
  });

  win.loadFile(path.join(__dirname, "renderer", "index.html"));
}

app.whenReady().then(() => {
  createWindow();

  app.on("activate", () => {
    if (BrowserWindow.getAllWindows().length === 0) createWindow();
  });
});

app.on("window-all-closed", () => {
  if (process.platform !== "darwin") app.quit();
});

/**
 * Touch ID prompt — macOS only.
 * Returns { success: true } on approval, { success: false, error: string } otherwise.
 */
ipcMain.handle("touch-id-prompt", async (_event, reason) => {
  if (process.platform !== "darwin") {
    return { success: false, error: "Touch ID is only available on macOS." };
  }
  try {
    await systemPreferences.promptTouchID(reason);
    return { success: true };
  } catch (err) {
    return { success: false, error: err.message };
  }
});

/**
 * Send fingerprint slot ID to the backend.
 * Mirrors the JSON payload used by the Arduino/ESP32 firmware:
 *   { "api_key": "...", "id": <int> }
 */
ipcMain.handle("fingerprint-send", async (_event, id) => {
  if (!HOST) {
    return {
      success: false,
      status: 0,
      message: "Backend not configured. Copy secrets.example.js to secrets.js and fill in HOST and API_KEY.",
    };
  }
  try {
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
  } catch (err) {
    return { success: false, status: 0, message: err.message };
  }
});
