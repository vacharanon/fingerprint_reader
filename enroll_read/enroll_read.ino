#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>
#include <Keypad_I2C.h>
#include <Keypad.h>

// OLED 128x32 I2C SSD1306 Chip
// Same wiring as LiquidCrystal_I2C
// OLED -> Arduino
// SCL -> A5
// SDA -> A4
// GND -> GND
// VCC -> 5V
#define OLED_SCREEN_WIDTH 128     // pixel
#define OLED_SCREEN_HEIGHT 32     // pixel
#define OLED_SCREEN_ADDRESS 0x3C  ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
#define OLED_RESET -1
Adafruit_SSD1306 oled(OLED_SCREEN_WIDTH, OLED_SCREEN_HEIGHT, &Wire, OLED_RESET);

// Fingerprint Sensor AS608 -> Arduino
// VCC -> 3.3V
// GND -> GND
// RX -> 3
// TX -> 2
SoftwareSerial mySerial(2, 3);
uint8_t id;

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

#define RED_LED 12
#define GREEN_LED 13
// text sizes
// 1 = 8px
// 3 = 16px

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
bool wifiReady = false;

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
  delay(1000);  // Pause for 2 seconds

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
    oled.println("A=reader,B=enroll");
    oled.print("Autoselect in ");
    oled.print(secondsLeft);
    oled.display();
    if (now - start >= 3000) {
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
      }
    }
  }
  if (mode == 1) {
    oled.clearDisplay();
    oled.setTextColor(WHITE);
    oled.setCursor(0, 0);
    oled.setTextSize(3);
    oled.println("READER");
    oled.display();
  } else if (mode == 2) {
    oled.clearDisplay();
    oled.setTextColor(WHITE);
    oled.setCursor(0, 0);
    oled.setTextSize(3);
    oled.println("ENROLL");
    oled.display();
  }
}

void loop()  // run over and over again
{
  if (mode == 1) {
    String receivedString;
    while (!wifiReady) {
      if (Serial.available() > 0) {
        char ch = Serial.read();
        if (ch == '.' && receivedString.length() > 10) {
          receivedString = "";
        }
        receivedString += ch;
        oled.clearDisplay();
        oled.setTextColor(WHITE);
        oled.setCursor(0, 0);
        oled.setTextSize(1);
        oled.println(receivedString);
        oled.display();
        if (receivedString.indexOf("[ESP8266]READY\n") > -1) {
          wifiReady = true;
          delay(1000); // if you wanna read ssid
          break;
        }
      }
      delay(50);
    }
    digitalWrite(RED_LED, LOW);
    digitalWrite(GREEN_LED, HIGH);
    receivedString = "";
    oled.clearDisplay();
    oled.setTextColor(WHITE);
    oled.setCursor(0, 0);
    oled.setTextSize(3);
    oled.println("Ready");
    oled.display();

    uint8_t p = finger.getImage();
    if (p != FINGERPRINT_OK) return -1;

    p = finger.image2Tz();
    if (p != FINGERPRINT_OK) return -1;

    p = finger.fingerFastSearch();
    if (p != FINGERPRINT_OK) return -1;

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
      oled.display();
      Serial.print("[F_ID]");
      Serial.print(id);
      Serial.print("\n");
      delay(1000);

      oled.clearDisplay();
      oled.setTextColor(WHITE);
      oled.setCursor(0, 0);
      oled.setTextSize(3);
      oled.print("Sending");
      oled.display();
      while (Serial.available() == 0) {}  //wait for data available

      receivedString = Serial.readStringUntil('\n');
      oled.clearDisplay();
      oled.setTextColor(WHITE);
      oled.setCursor(0, 0);
      oled.setTextSize(2);
      oled.print(receivedString);
      oled.display();
      delay(3000);
      digitalWrite(RED_LED, LOW);
      digitalWrite(GREEN_LED, HIGH);
    }
    delay(50);
  } else if (mode == 2) {
    digitalWrite(RED_LED, LOW);
    digitalWrite(GREEN_LED, HIGH);
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
          if (id == 0 || id > 128) {  // ID #0 not allowed, try again!
            oled.clearDisplay();
            oled.setTextColor(WHITE);
            oled.setCursor(0, 0);
            oled.setTextSize(3);
            oled.print("Invalid");
            oled.display();
            delay(1000);
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
                // case FINGERPRINT_PACKETRECIEVEERR:
                //   Serial.println("Communication error");
                //   break;
                // case FINGERPRINT_IMAGEFAIL:
                //   Serial.println("Imaging error");
                //   break;
                default:
                  printError("Unknown error");
                  return p;
                  // break;
              }
            }

            // OK success!
            p = finger.image2Tz(1);
            switch (p) {
              case FINGERPRINT_OK:
                Serial.println("Image converted");
                break;
              default:
                printError("Unknown error");
                return p;
            }

            oled.clearDisplay();
            oled.setTextColor(WHITE);
            oled.setCursor(0, 0);
            oled.setTextSize(2);
            oled.println("Remove");
            oled.display();
            delay(2000);
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
                // case FINGERPRINT_PACKETRECIEVEERR:
                //   Serial.println("Communication error");
                //   return;
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
                // Serial.println("Image converted");
                break;
              default:
                printError("Unknown error");
                return p;
            }

            p = finger.createModel();
            // if (p == FINGERPRINT_OK) {
            //   // Serial.println("Prints matched!");
            // } else {
            //   printError("Unknown error");
            //   return p;
            // }

            if (p != FINGERPRINT_OK) {
              printError("Unknown error");
              return p;
            }

            p = finger.storeModel(id);
            if (p != FINGERPRINT_OK) {
              printError("Unknown error");
              return p;
            }
            // if (p == FINGERPRINT_OK) {
            //   // Serial.println("Stored!");
            // } else {
            //   printError("Unknown error");
            //   return p;
            // }

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
            delay(3000);
            break;
          }
        } else if (ch == '0' || ch == '1' || ch == '2' || ch == '3' || ch == '4' || ch == '5' || ch == '6' || ch == '7' || ch == '8' || ch == '9') {
          receivedString += ch;
        }
      }
    }
    delay(50);
  }
}

void printError(String error) {
  oled.clearDisplay();
  oled.setTextColor(WHITE);
  oled.setCursor(0, 0);
  oled.setTextSize(1);
  oled.print(error);
  oled.display();
  delay(3000);
}
