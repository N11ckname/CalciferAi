/*
 * LUCIA - Contrôleur de four céramique
 * Système de thermostat intelligent pour four électrique
 */

#include <Wire.h>
#include <U8g2lib.h>
#include <Adafruit_MAX31856.h>
#include <Encoder.h>
#include <EEPROM.h>
#include "definitions.h"
#include "display.h"
#include "temperature.h"

// ===== PINS DEFINITION =====
#define PIN_ENCODER_CLK 2
#define PIN_ENCODER_DT 3
#define PIN_ENCODER_SW 4
#define PIN_PUSH_BUTTON 5
#define PIN_RELAY 6
#define PIN_MAX_CS 10
#define PIN_LED A1

// ===== GLOBAL OBJECTS =====
// Utilise mode 2-page au lieu de full buffer pour économiser ~768 octets de RAM
U8G2_SH1106_128X64_NONAME_2_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
Adafruit_MAX31856 max31856 = Adafruit_MAX31856(PIN_MAX_CS);
Encoder encoder(PIN_ENCODER_CLK, PIN_ENCODER_DT);

// ===== PROGRAM STATE =====
ProgramState progState = PROG_OFF;
Phase currentPhase = PHASE_0;

// ===== FIRING PROGRAM PARAMETERS =====
FiringParams params = {100, 50, 5, 570, 250, 15, 1100, 200, 20, 150, 200};

// ===== UI PARAMETERS =====
EditMode editMode = NAV_MODE;
int selectedParam = 0;
const int NUM_PARAMS = 11;
long encoderPosition = 0;

// ===== TIMING VARIABLES =====
unsigned long lastDisplayUpdate = 0;
unsigned long programStartTime = 0;
unsigned long phaseStartTime = 0;
unsigned long plateauStartTime = 0;
unsigned long tempFailStartTime = 0;
bool tempFailActive = false;
#ifdef ENABLE_GRAPH
unsigned long lastGraphUpdate = 0;
#define GRAPH_UPDATE_INTERVAL 30000  // 30 secondes entre chaque point
#endif

// ===== BUTTON STATES =====
bool lastEncoderButton = HIGH;
bool lastPushButton = HIGH;
unsigned long encoderButtonPressTime = 0;
bool longPressHandled = false;
bool showGraph = false;

// ===== TEMPERATURE CONTROL =====
float targetTemp = 0;
bool plateauReached = false;

// ===== EEPROM ADDRESSES =====
#define EEPROM_MAGIC 0x4C55  // "LU" magic number to detect first use
#define EEPROM_ADDR_MAGIC 0
#define EEPROM_ADDR_PARAMS 2

// ===== GRAPH DATA =====
// Désactivé pour économiser RAM sur Arduino Uno (512 octets)
// Pour activer : décommentez la ligne ci-dessous (nécessite Arduino Mega ou optimisations)
// #define ENABLE_GRAPH
#ifdef ENABLE_GRAPH
  #define GRAPH_SIZE 64
  float graphActual[GRAPH_SIZE];
  float graphTarget[GRAPH_SIZE];
  int graphIndex = 0;
#else
  int graphIndex = 0;  // Dummy variable
#endif

void setup() {
  // Initialize serial for debugging
  Serial.begin(9600);
  
  // Initialize pins
  pinMode(PIN_ENCODER_SW, INPUT_PULLUP);
  pinMode(PIN_PUSH_BUTTON, INPUT_PULLUP);
  pinMode(PIN_RELAY, OUTPUT);
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_RELAY, LOW);
  digitalWrite(PIN_LED, LOW);
  
  // Initialize display
  u8g2.begin();
  u8g2.setContrast(255);
  
  // Initialize MAX31856
  if (!max31856.begin()) {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(0, 20, "MAX31856 Error!");
    u8g2.sendBuffer();
    while (1) delay(10);
  }
  
  max31856.setThermocoupleType(MAX31856_TCTYPE_S);
  max31856.setConversionMode(MAX31856_CONTINUOUS);
  
  // Load parameters from EEPROM
  loadFromEEPROM();
  
  // Initialize temperature control
  initTemperatureControl();
  
  // Initial display
  updateDisplay();
}

void loop() {
  unsigned long currentMillis = millis();
  
  // Read temperature
  float temp = readTemperature();
  
  // Check for temperature reading failure
  if (isnan(temp) || temp < -100 || temp > 1500) {
    if (!tempFailActive) {
      tempFailActive = true;
      tempFailStartTime = currentMillis;
    } else if (currentMillis - tempFailStartTime > 120000) { // 2 minutes
      // Critical error - stop heating
      if (progState == PROG_ON) {
        progState = PROG_OFF;
        currentPhase = PHASE_0;
        setRelay(false);
      }
      displayCriticalError();
      waitForButtonPress();
      tempFailActive = false;
      return;
    }
  } else {
    tempFailActive = false;
  }
  
  // Handle buttons
  handleButtons(currentMillis);
  
  // Handle encoder
  if (progState == PROG_OFF && !showGraph) {
    handleEncoder();
  }
  
  // Update program state
  if (progState == PROG_ON) {
    updateProgram(currentMillis, temp);
    #ifdef ENABLE_GRAPH
    updateGraphData(currentMillis, temp);
    #endif
  }
  
  // Update display
  if (currentMillis - lastDisplayUpdate >= 100) {
    lastDisplayUpdate = currentMillis;
    updateDisplay();
  }
  
  // Update temperature control
  updateTemperatureControl(temp, targetTemp, progState == PROG_ON);
}

