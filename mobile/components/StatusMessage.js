import React from "react";
import { StyleSheet, Text, View } from "react-native";

/**
 * A colored banner used to show success / error / info messages.
 *
 * Props:
 *   message  {string}              – Text to display (falsy → nothing rendered)
 *   type     {"success"|"error"|"info"}  – Colour variant (default "info")
 */
export default function StatusMessage({ message, type = "info" }) {
  if (!message) return null;

  const containerStyle =
    type === "success"
      ? styles.success
      : type === "error"
      ? styles.error
      : styles.info;

  return (
    <View style={[styles.container, containerStyle]}>
      <Text style={styles.text}>{message}</Text>
    </View>
  );
}

const styles = StyleSheet.create({
  container: {
    borderRadius: 8,
    marginVertical: 12,
    paddingHorizontal: 16,
    paddingVertical: 12,
  },
  success: { backgroundColor: "#d4edda" },
  error: { backgroundColor: "#f8d7da" },
  info: { backgroundColor: "#d1ecf1" },
  text: { fontSize: 14, textAlign: "center" },
});
