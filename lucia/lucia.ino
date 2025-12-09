/*
 * LUCIA - Contrôleur de four céramique
 * Système de thermostat intelligent pour four électrique
 */

#include <Wire.h>
#include <SPI.h>
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
FiringParams paramsBackup; // Sauvegarde pour détecter les changements

// ===== SETTINGS PARAMETERS =====
SettingsParams settings = {1000, 2.0, 0.5, 0.0}; // pcycle (ms), kp, ki, kd
SettingsParams settingsBackup; // Sauvegarde pour détecter les changements
int selectedSetting = 0;
const int NUM_SETTINGS = 5; // Heat Cycle, Kp, Ki, Kd, Exit

// ===== UI PARAMETERS =====
EditMode editMode = NAV_MODE;
int selectedParam = 0;
const int NUM_PARAMS = 12; // 11 paramètres + 1 pour l'icône settings (index 11)
long encoderPosition = 0;

// ===== TIMING VARIABLES =====
unsigned long lastDisplayUpdate = 0;
unsigned long programStartTime = 0;
unsigned long phaseStartTime = 0;
unsigned long plateauStartTime = 0;
unsigned long tempFailStartTime = 0;
bool tempFailActive = false;
unsigned long lastTempRead = 0;
float cachedTemperature = NAN;
#define TEMP_READ_INTERVAL 500  // Lire la température toutes les 500ms (2 fois par seconde)

// ===== EEPROM PROTECTION =====
unsigned long lastEEPROMWrite = 0;
#define EEPROM_WRITE_MIN_INTERVAL 5000  // Minimum 5 secondes entre deux écritures EEPROM
bool eepromWriteAllowed = true;  // Permettre la première écriture au démarrage
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
  
  // Initialize SPI for MAX31856 (CRITICAL: must be before max31856.begin())
  pinMode(PIN_MAX_CS, OUTPUT);
  digitalWrite(PIN_MAX_CS, HIGH); // CS doit être HIGH quand non utilisé
  SPI.begin();
  delay(100); // Petit délai pour stabilisation
  
  // Initialize MAX31856
  if (!max31856.begin()) {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(0, 20, "MAX31856 Error!");
    u8g2.drawStr(0, 35, "Check wiring");
    u8g2.sendBuffer();
    while (1) delay(1000);
  }
  
  max31856.setThermocoupleType(MAX31856_TCTYPE_S);
  max31856.setConversionMode(MAX31856_CONTINUOUS);
  
  // Test de lecture immédiate et initialisation du cache
  delay(500); // Attendre la première conversion
  float testTemp = readTemperature();
  cachedTemperature = testTemp; // Initialiser le cache
  lastTempRead = millis();
  
  // Load parameters from EEPROM (initialise aussi les backups)
  loadFromEEPROM();
  
  // S'assurer que settings est synchronisé avec les valeurs PID réelles
  settings.kp = KP;
  settings.ki = KI;
  settings.kd = KD;
  // Mettre à jour le backup après synchronisation
  settingsBackup = settings;
  
  // Initialize temperature control
  initTemperatureControl();
  
  // Initial display
  updateDisplay(0);
}

