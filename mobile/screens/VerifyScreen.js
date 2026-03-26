import React, { useEffect, useState } from "react";
import {
  ActivityIndicator,
  Alert,
  KeyboardAvoidingView,
  Platform,
  StyleSheet,
  Text,
  TextInput,
  TouchableOpacity,
  View,
} from "react-native";
import * as LocalAuthentication from "expo-local-authentication";
import { sendFingerprintId } from "../api/fingerprintApi";
import StatusMessage from "../components/StatusMessage";

/**
 * Verify screen
 *
 * Flow:
 *  1. Check that the device has biometric hardware and enrolled biometrics.
 *  2. User enters the fingerprint slot ID they previously enrolled.
 *  3. Tap "Verify" → OS biometric prompt appears (Touch ID / Face ID / Fingerprint).
 *  4. On success the app sends the slot ID to the backend (same payload as the
 *     Arduino/ESP32 devices), giving the server a chance to act on the match.
 */
export default function VerifyScreen() {
  const [biometricAvailable, setBiometricAvailable] = useState(false);
  const [biometricType, setBiometricType] = useState("Biometric");
  const [fingerprintId, setFingerprintId] = useState("");
  const [loading, setLoading] = useState(false);
  const [status, setStatus] = useState({ message: "", type: "info" });

  // Check biometric capability on mount
  useEffect(() => {
    (async () => {
      const compatible = await LocalAuthentication.hasHardwareAsync();
      const enrolled = await LocalAuthentication.isEnrolledAsync();
      setBiometricAvailable(compatible && enrolled);

      if (compatible) {
        const types =
          await LocalAuthentication.supportedAuthenticationTypesAsync();
        if (
          types.includes(
            LocalAuthentication.AuthenticationType.FACIAL_RECOGNITION
          )
        ) {
          setBiometricType("Face ID");
        } else if (
          types.includes(LocalAuthentication.AuthenticationType.FINGERPRINT)
        ) {
          setBiometricType(
            Platform.OS === "ios" ? "Touch ID" : "Fingerprint"
          );
        }
      }
    })();
  }, []);

  const handleVerify = async () => {
    // Validate ID
    const id = parseInt(fingerprintId, 10);
    if (!fingerprintId || isNaN(id) || id < 1 || id > 127) {
      setStatus({
        message: "Please enter a valid slot ID between 1 and 127.",
        type: "error",
      });
      return;
    }

    if (!biometricAvailable) {
      Alert.alert(
        "Biometrics unavailable",
        "No biometric hardware found or no biometrics enrolled on this device."
      );
      return;
    }

    setLoading(true);
    setStatus({ message: "", type: "info" });

    try {
      // Step 1: Authenticate with device biometrics
      const authResult = await LocalAuthentication.authenticateAsync({
        promptMessage: `Verify fingerprint for slot #${id}`,
        cancelLabel: "Cancel",
        fallbackLabel: "Use Passcode",
        disableDeviceFallback: false,
      });

      if (!authResult.success) {
        setStatus({
          message:
            authResult.error === "user_cancel"
              ? "Verification cancelled."
              : `Authentication failed: ${authResult.error}`,
          type: "error",
        });
        return;
      }

      // Step 2: Notify the backend with the authenticated ID
      const result = await sendFingerprintId(id);
      if (result.success) {
        setStatus({
          message: `Slot #${id} verified! Access granted.`,
          type: "success",
        });
      } else {
        setStatus({
          message: `Server returned: ${result.message}`,
          type: "error",
        });
      }
    } catch (error) {
      setStatus({
        message: `Error: ${error.message}`,
        type: "error",
      });
    } finally {
      setLoading(false);
    }
  };

  return (
    <KeyboardAvoidingView
      style={styles.container}
      behavior={Platform.OS === "ios" ? "padding" : "height"}
    >
      <View style={styles.inner}>
        <Text style={styles.title}>Verify Fingerprint</Text>
        <Text style={styles.subtitle}>
          Enter your slot ID and authenticate with{" "}
          <Text style={styles.biometricType}>{biometricType}</Text> to verify
          your identity with the reader system.
        </Text>

        {!biometricAvailable && (
          <StatusMessage
            message="No biometric hardware or biometrics not enrolled on this device."
            type="error"
          />
        )}

        <Text style={styles.label}>Fingerprint Slot ID (1–127)</Text>
        <TextInput
          style={styles.input}
          keyboardType="number-pad"
          maxLength={3}
          placeholder="e.g. 1"
          value={fingerprintId}
          onChangeText={setFingerprintId}
          editable={!loading}
          returnKeyType="done"
        />

        <StatusMessage message={status.message} type={status.type} />

        <TouchableOpacity
          style={[
            styles.button,
            (!biometricAvailable || loading) && styles.buttonDisabled,
          ]}
          onPress={handleVerify}
          disabled={!biometricAvailable || loading}
        >
          {loading ? (
            <ActivityIndicator color="#fff" />
          ) : (
            <Text style={styles.buttonText}>
              Verify with {biometricType}
            </Text>
          )}
        </TouchableOpacity>

        <Text style={styles.hint}>
          Authenticate with the same biometric used during enrollment to notify
          the server of a successful verification.
        </Text>
      </View>
    </KeyboardAvoidingView>
  );
}

const styles = StyleSheet.create({
  container: { flex: 1, backgroundColor: "#fff" },
  inner: {
    flex: 1,
    padding: 24,
    justifyContent: "center",
  },
  title: {
    fontSize: 26,
    fontWeight: "bold",
    marginBottom: 10,
    textAlign: "center",
  },
  subtitle: {
    fontSize: 14,
    color: "#555",
    textAlign: "center",
    marginBottom: 28,
    lineHeight: 21,
  },
  biometricType: { fontWeight: "bold", color: "#34C759" },
  label: { fontSize: 14, fontWeight: "600", marginBottom: 6 },
  input: {
    borderWidth: 1,
    borderColor: "#ccc",
    borderRadius: 8,
    paddingHorizontal: 14,
    paddingVertical: 12,
    fontSize: 18,
    marginBottom: 4,
    textAlign: "center",
  },
  button: {
    backgroundColor: "#34C759",
    paddingVertical: 15,
    borderRadius: 10,
    alignItems: "center",
    marginTop: 8,
  },
  buttonDisabled: { backgroundColor: "#9fe0b1" },
  buttonText: { color: "#fff", fontSize: 17, fontWeight: "600" },
  hint: {
    marginTop: 20,
    fontSize: 12,
    color: "#888",
    textAlign: "center",
    lineHeight: 18,
  },
});
