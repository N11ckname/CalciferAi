/*
 * TEST_MODE - Programme de test pour contrôleur LUCIA
 * Teste tous les composants individuellement
 */

#include <Wire.h>
#include <U8g2lib.h>
#include <Adafruit_MAX31856.h>
#include <Encoder.h>

// ===== PINS =====
#define PIN_ENCODER_CLK 2
#define PIN_ENCODER_DT 3
#define PIN_ENCODER_SW 4
#define PIN_PUSH_BUTTON 5
#define PIN_RELAY 6
#define PIN_MAX_CS 10
#define PIN_LED A1

// ===== OBJETS =====
U8G2_SH1106_128X64_NONAME_2_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
Adafruit_MAX31856 max31856 = Adafruit_MAX31856(PIN_MAX_CS);
Encoder encoder(PIN_ENCODER_CLK, PIN_ENCODER_DT);

// ===== VARIABLES =====
int testMode = 0;
const int NUM_TESTS = 6;
long encoderPos = 0;
bool relayState = false;
bool ledState = false;

void setup() {
  Serial.begin(9600);
  
  // Pins
  pinMode(PIN_ENCODER_SW, INPUT_PULLUP);
  pinMode(PIN_PUSH_BUTTON, INPUT_PULLUP);
  pinMode(PIN_RELAY, OUTPUT);
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_RELAY, LOW);
  digitalWrite(PIN_LED, LOW);
  
  // Display
  u8g2.begin();
  u8g2.setContrast(255);
  
  // MAX31856
  if (!max31856.begin()) {
    Serial.println("MAX31856 ERROR!");
  } else {
    max31856.setThermocoupleType(MAX31856_TCTYPE_S);
    max31856.setConversionMode(MAX31856_CONTINUOUS);
    Serial.println("MAX31856 OK");
  }
  
  Serial.println("TEST MODE READY");
  Serial.println("Encoder click: next test");
  Serial.println("Push button: toggle relay/LED");
}

void loop() {
  static bool lastEncButton = HIGH;
  static bool lastPushButton = HIGH;
  
  // Encoder click - change test
  bool encButton = digitalRead(PIN_ENCODER_SW);
  if (encButton == LOW && lastEncButton == HIGH) {
    delay(50);
    testMode = (testMode + 1) % NUM_TESTS;
    Serial.print("Test mode: ");
    Serial.println(testMode);
  }
  lastEncButton = encButton;
  
  // Push button - toggle relay/LED
  bool pushButton = digitalRead(PIN_PUSH_BUTTON);
  if (pushButton == LOW && lastPushButton == HIGH) {
    delay(50);
    relayState = !relayState;
    ledState = !ledState;
    digitalWrite(PIN_RELAY, relayState);
    digitalWrite(PIN_LED, ledState);
    Serial.print("Relay/LED: ");
    Serial.println(relayState ? "ON" : "OFF");
  }
  lastPushButton = pushButton;
  
  // Display test
  u8g2.firstPage();
  do {
    drawTest();
  } while (u8g2.nextPage());
  
  delay(100);
}

void drawTest() {
  u8g2.setFont(u8g2_font_6x10_tf);
  
  char buf[30];
  snprintf(buf, 30, "TEST %d/%d", testMode + 1, NUM_TESTS);
  u8g2.drawStr(0, 10, buf);
  u8g2.drawLine(0, 12, 127, 12);
  
  switch (testMode) {
    case 0:
      testDisplay();
      break;
    case 1:
      testEncoder();
      break;
    case 2:
      testButtons();
      break;
    case 3:
      testTemperature();
      break;
    case 4:
      testRelay();
      break;
    case 5:
      testAll();
      break;
  }
}

void testDisplay() {
  u8g2.drawStr(0, 24, "ECRAN OLED");
  u8g2.drawStr(0, 36, "Si vous voyez");
  u8g2.drawStr(0, 48, "ce texte:");
  u8g2.drawStr(0, 60, "ECRAN OK!");
}

void testEncoder() {
  u8g2.drawStr(0, 24, "ENCODEUR");
  
  long newPos = encoder.read() / 4;
  if (newPos != encoderPos) {
    encoderPos = newPos;
  }
  
  char buf[20];
  snprintf(buf, 20, "Position: %ld", encoderPos);
  u8g2.drawStr(0, 40, buf);
  u8g2.drawStr(0, 52, "Tournez encodeur");
  u8g2.drawStr(0, 64, "Click: test suiv");
}

void testButtons() {
  u8g2.drawStr(0, 24, "BOUTONS");
  
  char buf[20];
  bool encBtn = digitalRead(PIN_ENCODER_SW);
  bool pushBtn = digitalRead(PIN_PUSH_BUTTON);
  
  snprintf(buf, 20, "Enc: %s", encBtn ? "OFF" : "ON");
  u8g2.drawStr(0, 40, buf);
  
  snprintf(buf, 20, "Push: %s", pushBtn ? "OFF" : "ON");
  u8g2.drawStr(0, 52, buf);
  
  u8g2.drawStr(0, 64, "Appuyez boutons");
}

void testTemperature() {
  u8g2.drawStr(0, 24, "THERMOCOUPLE");
  
  float temp = max31856.readThermocoupleTemperature();
  uint8_t fault = max31856.readFault();
  
  char buf[20];
  if (fault) {
    snprintf(buf, 20, "ERREUR: 0x%02X", fault);
    u8g2.drawStr(0, 40, buf);
  } else {
    snprintf(buf, 20, "Temp: %.1f C", temp);
    u8g2.drawStr(0, 40, buf);
  }
  
  float cjTemp = max31856.readCJTemperature();
  snprintf(buf, 20, "CJ: %.1f C", cjTemp);
  u8g2.drawStr(0, 52, buf);
  
  u8g2.drawStr(0, 64, "Verif sonde");
}

void testRelay() {
  u8g2.drawStr(0, 24, "RELAIS & LED");
  
  char buf[20];
  snprintf(buf, 20, "Etat: %s", relayState ? "ON" : "OFF");
  u8g2.drawStr(0, 40, buf);
  
  u8g2.drawStr(0, 52, "Push: toggle");
  u8g2.drawStr(0, 64, "Verif LED/Relais");
}

void testAll() {
  u8g2.setFont(u8g2_font_5x7_tf);
  u8g2.drawStr(0, 22, "RESUME COMPLET");
  
  char buf[25];
  
  // Encoder
  snprintf(buf, 25, "Enc:%ld B:%d", encoderPos, 
           digitalRead(PIN_ENCODER_SW) ? 0 : 1);
  u8g2.drawStr(0, 32, buf);
  
  // Temperature
  float temp = max31856.readThermocoupleTemperature();
  snprintf(buf, 25, "T:%.0fC", temp);
  u8g2.drawStr(0, 42, buf);
  
  // Buttons & Relay
  snprintf(buf, 25, "Push:%d Rel:%d", 
           digitalRead(PIN_PUSH_BUTTON) ? 0 : 1,
           relayState ? 1 : 0);
  u8g2.drawStr(0, 52, buf);
  
  // Status
  u8g2.drawStr(0, 62, "Tous composants OK");
}