void loop() {
  unsigned long currentMillis = millis();
  
  // Read temperature (limité à 2 fois par seconde pour éviter de surcharger le MAX31856)
  float temp;
  if (currentMillis - lastTempRead >= TEMP_READ_INTERVAL) {
    temp = readTemperature();
    cachedTemperature = temp;
    lastTempRead = currentMillis;
  } else {
    temp = cachedTemperature; // Utiliser la dernière valeur lue
  }
  
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
  
  // Handle encoder (limité à 50Hz pour éviter de surcharger avec encoder.read())
  static unsigned long lastEncoderCheck = 0;
  if ((progState == PROG_OFF || progState == SETTINGS) && !showGraph && (currentMillis - lastEncoderCheck >= 20)) {
    handleEncoder();
    lastEncoderCheck = currentMillis;
  }
  
  // Update program state
  if (progState == PROG_ON) {
    updateProgram(currentMillis, temp);
    #ifdef ENABLE_GRAPH
    updateGraphData(currentMillis, temp);
    #endif
  }
  
  // Update display (passer currentMillis pour éviter les appels millis() supplémentaires)
  if (currentMillis - lastDisplayUpdate >= 100) {
    lastDisplayUpdate = currentMillis;
    updateDisplay(currentMillis);
  }
  
  // Update temperature control (passer currentMillis au lieu de le rappeler)
  updateTemperatureControl(temp, targetTemp, progState == PROG_ON, currentMillis);
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
        // Vérifier si on est sur l'icône settings (selectedParam == -1 ou position spéciale)
        // On utilise selectedParam == 11 comme indicateur pour l'icône settings
        if (selectedParam == 11 && editMode == NAV_MODE) {
          // Entrer dans les settings
          progState = SETTINGS;
          selectedSetting = 0;
          editMode = NAV_MODE;
        } else {
          // Basculer le mode d'édition normal
          toggleEditMode();
        }
      } else if (progState == SETTINGS) {
        // En mode SETTINGS, gérer la navigation/édition
        if (selectedSetting == 4) {
          // Bouton Exit : retourner à PROG_OFF
          progState = PROG_OFF;
          selectedParam = 0;
          editMode = NAV_MODE;
        } else {
          // Toggle edit mode pour les autres paramètres
          toggleSettingsEditMode();
        }
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
    // Inverser le sens du delta pour inverser la direction de défilement
    int delta = encoderPosition - newPosition;
    encoderPosition = newPosition;
    
    if (progState == SETTINGS) {
      // Gérer l'encodeur dans l'écran settings
      if (editMode == NAV_MODE) {
        // Navigate between settings
        selectedSetting += delta;
        if (selectedSetting < 0) selectedSetting = NUM_SETTINGS - 1;
        if (selectedSetting >= NUM_SETTINGS) selectedSetting = 0;
      } else {
        // Edit selected setting
        editSetting(delta);
      }
    } else if (progState == PROG_OFF) {
      // Gérer l'encodeur dans l'écran phase 0
      if (editMode == NAV_MODE) {
        // Navigate between parameters (0-10 pour les paramètres, 11 pour l'icône settings)
        selectedParam += delta;
        if (selectedParam < 0) selectedParam = NUM_PARAMS - 1;
        if (selectedParam >= NUM_PARAMS) selectedParam = 0;
        // selectedParam == 11 correspond à l'icône settings
      } else {
        // Edit selected parameter (mais pas l'icône settings)
        if (selectedParam < NUM_PARAMS - 1) {
          editParameter(delta);
        }
      }
    }
  }
}

void toggleEditMode() {
  if (editMode == NAV_MODE) {
    editMode = EDIT_MODE;
    // Sauvegarder les valeurs actuelles pour détecter les changements
    paramsBackup = params;
  } else {
    editMode = NAV_MODE;
    // Sauvegarder seulement si les paramètres ont changé
    saveToEEPROMIfChanged();
  }
}

void toggleSettingsEditMode() {
  if (editMode == NAV_MODE) {
    editMode = EDIT_MODE;
    // Sauvegarder les valeurs actuelles pour détecter les changements
    settingsBackup = settings;
  } else {
    editMode = NAV_MODE;
    // Sauvegarder seulement si les settings ont changé
    saveSettingsToEEPROMIfChanged();
  }
}