void handleButtons(unsigned long currentMillis) {
  // Handle encoder button
  bool encoderButton = digitalRead(PIN_ENCODER_SW);
  
  if (encoderButton == LOW && lastEncoderButton == HIGH) {
    encoderButtonPressTime = currentMillis;
    longPressHandled = false;
  }
  
  if (encoderButton == HIGH && lastEncoderButton == LOW) {
    if (!longPressHandled && currentMillis - encoderButtonPressTime < 1000) {
      // Short press
      if (progState == PROG_ON) {
        // En mode PROG_ON, switcher entre l'écran normal et le graphe
        #ifdef ENABLE_GRAPH
        showGraph = !showGraph;
        #endif
      } else if (progState == PROG_OFF) {
        // En mode PROG_OFF, basculer le mode d'édition
        toggleEditMode();
      }
    }
  }
  
  lastEncoderButton = encoderButton;
  
  // Handle push button
  bool pushButton = digitalRead(PIN_PUSH_BUTTON);
  
  if (pushButton == LOW && lastPushButton == HIGH) {
    delay(50); // Debounce
    pushButton = digitalRead(PIN_PUSH_BUTTON);
    if (pushButton == LOW) {
      toggleProgState();
    }
  }
  
  lastPushButton = pushButton;
}

void handleEncoder() {
  long newPosition = encoder.read() / 4;
  
  if (newPosition != encoderPosition) {
    int delta = newPosition - encoderPosition;
    encoderPosition = newPosition;
    
    if (editMode == NAV_MODE) {
      // Navigate between parameters
      selectedParam += delta;
      if (selectedParam < 0) selectedParam = NUM_PARAMS - 1;
      if (selectedParam >= NUM_PARAMS) selectedParam = 0;
    } else {
      // Edit selected parameter
      editParameter(delta);
    }
  }
}

void toggleEditMode() {
  if (editMode == NAV_MODE) {
    editMode = EDIT_MODE;
  } else {
    editMode = NAV_MODE;
    saveToEEPROM();
  }
}

void toggleProgState() {
  if (progState == PROG_OFF) {
    progState = PROG_ON;
    currentPhase = PHASE_1;
    programStartTime = millis();
    phaseStartTime = millis();
    plateauReached = false;
    targetTemp = readTemperature();
    #ifdef ENABLE_GRAPH
    graphIndex = 0;
    lastGraphUpdate = millis();
    #endif
    resetPID();
  } else {
    progState = PROG_OFF;
    currentPhase = PHASE_0;
    setRelay(false);
    showGraph = false;
  }
}

void editParameter(int delta) {
  int* value;
  int minVal, maxVal, step;
  
  // Ordre de sélection: gauche à droite, haut en bas
  switch (selectedParam) {
    // Phase 1
    case 0: value = &params.step1Speed; minVal = 1; maxVal = 1000; step = 10; break;   // P1 gauche
    case 1: value = &params.step1Temp; minVal = 0; maxVal = 1500; step = 10; break;    // P1 centre
    case 2: value = &params.step1Wait; minVal = 0; maxVal = 999; step = 5; break;      // P1 droite
    // Phase 2
    case 3: value = &params.step2Speed; minVal = 1; maxVal = 1000; step = 10; break;   // P2 gauche
    case 4: value = &params.step2Temp; minVal = 0; maxVal = 1500; step = 10; break;    // P2 centre
    case 5: value = &params.step2Wait; minVal = 0; maxVal = 999; step = 5; break;      // P2 droite
    // Phase 3
    case 6: value = &params.step3Speed; minVal = 1; maxVal = 1000; step = 10; break;   // P3 gauche
    case 7: value = &params.step3Temp; minVal = 0; maxVal = 1500; step = 10; break;    // P3 centre
    case 8: value = &params.step3Wait; minVal = 0; maxVal = 999; step = 5; break;      // P3 droite
    // Cooldown
    case 9: value = &params.step4Speed; minVal = 1; maxVal = 1000; step = 10; break;   // Cool gauche
    case 10: value = &params.step4Target; minVal = 0; maxVal = 1000; step = 10; break; // Cool droite
    default: return;
  }
  
  *value += delta * step;
  if (*value < minVal) *value = minVal;
  if (*value > maxVal) *value = maxVal;
}

