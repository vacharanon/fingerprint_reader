#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Fingerprint.h>
#include <Keypad_I2C.h>
#include <Keypad.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoOTA.h>
#include "secrets.h"

#define MAX_ID 128
#define BOOT_DELAY 1000
#define FEEDBACK_DELAY 2000
#define RESULT_DELAY 3000
#define LOOP_DELAY 50
#define MODE_SELECT_TIMEOUT 3000
#define ERROR_BLINK_DURATION 3000
#define ERROR_BLINK_INTERVAL 100
#define OTA_HOSTNAME "fingerprint-reader"

const char DIGITS[] = "0123456789";

#define OLED_SCREEN_WIDTH 128     // pixel
#define OLED_SCREEN_HEIGHT 32     // pixel
#define OLED_SCREEN_ADDRESS 0x3C  ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
#define OLED_RESET -1
Adafruit_SSD1306 oled(OLED_SCREEN_WIDTH, OLED_SCREEN_HEIGHT, &Wire, OLED_RESET);

uint8_t id;
// https://docs.espressif.com/projects/arduino-esp32/en/latest/migration_guides/2.x_to_3.0.html#uart-hardwareserial
// Default pins for some SoCs have been changed to avoid conflicts with other peripherals:
// * ESP32’s UART1 RX and TX pins are now GPIO26 and GPIO27, respectively;
// * ESP32’s UART2 RX and TX pins are now GPIO4 and GPIO25, respectively;
// * ESP32-S2’s UART1 RX and TX pins are now GPIO4 and GPIO5, respectively.
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&Serial2);

#define RED_LED 12
#define GREEN_LED 13

// Keypad and OLED are using I2C (SCL/SDA) but different address
// Use i2c_scanner to lookup
const uint8_t KEYPAD_ADDRESS = 0x20;
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  { '1', '2', '3', 'A' },
  { '4', '5', '6', 'B' },
  { '7', '8', '9', 'C' },
  { '*', '0', '#', 'D' }
};
byte rowPins[ROWS] = { 0, 1, 2, 3 };  // เชื่อมต่อกับ Pin แถวของปุ่มกด
byte colPins[COLS] = { 4, 5, 6, 7 };  // เชื่อมต่อกับ Pin คอลัมน์ของปุ่มกด
Keypad_I2C keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS, KEYPAD_ADDRESS, PCF8574);
uint8_t mode = 1;

const char *host = HOST;
bool wifiReady = false;
bool otaInProgress = false;
WiFiClientSecure client;
HTTPClient http;

// Display a single text string on OLED (clears screen first)
void oledShow(const char *text, uint8_t textSize) {
  oled.clearDisplay();
  oled.setTextColor(WHITE);
  oled.setCursor(0, 0);
  oled.setTextSize(textSize);
  oled.print(text);
  oled.display();
}
void oledShow(const String &text, uint8_t textSize) {
  oledShow(text.c_str(), textSize);
}

