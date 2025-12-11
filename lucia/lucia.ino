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
SettingsParams settings = {1000, 2.0, 0.5, 0.0, 10}; // pcycle (ms), kp, ki, kd, maxDelta (°C)
SettingsParams settingsBackup; // Sauvegarde pour détecter les changements
int selectedSetting = 0;
const int NUM_SETTINGS = 6; // Heat Cycle, Kp, Ki, Kd, Max delta, Exit
int settingsScrollOffset = 0; // Scroll pour l'écran settings

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
unsigned long lastGraphUpdate = 0;

// ===== EEPROM PROTECTION =====
unsigned long lastEEPROMWrite = 0;
bool eepromWriteAllowed = true;  // Permettre la première écriture au démarrage

// ===== BUTTON STATES =====
bool lastEncoderButton = HIGH;
bool lastPushButton = HIGH;
bool showGraph = false;
bool criticalErrorActive = false;

// ===== TEMPERATURE CONTROL =====
float targetTemp = 0;
bool plateauReached = false;

// ===== EEPROM ADDRESSES =====
#define EEPROM_MAGIC 0x4C55  // "LU" magic number to detect first use
#define EEPROM_ADDR_MAGIC 0
#define EEPROM_ADDR_PARAMS 2

// ===== GRAPH DATA =====
// Nouvelle implémentation optimisée avec décimation adaptative (384 octets)
// Compression : uint8_t pour températures (0-255 = 0-1280°C, résolution 5°C)
uint8_t graphTempRead[GRAPH_SIZE];    // Température mesurée (96 octets)
uint8_t graphTempTarget[GRAPH_SIZE];  // Température consigne (96 octets)
uint16_t graphTimeStamps[GRAPH_SIZE]; // Temps en secondes depuis début programme (192 octets)
uint8_t graphIndex = 0;               // Index du prochain point à écrire
uint8_t graphCount = 0;               // Nombre de points actuellement stockés
uint16_t samplingInterval = 2;        // Intervalle d'échantillonnage en secondes (commence à 2s)
uint16_t nextSamplingTime = 2;        // Prochain temps d'échantillonnage (en secondes)

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
  
  // Attente non-bloquante pour stabilisation SPI (100ms)
  unsigned long spiInitTime = millis();
  while (millis() - spiInitTime < 100) {
    // Attente active pour stabilisation
  }
  
  // Initialize MAX31856
  if (!max31856.begin()) {
    criticalErrorActive = true;
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(0, 20, "MAX31856 Error!");
    u8g2.drawStr(0, 35, "Check wiring");
    u8g2.drawStr(0, 50, "Press to retry");
    u8g2.sendBuffer();
    // Ne pas bloquer, gérer dans loop()
  } else {
    max31856.setThermocoupleType(MAX31856_TCTYPE_S);
    max31856.setConversionMode(MAX31856_CONTINUOUS);
    
    // Attente non-bloquante pour la première conversion (500ms)
    unsigned long conversionStartTime = millis();
    while (millis() - conversionStartTime < 500) {
      // Attente active pour première conversion
    }
    
    // Test de lecture et initialisation du cache
    float testTemp = readTemperature();
    cachedTemperature = testTemp;
    lastTempRead = millis();
  }
  
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
  
  // Gestion de l'erreur critique MAX31856 (si détectée au setup)
  if (criticalErrorActive) {
    // Attendre un appui sur le bouton pour retry
    bool pushButton = digitalRead(PIN_PUSH_BUTTON);
    if (pushButton == LOW && lastPushButton == HIGH) {
      // Tentative de réinitialisation du MAX31856
      if (max31856.begin()) {
        max31856.setThermocoupleType(MAX31856_TCTYPE_S);
        max31856.setConversionMode(MAX31856_CONTINUOUS);
        criticalErrorActive = false;
        cachedTemperature = readTemperature();
        lastTempRead = currentMillis;
      }
    }
    lastPushButton = pushButton;
    return; // Ne rien faire d'autre tant que l'erreur n'est pas résolue
  }
  
  // Lecture température (intervalle défini pour optimiser les performances)
  float temp;
  if (currentMillis - lastTempRead >= TEMP_READ_INTERVAL) {
    temp = readTemperature();
    cachedTemperature = temp;
    lastTempRead = currentMillis;
  } else {
    temp = cachedTemperature; // Utiliser la dernière valeur lue
  }
  
  // Vérification des erreurs de lecture température
  if (isnan(temp) || temp < -100 || temp > 1500) {
    if (!tempFailActive) {
      tempFailActive = true;
      tempFailStartTime = currentMillis;
    } else if (currentMillis - tempFailStartTime > TEMP_FAIL_TIMEOUT) {
      // Erreur critique - arrêt du chauffage pour sécurité
      if (progState == PROG_ON) {
        progState = PROG_OFF;
        currentPhase = PHASE_0;
        setRelay(false);
      }
      tempFailActive = false; // Réinitialiser pour permettre une nouvelle tentative
    }
  } else {
    tempFailActive = false;
  }
  
  // Gestion des boutons
  handleButtons(currentMillis);
  
  // Gestion de l'encodeur (intervalle optimisé pour réduire la charge CPU)
  static unsigned long lastEncoderCheck = 0;
  if ((progState == PROG_OFF || progState == SETTINGS) && !showGraph && 
      (currentMillis - lastEncoderCheck >= ENCODER_CHECK_INTERVAL)) {
    handleEncoder();
    lastEncoderCheck = currentMillis;
  }
  
  // Mise à jour de l'état du programme
  if (progState == PROG_ON) {
    updateProgram(currentMillis, temp);
    updateGraphData(currentMillis, temp);
  }
  
  // Mise à jour de l'affichage
  if (currentMillis - lastDisplayUpdate >= DISPLAY_UPDATE_INTERVAL) {
    lastDisplayUpdate = currentMillis;
    updateDisplay(currentMillis);
  }
  
  // Mise à jour du contrôle de température
  updateTemperatureControl(temp, targetTemp, progState == PROG_ON, currentMillis);
}

