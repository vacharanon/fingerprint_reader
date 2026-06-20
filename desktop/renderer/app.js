/**
 * Fingerprint Reader – Desktop renderer
 *
 * Handles screen navigation and the Enroll / Verify flows.
 * All IPC calls are made through window.electronAPI (exposed by preload.js).
 */

// ── Screen navigation ─────────────────────────────────────────

const screens = {
  home:   document.getElementById("screen-home"),
  enroll: document.getElementById("screen-enroll"),
  verify: document.getElementById("screen-verify"),
};

function showScreen(name) {
  Object.values(screens).forEach((s) => s.classList.remove("active"));
  screens[name].classList.add("active");
}

document.getElementById("btn-go-enroll").addEventListener("click", () => showScreen("enroll"));
document.getElementById("btn-go-verify").addEventListener("click", () => showScreen("verify"));
document.getElementById("btn-back-enroll").addEventListener("click", () => {
  clearStatus("enroll-status");
  showScreen("home");
});
document.getElementById("btn-back-verify").addEventListener("click", () => {
  clearStatus("verify-status");
  showScreen("home");
});

// ── Status helpers ────────────────────────────────────────────

function setStatus(id, message, type) {
  const el = document.getElementById(id);
  el.textContent = message;
  el.className = `status ${type}`;
}

function clearStatus(id) {
  const el = document.getElementById(id);
  el.textContent = "";
  el.className = "status hidden";
}

// ── Shared biometric + backend flow ───────────────────────────

/**
 * Run the Touch ID → backend flow used by both Enroll and Verify.
 *
 * @param {object} opts
 * @param {string}  opts.inputId      ID of the slot-number <input>
 * @param {string}  opts.statusId     ID of the status <div>
 * @param {string}  opts.buttonId     ID of the action <button>
 * @param {string}  opts.promptMsg    Reason shown in the Touch ID dialog
 * @param {string}  opts.successMsg   Message shown on full success (slot replaced by actual id)
 */
async function runBiometricFlow({ inputId, statusId, buttonId, promptMsg, successMsg }) {
  clearStatus(statusId);

  const rawValue = document.getElementById(inputId).value.trim();
  const id = parseInt(rawValue, 10);

  if (!rawValue || isNaN(id) || id < 1 || id > 127) {
    setStatus(statusId, "Please enter a valid slot ID between 1 and 127.", "error");
    return;
  }

  const btn = document.getElementById(buttonId);
  btn.disabled = true;
  btn.textContent = "Authenticating…";

  try {
    // Step 1: Touch ID
    const authResult = await window.electronAPI.promptTouchID(
      promptMsg.replace("{id}", id)
    );

    if (!authResult.success) {
      setStatus(
        statusId,
        authResult.error === "system cancelled"
          ? "Authentication cancelled."
          : `Touch ID failed: ${authResult.error}`,
        "error"
      );
      return;
    }

    // Step 2: Backend
    const result = await window.electronAPI.sendFingerprint(id);
    if (result.success) {
      setStatus(statusId, successMsg.replace("{id}", id), "success");
      document.getElementById(inputId).value = "";
    } else {
      setStatus(statusId, `Server error: ${result.message}`, "error");
    }
  } catch (err) {
    setStatus(statusId, `Unexpected error: ${err.message}`, "error");
  } finally {
    btn.disabled = false;
    btn.textContent = btn.dataset.label;
  }
}

// ── Enroll button ─────────────────────────────────────────────

const btnEnroll = document.getElementById("btn-enroll");
btnEnroll.dataset.label = btnEnroll.textContent;

btnEnroll.addEventListener("click", () =>
  runBiometricFlow({
    inputId:    "enroll-id",
    statusId:   "enroll-status",
    buttonId:   "btn-enroll",
    promptMsg:  "Enroll fingerprint for slot #{id}",
    successMsg: "Fingerprint slot #{id} enrolled successfully!",
  })
);

// ── Verify button ─────────────────────────────────────────────

const btnVerify = document.getElementById("btn-verify");
btnVerify.dataset.label = btnVerify.textContent;

btnVerify.addEventListener("click", () =>
  runBiometricFlow({
    inputId:    "verify-id",
    statusId:   "verify-status",
    buttonId:   "btn-verify",
    promptMsg:  "Verify fingerprint for slot #{id}",
    successMsg: "Slot #{id} verified! Access granted.",
  })
);