void updateProgram(unsigned long currentMillis, float currentTemp) {
  unsigned long phaseElapsed = currentMillis - phaseStartTime;
  
  switch (currentPhase) {
    case PHASE_1:
      targetTemp = calculateTargetTemp(currentTemp, params.step1Temp, params.step1Speed, phaseElapsed);
      if (checkPhaseComplete(currentTemp, params.step1Temp, plateauReached, plateauStartTime, params.step1Wait, currentMillis)) {
        currentPhase = PHASE_2;
        phaseStartTime = currentMillis;
        plateauReached = false;
      }
      break;
      
    case PHASE_2:
      targetTemp = calculateTargetTemp(currentTemp, params.step2Temp, params.step2Speed, phaseElapsed);
      if (checkPhaseComplete(currentTemp, params.step2Temp, plateauReached, plateauStartTime, params.step2Wait, currentMillis)) {
        currentPhase = PHASE_3;
        phaseStartTime = currentMillis;
        plateauReached = false;
      }
      break;
      
    case PHASE_3:
      targetTemp = calculateTargetTemp(currentTemp, params.step3Temp, params.step3Speed, phaseElapsed);
      if (checkPhaseComplete(currentTemp, params.step3Temp, plateauReached, plateauStartTime, params.step3Wait, currentMillis)) {
        currentPhase = PHASE_4_COOLDOWN;
        phaseStartTime = currentMillis;
        plateauReached = false;
      }
      break;
      
    case PHASE_4_COOLDOWN:
      targetTemp = calculateCoolingTarget(currentTemp, params.step4Target, params.step4Speed, phaseElapsed);
      if (currentTemp <= params.step4Target) {
        progState = PROG_OFF;
        currentPhase = PHASE_0;
        setRelay(false);
      }
      break;
  }
}

float calculateTargetTemp(float startTemp, int targetTemp, int speed, unsigned long elapsed) {
  float calculatedTarget = startTemp + (speed * (elapsed / 3600000.0));
  return (calculatedTarget > targetTemp) ? targetTemp : calculatedTarget;
}

float calculateCoolingTarget(float startTemp, int targetTemp, int speed, unsigned long elapsed) {
  float calculatedTarget = startTemp - (speed * (elapsed / 3600000.0));
  return (calculatedTarget < targetTemp) ? targetTemp : calculatedTarget;
}

bool checkPhaseComplete(float currentTemp, int phaseTemp, bool &reached, unsigned long &plateauStart, int waitMinutes, unsigned long currentMillis) {
  if (!reached && currentTemp >= phaseTemp - 5 && currentTemp >= phaseTemp) {
    reached = true;
    plateauStart = currentMillis;
  }
  
  if (reached) {
    unsigned long plateauElapsed = currentMillis - plateauStart;
    if (plateauElapsed >= waitMinutes * 60000UL) {
      return true;
    }
  }
  
  return false;
}

#ifdef ENABLE_GRAPH
void updateGraphData(unsigned long currentMillis, float temp) {
  // Enregistrer un point toutes les 30 secondes pour couvrir 32 minutes avec 64 points
  if (currentMillis - lastGraphUpdate >= GRAPH_UPDATE_INTERVAL) {
    lastGraphUpdate = currentMillis;
    
    if (graphIndex < GRAPH_SIZE) {
      graphActual[graphIndex] = temp;
      graphTarget[graphIndex] = targetTemp;
      graphIndex++;
    } else {
      // Décaler les données vers la gauche pour garder les plus récentes
      for (int i = 0; i < GRAPH_SIZE - 1; i++) {
        graphActual[i] = graphActual[i + 1];
        graphTarget[i] = graphTarget[i + 1];
      }
      graphActual[GRAPH_SIZE - 1] = temp;
      graphTarget[GRAPH_SIZE - 1] = targetTemp;
    }
  }
}
#endif

void updateDisplay() {
  u8g2.firstPage();
  do {
    #ifdef ENABLE_GRAPH
    if (showGraph) {
      drawGraph();
    } else 
    #endif
    if (tempFailActive && (millis() - tempFailStartTime > 120000)) {
      // Error is displayed in main loop
    } else {
      if (progState == PROG_OFF) {
        drawProgOffScreen();
      } else {
        drawProgOnScreen();
      }
    }
  } while (u8g2.nextPage());
}

void displayCriticalError() {
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(0, 10, "ERROR!");
    u8g2.drawStr(0, 25, "Temp fail 2min");
    u8g2.drawStr(0, 40, "Heat stopped");
    u8g2.drawStr(0, 55, "Press button");
  } while (u8g2.nextPage());
}

void waitForButtonPress() {
  while (digitalRead(PIN_PUSH_BUTTON) == HIGH) {
    delay(10);
  }
  delay(50); // Debounce
  while (digitalRead(PIN_PUSH_BUTTON) == LOW) {
    delay(10);
  }
}

void saveToEEPROM() {
  EEPROM.put(EEPROM_ADDR_MAGIC, EEPROM_MAGIC);
  EEPROM.put(EEPROM_ADDR_PARAMS, params);
}

void loadFromEEPROM() {
  uint16_t magic;
  EEPROM.get(EEPROM_ADDR_MAGIC, magic);
  
  if (magic == EEPROM_MAGIC) {
    EEPROM.get(EEPROM_ADDR_PARAMS, params);
  } else {
    // First use - save default values
    saveToEEPROM();
  }
}