void editSetting(int delta) {
  switch (selectedSetting) {
    case 0: // Heat Cycle (Pcycle)
      settings.pcycle += delta * 100; // Incrément de 100ms
      if (settings.pcycle < 100) settings.pcycle = 100;
      if (settings.pcycle > 10000) settings.pcycle = 10000;
      CYCLE_LENGTH = settings.pcycle; // Mettre à jour immédiatement
      break;
    case 1: // Kp - modifier directement la valeur réelle du PID
      KP += delta * 0.1; // Incrément de 0.1
      if (KP < 0.0) KP = 0.0;
      if (KP > 10.0) KP = 10.0;
      settings.kp = KP; // Synchroniser settings avec la valeur réelle
      break;
    case 2: // Ki - modifier directement la valeur réelle du PID
      KI += delta * 0.1; // Incrément de 0.1
      if (KI < 0.0) KI = 0.0;
      if (KI > 10.0) KI = 10.0;
      settings.ki = KI; // Synchroniser settings avec la valeur réelle
      break;
    case 3: // Kd - modifier directement la valeur réelle du PID
      KD += delta * 0.1; // Incrément de 0.1
      if (KD < 0.0) KD = 0.0;
      if (KD > 10.0) KD = 10.0;
      settings.kd = KD; // Synchroniser settings avec la valeur réelle
      break;
    case 4: // Exit - ne rien faire, géré par le bouton
      break;
  }
}

void saveSettingsToEEPROM() {
  // Vérifier la protection temporelle (minimum 5 secondes entre écritures)
  unsigned long currentMillis = millis();
  if (!eepromWriteAllowed) {
    // Vérifier si suffisamment de temps s'est écoulé depuis la dernière écriture
    // La soustraction gère automatiquement le rollover de millis()
    if (currentMillis - lastEEPROMWrite < EEPROM_WRITE_MIN_INTERVAL) {
      return; // Trop tôt, ignorer l'écriture pour protéger l'EEPROM
    }
  }
  
  // Sauvegarder les settings dans l'EEPROM après les params (toujours)
  EEPROM.put(EEPROM_ADDR_PARAMS + sizeof(FiringParams), settings);
  lastEEPROMWrite = currentMillis; // Mettre à jour le timestamp
  eepromWriteAllowed = false; // Activer la protection après la première écriture
}

