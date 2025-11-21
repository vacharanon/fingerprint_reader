#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Fingerprint.h>

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

uint8_t id;

// https://docs.espressif.com/projects/arduino-esp32/en/latest/migration_guides/2.x_to_3.0.html#uart-hardwareserial
// Default pins for some SoCs have been changed to avoid conflicts with other peripherals: 
// * ESP32’s UART1 RX and TX pins are now GPIO26 and GPIO27, respectively; 
// * ESP32’s UART2 RX and TX pins are now GPIO4 and GPIO25, respectively; 
// * ESP32-S2’s UART1 RX and TX pins are now GPIO4 and GPIO5, respectively.
Adafruit_Fingerprint finger = Adafruit_Fingerprint(& Serial2);

// #define RED_LED 12
// #define GREEN_LED 13
// text sizes
// 1 = 8px
// 3 = 16px

bool wifiReady = true;

void setup() {
  Serial.begin(115200);
  // pinMode(RED_LED, OUTPUT);
  // pinMode(GREEN_LED, OUTPUT);
  // digitalWrite(RED_LED, HIGH);
  // digitalWrite(GREEN_LED, LOW);
  if (!oled.begin(SSD1306_SWITCHCAPVCC, OLED_SCREEN_ADDRESS)) {
    Serial.println("SSD1306 allocation failed");
    for (;;)
      ;  // Don't proceed, loop forever
  } else {
    Serial.println("OLED connected");
  }

  oled.clearDisplay();
  oled.setTextColor(WHITE);
  oled.setTextSize(3);
  oled.setCursor(0, 8);
  oled.setTextSize(3);
  oled.println("PALO IT");
  oled.display();
  delay(2000);  // Pause for 2 seconds

  finger.begin(57600);

  if (finger.verifyPassword()) {
    Serial.println("Fingerprint reader ready");
  } else {
    Serial.println("Did not find fingerprint sensor");
    while (1) {
      delay(1);
    }
  }
}

String receivedString;
void loop()  // run over and over again
{
  receivedString = "";
  while (!wifiReady) {
    if (Serial.available() > 0) {
      char inChar = Serial.read();
      if (inChar == '.' && receivedString.length() > 10) {
        receivedString = ""; 
      }
      receivedString += inChar;
      oled.clearDisplay();
      oled.setTextColor(WHITE);
      oled.setCursor(0, 0);
      oled.setTextSize(1);
      oled.println(receivedString);
      oled.display();
      if (receivedString.indexOf("[ESP8266]READY\n") > -1) {
        wifiReady = true;
        delay(1000);
      }
    }
  }
  // digitalWrite(RED_LED, LOW);
  // digitalWrite(GREEN_LED, HIGH);
  receivedString = "";
  oled.clearDisplay();
  oled.setTextColor(WHITE);
  oled.setCursor(0, 0);
  oled.setTextSize(3);
  oled.println("Ready");
  oled.display();

  int id = getFingerprintIDez();
  if (id > 0) {
    // digitalWrite(RED_LED, HIGH);
    // digitalWrite(GREEN_LED, LOW);
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
    while (Serial.available() == 0) {} //wait for data available

    receivedString = Serial.readStringUntil('\n');
    oled.clearDisplay();
    oled.setTextColor(WHITE);
    oled.setCursor(0, 0);
    oled.setTextSize(2);
    oled.print(receivedString);
    oled.display();
    delay(3000);
    // digitalWrite(RED_LED, LOW);
    // digitalWrite(GREEN_LED, HIGH);
  }
  delay(50);  //don't ned to run this at full speed.
}

// returns -1 if failed, otherwise returns ID #
int getFingerprintIDez() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK) return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK) return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK) return -1;

  // found a match!
  Serial.print("Found ID #");
  Serial.print(finger.fingerID);
  Serial.print(" with confidence of ");
  Serial.println(finger.confidence);
  return finger.fingerID;
}
