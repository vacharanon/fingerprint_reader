import React from "react";
import { NavigationContainer } from "@react-navigation/native";
import { createNativeStackNavigator } from "@react-navigation/native-stack";
import { StatusBar } from "expo-status-bar";

import HomeScreen from "./screens/HomeScreen";
import EnrollScreen from "./screens/EnrollScreen";
import VerifyScreen from "./screens/VerifyScreen";

const Stack = createNativeStackNavigator();

export default function App() {
  return (
    <NavigationContainer>
      <StatusBar style="auto" />
      <Stack.Navigator initialRouteName="Home">
        <Stack.Screen
          name="Home"
          component={HomeScreen}
          options={{ title: "Fingerprint Reader" }}
        />
        <Stack.Screen
          name="Enroll"
          component={EnrollScreen}
          options={{ title: "Enroll Fingerprint" }}
        />
        <Stack.Screen
          name="Verify"
          component={VerifyScreen}
          options={{ title: "Verify Fingerprint" }}
        />
      </Stack.Navigator>
    </NavigationContainer>
  );
}