void handleButtons(unsigned long currentMillis) {
  // Gestion du bouton encodeur
  bool encoderButton = digitalRead(PIN_ENCODER_SW);
  
  if (encoderButton == LOW && lastEncoderButton == HIGH) {
    // Appui détecté - exécuter l'action immédiatement
    if (progState == PROG_ON) {
      showGraph = !showGraph;
    } else if (progState == PROG_OFF) {
      if (selectedParam == 11 && editMode == NAV_MODE) {
        progState = SETTINGS;
        selectedSetting = 0;
        editMode = NAV_MODE;
      } else {
        toggleEditMode();
      }
    } else if (progState == SETTINGS) {
      if (selectedSetting == 5) {
        progState = PROG_OFF;
        selectedParam = 0;
        editMode = NAV_MODE;
      } else {
        toggleSettingsEditMode();
      }
    }
  }
  
  lastEncoderButton = encoderButton;
  
  // Gestion du bouton push
  bool pushButton = digitalRead(PIN_PUSH_BUTTON);
  
  if (pushButton == LOW && lastPushButton == HIGH) {
    // Appui détecté - exécuter l'action immédiatement
    toggleProgState();
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
        // Navigate between settings (sans boucler)
        selectedSetting += delta;
        // Limiter aux bornes sans boucler
        if (selectedSetting < 0) selectedSetting = 0;
        if (selectedSetting >= NUM_SETTINGS) selectedSetting = NUM_SETTINGS - 1;
      } else {
        // Edit selected setting
        editSetting(delta);
      }
    } else if (progState == PROG_OFF) {
      // Gérer l'encodeur dans l'écran phase 0
      if (editMode == NAV_MODE) {
        // Navigate between parameters (sans boucler)
        selectedParam += delta;
        // Limiter aux bornes sans boucler
        if (selectedParam < 0) selectedParam = 0;
        if (selectedParam >= NUM_PARAMS) selectedParam = NUM_PARAMS - 1;
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
    case 4: // Max delta
      settings.maxDelta += delta * 1; // Incrément de 1°C
      if (settings.maxDelta < 1) settings.maxDelta = 1;
      if (settings.maxDelta > 50) settings.maxDelta = 50;
      break;
    case 5: // Exit - ne rien faire, géré par le bouton
      break;
  }
}

