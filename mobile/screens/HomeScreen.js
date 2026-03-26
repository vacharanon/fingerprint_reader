import React from "react";
import {
  StyleSheet,
  Text,
  TouchableOpacity,
  View,
} from "react-native";

/**
 * Home screen – presents two options:
 *   • Enroll   – save a new fingerprint ID linked to this device's biometric
 *   • Verify   – authenticate with biometrics and notify the backend
 */
export default function HomeScreen({ navigation }) {
  return (
    <View style={styles.container}>
      <Text style={styles.title}>Fingerprint Reader</Text>
      <Text style={styles.subtitle}>
        Use your device's biometric sensor{"\n"}
        (Touch ID / Face ID / Fingerprint){"\n"}
        to enroll or verify with the reader system.
      </Text>

      <TouchableOpacity
        style={[styles.button, styles.buttonEnroll]}
        onPress={() => navigation.navigate("Enroll")}
      >
        <Text style={styles.buttonText}>Enroll Fingerprint</Text>
      </TouchableOpacity>

      <TouchableOpacity
        style={[styles.button, styles.buttonVerify]}
        onPress={() => navigation.navigate("Verify")}
      >
        <Text style={styles.buttonText}>Verify Fingerprint</Text>
      </TouchableOpacity>
    </View>
  );
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    alignItems: "center",
    justifyContent: "center",
    padding: 24,
    backgroundColor: "#fff",
  },
  title: {
    fontSize: 28,
    fontWeight: "bold",
    marginBottom: 12,
    textAlign: "center",
  },
  subtitle: {
    fontSize: 15,
    color: "#555",
    textAlign: "center",
    marginBottom: 40,
    lineHeight: 22,
  },
  button: {
    width: "80%",
    paddingVertical: 16,
    borderRadius: 10,
    alignItems: "center",
    marginVertical: 8,
  },
  buttonEnroll: { backgroundColor: "#007AFF" },
  buttonVerify: { backgroundColor: "#34C759" },
  buttonText: { color: "#fff", fontSize: 17, fontWeight: "600" },
});