void setup() {

  Serial.begin(115200);
  Wire.begin();
  if (!oled.begin(SSD1306_SWITCHCAPVCC, OLED_SCREEN_ADDRESS)) {
    Serial.println("SSD1306 allocation failed");
    for (;;)
      ;  // Don't proceed, loop forever
  } else {
    Serial.println("OLED connected");
  }

  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  digitalWrite(RED_LED, HIGH);
  digitalWrite(GREEN_LED, LOW);

  oled.clearDisplay();
  oled.setTextColor(WHITE);
  oled.setTextSize(3);
  oled.setCursor(0, 8);
  oled.setTextSize(3);
  oled.println("PALO IT");
  oled.display();
  delay(BOOT_DELAY);

  // Connect WiFi early so OTA is available in all modes
  oledShow("WiFi...", 2);
  Serial.print("Connecting to SSID: ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  unsigned long wifiStart = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    digitalWrite(RED_LED, !digitalRead(RED_LED));
    if (millis() - wifiStart > 15000) {
      Serial.println("\nWiFi connection timed out — continuing without WiFi");
      break;
    }
  }
  if (WiFi.status() == WL_CONNECTED) {
    WiFi.setAutoReconnect(true);
    Serial.print("\nSSID: ");
    Serial.println(WiFi.SSID());
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    wifiReady = true;

    // TODO: Replace with client.setCACert(root_ca) for production
    client.setInsecure();  // WARNING: disables SSL verification — PoC only

    // OTA setup
    ArduinoOTA.setHostname(OTA_HOSTNAME);
    ArduinoOTA.onStart([]() {
      otaInProgress = true;
      oledShow("OTA Update", 2);
      Serial.println("OTA start");
    });
    ArduinoOTA.onEnd([]() {
      otaInProgress = false;
      oledShow("OTA Done!", 2);
      Serial.println("OTA end");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
      unsigned int pct = progress / (total / 100);
      oled.clearDisplay();
      oled.setTextColor(WHITE);
      oled.setCursor(0, 0);
      oled.setTextSize(2);
      oled.println("Updating");
      oled.setTextSize(3);
      oled.print(pct);
      oled.println("%");
      oled.display();
      Serial.printf("OTA Progress: %u%%\r", pct);
    });
    ArduinoOTA.onError([](ota_error_t error) {
      otaInProgress = false;
      Serial.printf("OTA Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) printError("OTA Auth Fail", 2);
      else if (error == OTA_BEGIN_ERROR) printError("OTA Begin Fail", 2);
      else if (error == OTA_CONNECT_ERROR) printError("OTA Connect Fail", 2);
      else if (error == OTA_RECEIVE_ERROR) printError("OTA Recv Fail", 2);
      else if (error == OTA_END_ERROR) printError("OTA End Fail", 2);
    });
    ArduinoOTA.begin();
    Serial.println("OTA ready — hostname: " + String(OTA_HOSTNAME));
  }

  finger.begin(57600);

  if (finger.verifyPassword()) {
    Serial.println("Fingerprint reader ready");
  } else {
    Serial.println("Did not find fingerprint sensor");
    while (1) {
      delay(1);
    }
  }

  keypad.begin(makeKeymap(keys));

  uint32_t start = millis();
  while (1) {
    uint32_t now = millis();
    int secondsLeft = 3 - ((now - start) / 1000);
    if (secondsLeft < 0) secondsLeft = 0;
    oled.clearDisplay();
    oled.setCursor(0, 0);
    oled.setTextSize(2);
    oled.println("MODE");
    oled.setTextSize(1);
    oled.println("A) Reader B) Enroll");
    oled.println("C) Delete D) Status");
    oled.print("Autoselect in ");
    oled.print(secondsLeft);
    oled.display();
    if (now - start >= MODE_SELECT_TIMEOUT) {
      break;
    }
    char ch = keypad.getKey();
    if (ch) {
      if (ch == 'A') {
        mode = 1;
        break;
      } else if (ch == 'B') {
        mode = 2;
        break;
      } else if (ch == 'C') {
        mode = 3;
        break;
      } else if (ch == 'D') {
        mode = 4;
        break;
      }
    }
  }
  if (mode == 1) {
    oledShow("READER", 3);
  } else if (mode == 2) {
    oledShow("ENROLL", 3);
  } else if (mode == 3) {
    oledShow("DELETE", 3);
  } else if (mode == 4) {
    oledShow("STATUS", 3);
  }
}

void loop()  // run over and over again
{
  // Handle OTA in every loop iteration, regardless of mode
  if (wifiReady) {
    ArduinoOTA.handle();
  }
  if (otaInProgress) return;  // pause normal operation during OTA

  if (mode == 1) {
    String receivedString;
    while (!wifiReady) {
      // WiFi failed at boot — retry once
      Serial.print("Connecting to SSID: ");
      Serial.println(ssid);
      WiFi.mode(WIFI_STA);
      WiFi.begin(ssid, password);
      while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
        digitalWrite(RED_LED, !digitalRead(RED_LED));
      }
      Serial.println("");
      WiFi.setAutoReconnect(true);
      Serial.print("SSID: ");
      Serial.println(WiFi.SSID());

      client.setInsecure();
      wifiReady = true;
    }
    if (wifiReady) {
      if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi no longer connected. Attempting to reconnect...");
        WiFi.disconnect();
        WiFi.reconnect();
        oledShow("Reconnecting", 3);
        delay(FEEDBACK_DELAY);  
        if (WiFi.status() != WL_CONNECTED) { 
          Serial.println("WiFi connection still unavailable.");
          return;   
        }
        Serial.println("WiFi connection reestablished.");
      }
    }
    digitalWrite(RED_LED, LOW);
    digitalWrite(GREEN_LED, HIGH);
    receivedString = "";
    oledShow("Ready", 3);

    uint8_t p = finger.getImage();
    if (p != FINGERPRINT_OK) return;

    p = finger.image2Tz();
    if (p != FINGERPRINT_OK) return;

    p = finger.fingerFastSearch();
    if (p != FINGERPRINT_OK) return;

    int id = finger.fingerID;
    if (id > 0) {
      digitalWrite(RED_LED, HIGH);
      digitalWrite(GREEN_LED, LOW);
      oled.clearDisplay();
      oled.setTextColor(WHITE);
      oled.setCursor(0, 0);
      oled.setTextSize(3);
      oled.print("ID=");
      oled.println(id);
      oled.setTextSize(1);
      oled.print("Confidence: ");
      oled.println(finger.confidence);
      oled.display();
      Serial.print("[F_ID]");
      Serial.print(id);
      Serial.print(" conf=");
      Serial.println(finger.confidence);
      delay(BOOT_DELAY);

      oledShow("Sending", 3);

      if (http.begin(client, host)) {
        http.addHeader("Content-Type", "application/json");
        http.addHeader("Authorization", "Bearer " + String(apiKey));

        String jsonPayload = "{\"id\":" + String(id) + "}";
        int httpCode = http.POST(jsonPayload);

        Serial.println(httpCode);
        
        if (httpCode == 200) {
          oledShow("OK", 3);
          delay(FEEDBACK_DELAY);  
        } else {
         switch (httpCode) {
            case HTTPC_ERROR_CONNECTION_REFUSED:
              printError("Connection refused", 2);
              break;
            case HTTPC_ERROR_SEND_HEADER_FAILED:
              printError("Send header failed", 2);
              break;
            case HTTPC_ERROR_SEND_PAYLOAD_FAILED:
              printError("Send payload failed", 2);
              break;
            case HTTPC_ERROR_NOT_CONNECTED:
              printError("Not connected", 2);
              break;
            case HTTPC_ERROR_CONNECTION_LOST:
              printError("Connection lost", 2);
              break;
            case HTTPC_ERROR_NO_STREAM:
              printError("No stream", 2);
              break;
            case HTTPC_ERROR_NO_HTTP_SERVER:
              printError("No HTTP server", 2);
              break;
            case HTTPC_ERROR_TOO_LESS_RAM:
              printError("Too less RAM", 2);
              break;
            case HTTPC_ERROR_ENCODING:
              printError("Encoding error", 2);
              break;
            case HTTPC_ERROR_STREAM_WRITE:
              printError("Stream write error", 2);
              break;
            case HTTPC_ERROR_READ_TIMEOUT:
              printError("Read timeout", 2);
              break;
            default:
              printError(String(httpCode), 3);
          }
        }
        http.end();
      }

      digitalWrite(RED_LED, LOW);
      digitalWrite(GREEN_LED, HIGH);
    }
  } else if (mode == 2) {
    digitalWrite(RED_LED, HIGH);
    digitalWrite(GREEN_LED, LOW);
    String receivedString;
    while (1) {
      if (receivedString != "") {
        oled.clearDisplay();
        oled.setTextColor(WHITE);
        oled.setCursor(0, 0);
        oled.setTextSize(3);
        oled.print(receivedString);
        oled.display();
      } else {
        oled.clearDisplay();
        oled.setTextColor(WHITE);
        oled.setCursor(0, 0);
        oled.setTextSize(1);
        oled.print("Type in the ID then #");
        oled.display();
      }
      char ch = keypad.getKey();
      if (ch) {
        if (ch == '#') {
          id = receivedString.toInt();
          if (id == 0 || id > MAX_ID) {  // ID #0 not allowed, try again!
            oledShow("Invalid", 3);
            delay(BOOT_DELAY);
            break;
          } else {
            int p = -1;
            oled.clearDisplay();
            oled.setTextColor(WHITE);
            oled.setCursor(0, 0);
            oled.setTextSize(1);
            oled.print("ID=");
            oled.println(id);
            oled.setTextSize(2);
            oled.println("Scan #1");
            oled.display();
            while (p != FINGERPRINT_OK) {
              p = finger.getImage();
              switch (p) {
                case FINGERPRINT_OK:
                  // Serial.println("Image taken");
                  break;
                case FINGERPRINT_NOFINGER:
                  // Serial.print(".");
                  break;
                case FINGERPRINT_PACKETRECIEVEERR:
                  printError("Communication error");
                  return;
                case FINGERPRINT_IMAGEFAIL:
                  printError("Imaging error");
                  return;
                default:
                  printError("Unknown error");
                  return;
                  // break;
              }
            }

            // OK success!
            p = finger.image2Tz(1);
            switch (p) {
              case FINGERPRINT_OK:
                // Serial.println("Image converted");
                break;
              case FINGERPRINT_IMAGEMESS:
                printError("Image too messy");
                return;
              case FINGERPRINT_PACKETRECIEVEERR:
                printError("Communication error");
                return;
              case FINGERPRINT_FEATUREFAIL:
                printError("Could not find fingerprint features");
                return;
              case FINGERPRINT_INVALIDIMAGE:
                printError("Could not find fingerprint features");
                return;
              default:
                printError("Unknown error");
                return;
            }

            oledShow("Remove", 2);
            delay(FEEDBACK_DELAY);
            p = 0;
            while (p != FINGERPRINT_NOFINGER) {
              p = finger.getImage();
            }

            p = -1;
            oled.clearDisplay();
            oled.setTextColor(WHITE);
            oled.setCursor(0, 0);
            oled.setTextSize(1);
            oled.print("ID=");
            oled.println(id);
            oled.setTextSize(2);
            oled.println("Scan #2");
            oled.display();
            while (p != FINGERPRINT_OK) {
              p = finger.getImage();
              switch (p) {
                case FINGERPRINT_OK:
                  Serial.println("Image taken");
                  break;
                case FINGERPRINT_NOFINGER:
                  // Serial.print(".");
                  break;
                case FINGERPRINT_PACKETRECIEVEERR:
                  printError("Communication error");
                  return;
                case FINGERPRINT_IMAGEFAIL:
                  printError("Imaging error");
                  return;
                default:
                  printError("Unknown error");
                  return;
              }
            }

            p = finger.image2Tz(2);
            switch (p) {
              case FINGERPRINT_OK:
                Serial.println("Image converted");
                break;
              case FINGERPRINT_IMAGEMESS:
                printError("Image too messy");
                return;
              case FINGERPRINT_PACKETRECIEVEERR:
                printError("Communication error");
                return;
              case FINGERPRINT_FEATUREFAIL:
                printError("Could not find fingerprint features");
                return;
              case FINGERPRINT_INVALIDIMAGE:
                printError("Could not find fingerprint features");
                return;
              default:
                printError("Unknown error");
                return;
            }

            p = finger.createModel();
            if (p == FINGERPRINT_OK) {
              Serial.println("Prints matched!");
            } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
              printError("Communication error");
              return;
            } else if (p == FINGERPRINT_ENROLLMISMATCH) {
              printError("Fingerprints did not match");
              return;
            } else {
              printError("Unknown error");
              return;
            }

            p = finger.storeModel(id);
            if (p == FINGERPRINT_OK) {
              Serial.println("Stored!");
            } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
              printError("Communication error");
              return;
            } else if (p == FINGERPRINT_BADLOCATION) {
              printError("Could not store in that location");
              return;
            } else if (p == FINGERPRINT_FLASHERR) {
              printError("Error writing to flash");
              return;
            } else {
              printError("Unknown error");
              return;
            }

            receivedString = "";
            oled.clearDisplay();
            oled.setTextColor(WHITE);
            oled.setCursor(0, 0);
            oled.setTextSize(1);
            oled.print("ID=");
            oled.println(id);
            oled.setTextSize(2);
            oled.println("DONE");
            oled.display();
            delay(RESULT_DELAY);
            break;
          }
        } else if (strchr(DIGITS, ch)) {
          if (receivedString.length() < 3) { // max 3 digits (IDs 1-128)
            receivedString += ch;
          }
        }
      }
    }
  } else if (mode == 3) {
    digitalWrite(RED_LED, HIGH);
    digitalWrite(GREEN_LED, LOW);
    String receivedString;
    while (1) {
      if (receivedString != "") {
        oled.clearDisplay();
        oled.setTextColor(WHITE);
        oled.setCursor(0, 0);
        oled.setTextSize(3);
        oled.print(receivedString);
        oled.display();
      } else {
        oled.clearDisplay();
        oled.setTextColor(WHITE);
        oled.setCursor(0, 0);
        oled.setTextSize(1);
        oled.print("Type in the ID to delete then #");
        oled.display();
      }
      char ch = keypad.getKey();
      if (ch) {
        if (ch == '#') {
          id = receivedString.toInt();
          if (id == 0 || id > MAX_ID) {  // ID #0 not allowed, try again!
            oledShow("Invalid", 3);
            delay(BOOT_DELAY);
            break;
          } else {
            int p = finger.deleteModel(id);
            if (p == FINGERPRINT_OK) {
              Serial.println("Deleted!");
              oledShow("Deleted", 2);
              delay(FEEDBACK_DELAY);
            } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
              printError("Communication error");
              return;
            } else if (p == FINGERPRINT_BADLOCATION) {
              printError("Could not delete in that location");
              return;
            } else if (p == FINGERPRINT_FLASHERR) {
              printError("Error writing to flash");
              return;
            } else {
              printError("Unknown error");
              return;
            }

            receivedString = "";
            break;
          }
        } else if (strchr(DIGITS, ch)) {
          if (receivedString.length() < 3) { // max 3 digits (IDs 1-128)
            receivedString += ch;
          }
        }
      }
    }
  } else if (mode == 4) {
    digitalWrite(RED_LED, HIGH);
    digitalWrite(GREEN_LED, LOW);

    int p = finger.getTemplateCount();
    if (p == FINGERPRINT_OK) {
      Serial.print("Sensor contains ");
      Serial.print(finger.templateCount);
      Serial.println(" templates");
      oled.clearDisplay();
      oled.setTextColor(WHITE);
      oled.setCursor(0, 0);
      oled.setTextSize(2);
      oled.print("Templates:");
      oled.print(finger.templateCount);
      oled.print("/");
      oled.println(MAX_ID);
      oled.display();
      delay(RESULT_DELAY);
    } else {
      printError("Could not get template count");
      return;
    }
  }
  delay(LOOP_DELAY);

}
void printError(String error) {
  printError(error, 1);
}
void printError(String error, uint8_t textSize) {
  oledShow(error, textSize);
  // Blink rapidly for error indication
  unsigned long blinkStart = millis();
  while (millis() - blinkStart < ERROR_BLINK_DURATION) {
    digitalWrite(RED_LED, HIGH);
    delay(ERROR_BLINK_INTERVAL);
    digitalWrite(RED_LED, LOW);
    delay(ERROR_BLINK_INTERVAL);
  }
}