void saveSettingsToEEPROMIfChanged() {
  // Sauvegarder seulement si les settings ont réellement changé
  if (settings.pcycle != settingsBackup.pcycle ||
      settings.kp != settingsBackup.kp ||
      settings.ki != settingsBackup.ki ||
      settings.kd != settingsBackup.kd) {
    saveSettingsToEEPROM();
    settingsBackup = settings; // Mettre à jour le backup
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
  // Utiliser des calculs entiers pour économiser le code
  unsigned long tempIncrease = (unsigned long)speed * elapsed / 3600000UL;
  float calculatedTarget = startTemp + (float)tempIncrease;
  return (calculatedTarget > targetTemp) ? (float)targetTemp : calculatedTarget;
}

float calculateCoolingTarget(float startTemp, int targetTemp, int speed, unsigned long elapsed) {
  // Utiliser des calculs entiers pour économiser le code
  unsigned long tempDecrease = (unsigned long)speed * elapsed / 3600000UL;
  float calculatedTarget = startTemp - (float)tempDecrease;
  return (calculatedTarget < targetTemp) ? (float)targetTemp : calculatedTarget;
}

bool checkPhaseComplete(float currentTemp, int phaseTemp, bool &reached, unsigned long &plateauStart, int waitMinutes, unsigned long currentMillis) {
  if (!reached && currentTemp >= (phaseTemp - 5) && currentTemp >= phaseTemp) {
    reached = true;
    plateauStart = currentMillis;
  }
  if (reached && (currentMillis - plateauStart) >= (unsigned long)waitMinutes * 60000UL) {
    return true;
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

void updateDisplay(unsigned long currentMillis) {
  u8g2.firstPage();
  do {
    #ifdef ENABLE_GRAPH
    if (showGraph) {
      drawGraph();
    } else 
    #endif
    if (tempFailActive && (currentMillis - tempFailStartTime > 120000)) {
      // Error is displayed in main loop
    } else {
      if (progState == SETTINGS) {
        drawSettingsScreen();
      } else if (progState == PROG_OFF) {
        drawProgOffScreen();
      } else {
        drawProgOnScreen(currentMillis); // Passer currentMillis pour éviter millis() dans la fonction
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
  // Vérifier la protection temporelle (minimum 5 secondes entre écritures)
  unsigned long currentMillis = millis();
  if (!eepromWriteAllowed) {
    // Vérifier si suffisamment de temps s'est écoulé depuis la dernière écriture
    // La soustraction gère automatiquement le rollover de millis()
    if (currentMillis - lastEEPROMWrite < EEPROM_WRITE_MIN_INTERVAL) {
      return; // Trop tôt, ignorer l'écriture pour protéger l'EEPROM
    }
  }
  
  // Sauvegarder toujours (pour première utilisation ou changement)
  EEPROM.put(EEPROM_ADDR_MAGIC, EEPROM_MAGIC);
  EEPROM.put(EEPROM_ADDR_PARAMS, params);
  lastEEPROMWrite = currentMillis; // Mettre à jour le timestamp
  eepromWriteAllowed = false; // Activer la protection après la première écriture
}

void saveToEEPROMIfChanged() {
  // Sauvegarder seulement si les paramètres ont réellement changé
  bool hasChanged = false;
  // Comparer tous les champs de params
  if (params.step1Speed != paramsBackup.step1Speed ||
      params.step1Temp != paramsBackup.step1Temp ||
      params.step1Wait != paramsBackup.step1Wait ||
      params.step2Speed != paramsBackup.step2Speed ||
      params.step2Temp != paramsBackup.step2Temp ||
      params.step2Wait != paramsBackup.step2Wait ||
      params.step3Speed != paramsBackup.step3Speed ||
      params.step3Temp != paramsBackup.step3Temp ||
      params.step3Wait != paramsBackup.step3Wait ||
      params.step4Speed != paramsBackup.step4Speed ||
      params.step4Target != paramsBackup.step4Target) {
    hasChanged = true;
  }
  
  if (hasChanged) {
    saveToEEPROM();
    paramsBackup = params; // Mettre à jour le backup
  }
}

void loadFromEEPROM() {
  uint16_t magic;
  EEPROM.get(EEPROM_ADDR_MAGIC, magic);
  
  if (magic == EEPROM_MAGIC) {
    EEPROM.get(EEPROM_ADDR_PARAMS, params);
    // Charger aussi les settings
    EEPROM.get(EEPROM_ADDR_PARAMS + sizeof(FiringParams), settings);
    // Vérifier et corriger les valeurs settings si invalides
    if (isnan(settings.kp) || settings.kp < 0.0 || settings.kp > 10.0) settings.kp = 2.0;
    if (isnan(settings.ki) || settings.ki < 0.0 || settings.ki > 10.0) settings.ki = 0.5;
    if (isnan(settings.kd) || settings.kd < 0.0 || settings.kd > 10.0) settings.kd = 0.0;
    if (settings.pcycle < 100 || settings.pcycle > 10000) settings.pcycle = 1000;
    // Appliquer les settings chargés (et validés)
    CYCLE_LENGTH = settings.pcycle;
    KP = settings.kp;
    KI = settings.ki;
    KD = settings.kd;
    // Initialiser les backups avec les valeurs chargées
    paramsBackup = params;
    settingsBackup = settings;
  } else {
    // First use - save default values
    // La protection est activée automatiquement après la première écriture
    saveToEEPROM();
    saveSettingsToEEPROM();
    // Initialiser les backups avec les valeurs par défaut
    paramsBackup = params;
    settingsBackup = settings;
  }
}

// Fonction pour obtenir la température mise en cache (lue toutes les 500ms dans loop())
float getCurrentTemperature() {
  return cachedTemperature;
}