void saveSettingsToEEPROM() {
  // Protection temporelle : minimum 5 secondes entre écritures pour préserver l'EEPROM
  unsigned long currentMillis = millis();
  if (!eepromWriteAllowed) {
    // Vérifier si l'intervalle minimum est respecté
    // La soustraction gère automatiquement le rollover de millis()
    if (currentMillis - lastEEPROMWrite < EEPROM_WRITE_MIN_INTERVAL) {
      return; // Intervalle insuffisant, écriture ignorée pour protéger l'EEPROM
    }
  }
  
  // Écrire les settings dans l'EEPROM (après la structure FiringParams)
  EEPROM.put(EEPROM_ADDR_PARAMS + sizeof(FiringParams), settings);
  lastEEPROMWrite = currentMillis;
  eepromWriteAllowed = false; // Activer la protection après la première écriture
}

void saveSettingsToEEPROMIfChanged() {
  // Sauvegarder seulement si les settings ont réellement changé
  if (settings.pcycle != settingsBackup.pcycle ||
      settings.kp != settingsBackup.kp ||
      settings.ki != settingsBackup.ki ||
      settings.kd != settingsBackup.kd ||
      settings.maxDelta != settingsBackup.maxDelta) {
    saveSettingsToEEPROM();
    settingsBackup = settings; // Mettre à jour le backup
  }
}

