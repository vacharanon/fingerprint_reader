
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>

#include "secrets.h"

int status = WL_IDLE_STATUS;

const char *host = HOST;
const char *fingerprint = FINGERPRINT;

WiFiClientSecure client;
HTTPClient http;

void setup()
{
  Serial.begin(115200);
  while (!Serial)
  {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // Serial.println("");
  // Serial.println("ESP8266 board info:");
  // Serial.print("\tChip ID: ");
  // Serial.println(ESP.getFlashChipId());
  // Serial.print("\tCore Version: ");
  // Serial.println(ESP.getCoreVersion());
  // Serial.print("\tChip Real Size: ");
  // Serial.println(ESP.getFlashChipRealSize());
  // Serial.print("\tChip Flash Size: ");
  // Serial.println(ESP.getFlashChipSize());
  // Serial.print("\tChip Flash Speed: ");
  // Serial.println(ESP.getFlashChipSpeed());
  // Serial.print("\tChip Speed: ");
  // Serial.println(ESP.getCpuFreqMHz());
  // Serial.print("\tChip Mode: ");
  // Serial.println(ESP.getFlashChipMode());
  // Serial.print("\tSketch Size: ");
  // Serial.println(ESP.getSketchSize());
  // Serial.print("\tSketch Free Space: ");
  // Serial.println(ESP.getFreeSketchSpace());

  Serial.print("Connecting to SSID: ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  status = WiFi.begin(ssid, password);
  Serial.println("");

  while (status != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
    status = WiFi.status();
  }
  Serial.println("");
  WiFi.setAutoReconnect(true);
  // printWifiStatus();
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  client.setFingerprint(fingerprint);
  Serial.write("[ESP8266]READY\n");
}

void loop()
{
  // if there is data from the 328P
  if (Serial.available() > 0)
  {
    String receivedString = Serial.readStringUntil('\n');
    if (receivedString.indexOf("[F_ID]") > -1)
    {

      // DO NOT PRINT ANYTHING TO SERIAL. THE MCU IS READING
      // Extract the ID from receivedString
      int idStartIndex = receivedString.indexOf("]") + 1;
      String idString = receivedString.substring(idStartIndex);
      // int fingerprintId = idString.toInt();

      if (http.begin(client, host))
      {
        http.addHeader("Content-Type", "application/json");

        // Use the extracted ID in the JSON payload
        String jsonPayload = "{\"api_key\":\"" + String(apiKey) + "\",\"id\":" + idString + "}";
        int httpCode = http.POST(jsonPayload);

        Serial.print("[ESP8266]");
        Serial.print(httpCode);
        Serial.print("\n");
        http.end();
      }
      else
      {
        Serial.println("[ESP8266]Unable to connect");
      }
    }
  }
}

void printWifiStatus()
{

  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}