void toggleProgState() {
  if (progState == PROG_OFF) {
    progState = PROG_ON;
    programStartTime = millis();
    phaseStartTime = millis();
    plateauReached = false;
    
    // Lire la température actuelle pour démarrage intelligent
    float currentTemp = readTemperature();
    targetTemp = currentTemp;
    
    // Détection automatique de la phase de départ basée sur la température actuelle
    if (!isnan(currentTemp) && currentTemp > 0) {
      if (currentTemp < params.step1Temp) {
        // Four froid ou en dessous de la cible Phase 1 : démarrer en Phase 1
        currentPhase = PHASE_1;
      } else if (currentTemp < params.step2Temp) {
        // Four déjà chaud (entre step1 et step2) : démarrer en Phase 2
        currentPhase = PHASE_2;
        // Ajuster targetTemp pour continuer la montée depuis la température actuelle
        targetTemp = currentTemp;
      } else if (currentTemp < params.step3Temp) {
        // Four très chaud (entre step2 et step3) : démarrer en Phase 3
        currentPhase = PHASE_3;
        targetTemp = currentTemp;
      } else {
        // Four à température finale ou plus : démarrer en cooldown
        currentPhase = PHASE_4_COOLDOWN;
        targetTemp = currentTemp;
      }
    } else {
      // Erreur de lecture ou température invalide : démarrer en Phase 1 par sécurité
      currentPhase = PHASE_1;
      targetTemp = 20.0; // Température par défaut
    }
    
    // Réinitialiser les données du graphe
    graphIndex = 0;
    graphCount = 0;
    samplingInterval = 5;
    nextSamplingTime = 5;
    lastGraphUpdate = millis();
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
  // Calcul de la température cible basé sur la vitesse de montée (°C/h)
  // Conversion : elapsed (ms) -> heures -> température
  unsigned long tempIncrease = (unsigned long)speed * elapsed / 3600000UL;
  float calculatedTarget = startTemp + (float)tempIncrease;
  return (calculatedTarget > targetTemp) ? (float)targetTemp : calculatedTarget;
}

float calculateCoolingTarget(float startTemp, int targetTemp, int speed, unsigned long elapsed) {
  // Calcul de la température cible basé sur la vitesse de descente (°C/h)
  // Conversion : elapsed (ms) -> heures -> température
  unsigned long tempDecrease = (unsigned long)speed * elapsed / 3600000UL;
  float calculatedTarget = startTemp - (float)tempDecrease;
  return (calculatedTarget < targetTemp) ? (float)targetTemp : calculatedTarget;
}

bool checkPhaseComplete(float currentTemp, int phaseTemp, bool &reached, unsigned long &plateauStart, int waitMinutes, unsigned long currentMillis) {
  // Vérifier si la température est dans la tolérance (maxDelta en dessous) ET au-dessus de la cible
  if (!reached && currentTemp >= (phaseTemp - settings.maxDelta) && currentTemp >= phaseTemp) {
    reached = true;
    plateauStart = currentMillis;
  }
  if (reached && (currentMillis - plateauStart) >= (unsigned long)waitMinutes * 60000UL) {
    return true;
  }
  return false;
}

// Convertit une température float en uint8_t (0-255 = 0-1280°C, résolution ~5°C)
uint8_t tempToUint8(float temp) {
  if (temp < 0) return 0;
  if (temp > 1280) return 255;
  return (uint8_t)(temp * 255.0 / 1280.0);
}

// Convertit un uint8_t en température float
float uint8ToTemp(uint8_t value) {
  return (float)value * 1280.0 / 255.0;
}

void updateGraphData(unsigned long currentMillis, float temp) {
  // Calculer le temps écoulé depuis le début du programme (en secondes)
  uint16_t elapsedSeconds = (currentMillis - programStartTime) / 1000;
  
  // Vérifier si il est temps d'enregistrer un nouveau point
  if (elapsedSeconds < nextSamplingTime) {
    return; // Pas encore temps d'enregistrer
  }
  
  // Enregistrer le point
  graphTempRead[graphIndex] = tempToUint8(temp);
  graphTempTarget[graphIndex] = tempToUint8(targetTemp);
  graphTimeStamps[graphIndex] = elapsedSeconds;
  
  // Avancer l'index (buffer circulaire)
  graphIndex++;
  if (graphIndex >= GRAPH_SIZE) {
    graphIndex = 0; // Revenir au début (écrasera les anciennes données)
  }
  
  // Incrémenter le compteur (max GRAPH_SIZE)
  if (graphCount < GRAPH_SIZE) {
    graphCount++;
  }
  
  // Calculer le prochain temps d'échantillonnage
  nextSamplingTime = elapsedSeconds + samplingInterval;
  
  // Décimation adaptative : augmenter l'intervalle d'échantillonnage progressivement
  // Stratégie : 5s → 10s → 15s → ... → 60s max (augmente tous les 20 points)
  static uint8_t pointsSinceLastIncrease = 0;
  pointsSinceLastIncrease++;
  
  if (pointsSinceLastIncrease >= 20 && samplingInterval < 60) {
    // Augmenter l'intervalle de 5 secondes (max 60 secondes)
    samplingInterval += 5;
    if (samplingInterval > 60) samplingInterval = 60;
    pointsSinceLastIncrease = 0;
  }
  
  lastGraphUpdate = currentMillis;
}

void updateDisplay(unsigned long currentMillis) {
  u8g2.firstPage();
  do {
    if (showGraph && progState == PROG_ON) {
      drawGraph();
    } else if (tempFailActive && (currentMillis - tempFailStartTime > TEMP_FAIL_TIMEOUT)) {
      // Afficher l'erreur critique de température
      u8g2.setFont(u8g2_font_6x10_tf);
      u8g2.drawStr(0, 10, "ERROR!");
      u8g2.drawStr(0, 25, "Temp fail 2min");
      u8g2.drawStr(0, 40, "Heat stopped");
      u8g2.drawStr(0, 55, "Check sensor");
    } else {
      if (progState == SETTINGS) {
        drawSettingsScreen();
      } else if (progState == PROG_OFF) {
        drawProgOffScreen();
      } else {
        drawProgOnScreen(currentMillis);
      }
    }
  } while (u8g2.nextPage());
}

void saveToEEPROM() {
  // Protection temporelle : minimum 5 secondes entre écritures pour préserver l'EEPROM
  unsigned long currentMillis = millis();
  if (!eepromWriteAllowed) {
    // Vérifier si l'intervalle minimum est respecté
    // La soustraction gère automatiquement le rollover de millis()
    if (currentMillis - lastEEPROMWrite < EEPROM_WRITE_MIN_INTERVAL) {
      return; // Intervalle insuffisant, écriture ignorée pour protéger l'EEPROM
    }
  }
  
  // Écrire le magic number et les paramètres de cuisson
  EEPROM.put(EEPROM_ADDR_MAGIC, EEPROM_MAGIC);
  EEPROM.put(EEPROM_ADDR_PARAMS, params);
  lastEEPROMWrite = currentMillis;
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
    if (settings.maxDelta < 1 || settings.maxDelta > 50) settings.maxDelta = 10;
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

