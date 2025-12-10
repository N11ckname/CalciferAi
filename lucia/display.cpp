/*
 * display.cpp - Gestion de l'affichage OLED
 */

#include "definitions.h"
#include "display.h"
#include "temperature.h"

// Buffer partagé pour économiser la RAM (utilisé par toutes les fonctions d'affichage)
static char sharedBuffer[20];

// Fonction helper : affiche un paramètre avec effet de sélection/édition
// - NAV_MODE : cadre autour du paramètre sélectionné
// - EDIT_MODE : inversion vidéo (négatif) du paramètre en édition
void drawParam(int x, int y, const char* label, int value, const char* unit, int paramIndex, bool drawBox) {
  snprintf(sharedBuffer, 20, "%s%d%s", label, value, unit);
  
  // Note: la fonte doit être définie par la fonction appelante
  
  if (selectedParam == paramIndex && editMode == NAV_MODE) {
    // Mode navigation : afficher un cadre autour du paramètre
    int width = strlen(sharedBuffer) * 6;
    u8g2.drawStr(x, y, sharedBuffer);
    u8g2.drawFrame(x - 1, y - 9, width + 2, 11);
  } else if (selectedParam == paramIndex && editMode == EDIT_MODE) {
    // Mode édition : inversion vidéo pour mettre en évidence
    int width = strlen(sharedBuffer) * 6;
    u8g2.setDrawColor(1);
    u8g2.drawBox(x - 1, y - 9, width + 2, 11);
    u8g2.setDrawColor(0);
    u8g2.drawStr(x, y, sharedBuffer);
    u8g2.setDrawColor(1);
  } else {
    // Paramètre normal non sélectionné
    u8g2.drawStr(x, y, sharedBuffer);
  }
}

void drawProgOffScreen() {
  // Écran principal : configuration des phases de cuisson
  // Utilise la température mise en cache (actualisée toutes les 500ms)
  float currentTemp = getCurrentTemperature();
  
  // Affichage de la température actuelle en gros en haut
  u8g2.setFont(u8g2_font_9x15_tf);
  if (isnan(currentTemp)) {
    u8g2.drawStr(0, 12, "?C");
  } else {
    // Arrondi à l'entier le plus proche pour économiser l'espace
    int tempInt = (int)(currentTemp + 0.5);
    snprintf(sharedBuffer, 20, "%dC", tempInt);
    u8g2.drawStr(0, 12, sharedBuffer);
  }
  
  // Configuration Phase 1 : Vitesse (°C/h), Température cible (°C), Temps de palier (min)
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.drawStr(0, 24, "P1:");
  drawParam(18, 24, "", params.step1Speed, "C/h", 0, false);
  drawParam(60, 24, ">", params.step1Temp, "C", 1, false);
  drawParam(105, 24, "", params.step1Wait, "m", 2, false);
  
  // Configuration Phase 2
  u8g2.drawStr(0, 36, "P2:");
  drawParam(18, 36, "", params.step2Speed, "C/h", 3, false);
  drawParam(60, 36, ">", params.step2Temp, "C", 4, false);
  drawParam(105, 36, "", params.step2Wait, "m", 5, false);
  
  // Configuration Phase 3
  u8g2.drawStr(0, 48, "P3:");
  drawParam(18, 48, "", params.step3Speed, "C/h", 6, false);
  drawParam(60, 48, ">", params.step3Temp, "C", 7, false);
  drawParam(105, 48, "", params.step3Wait, "m", 8, false);
  
  // Configuration Cooldown : Vitesse de refroidissement, Température cible finale
  u8g2.drawStr(0, 60, "Cool:");
  drawParam(30, 60, "", params.step4Speed, "C/h", 9, false);
  drawParam(80, 60, "<", params.step4Target, "C", 10, false);
  
  // Icône settings (S) en haut à droite
  u8g2.setFont(u8g2_font_6x10_tf);
  if (tempFailActive) {
    // Afficher avertissement si problème de température
    u8g2.drawStr(90, 10, "WARN");
  } else {
    // Icône settings avec effet de sélection
    if (selectedParam == 11 && editMode == NAV_MODE) {
      u8g2.drawFrame(113, 0, 15, 12);
      u8g2.drawStr(115, 10, "S");
    } else if (selectedParam == 11 && editMode == EDIT_MODE) {
      u8g2.setDrawColor(1);
      u8g2.drawBox(113, 0, 15, 12);
      u8g2.setDrawColor(0);
      u8g2.drawStr(115, 10, "S");
      u8g2.setDrawColor(1);
    } else {
      u8g2.drawStr(115, 10, "S");
    }
  }
}

// Fonction helper : affiche un paramètre de settings avec effet de sélection
// Layout : label à gauche, valeur alignée à droite
// - NAV_MODE : cadre sur toute la largeur
// - EDIT_MODE : inversion vidéo sur toute la largeur
void drawSettingParam(int y, const char* label, const char* value, int index) {
  // Calculer la position de la valeur pour alignement à droite
  int valueWidth = strlen(value) * 6;
  int valueX = 128 - valueWidth;
  
  // Dessiner le label et la valeur
  u8g2.drawStr(0, y, label);
  u8g2.drawStr(valueX, y, value);
  
  // Effet de sélection si c'est le paramètre actif
  if (selectedSetting == index) {
    int totalWidth = 128; // Largeur complète de l'écran
    if (editMode == NAV_MODE) {
      // Mode navigation : cadre
      u8g2.drawFrame(0, y - 9, totalWidth, 11);
    } else if (editMode == EDIT_MODE) {
      // Mode édition : inversion vidéo
      u8g2.setDrawColor(1);
      u8g2.drawBox(0, y - 9, totalWidth, 11);
      u8g2.setDrawColor(0);
      u8g2.drawStr(2, y, label);
      u8g2.drawStr(valueX, y, value);
      u8g2.setDrawColor(1);
    }
  }
}

void drawSettingsScreen() {
  // Écran de configuration des paramètres avancés (PID et cycle de chauffe)
  u8g2.setFont(u8g2_font_6x10_tf);
  
  // Titre
  u8g2.drawStr(0, 8, "SETTINGS");
  
  // Heat Cycle : durée du cycle PWM en millisecondes
  snprintf(sharedBuffer, 20, "%dms", settings.pcycle);
  drawSettingParam(20, "Heat Cycle", sharedBuffer, 0);
  
  // Kp : gain proportionnel du PID (utilise la valeur réelle depuis temperature.cpp)
  float kpValue = KP;
  if (isnan(kpValue) || kpValue < 0.0) kpValue = 2.0;
  dtostrf(kpValue, 4, 1, sharedBuffer);
  drawSettingParam(32, "Kp", sharedBuffer, 1);
  
  // Ki : gain intégral du PID
  float kiValue = KI;
  if (isnan(kiValue) || kiValue < 0.0) kiValue = 0.5;
  dtostrf(kiValue, 4, 1, sharedBuffer);
  drawSettingParam(44, "Ki", sharedBuffer, 2);
  
  // Kd : gain dérivé du PID
  float kdValue = KD;
  if (isnan(kdValue) || kdValue < 0.0) kdValue = 0.0;
  dtostrf(kdValue, 4, 1, sharedBuffer);
  drawSettingParam(56, "Kd", sharedBuffer, 3);
  
  // Exit : retour à l'écran principal
  u8g2.drawStr(0, 64, "Exit");
  
  // Effet de sélection pour Exit
  if (selectedSetting == 4) {
    int exitWidth = 4 * 6; // "Exit" = 4 caractères * 6 pixels
    if (editMode == NAV_MODE) {
      u8g2.drawFrame(0, 55, exitWidth + 2, 11);
    } else if (editMode == EDIT_MODE) {
      u8g2.setDrawColor(1);
      u8g2.drawBox(0, 55, exitWidth + 2, 11);
      u8g2.setDrawColor(0);
      u8g2.drawStr(2, 64, "Exit");
      u8g2.setDrawColor(1);
    }
  }
}

void drawProgOnScreen(unsigned long currentMillis) {
  // Écran de cuisson en cours : affiche les informations de la phase active
  float currentTemp = getCurrentTemperature();
  int powerHold = getPowerHold();
  
  u8g2.setFont(u8g2_font_6x10_tf);
  
  // Déterminer les informations de la phase en cours
  const char* phaseTitle = "";
  int phaseTargetTemp = 0;
  int phaseSpeed = 0;
  int phaseWait = 0;
  float phaseStartTemp = 0.0;
  
  switch (currentPhase) {
    case PHASE_1:
      phaseTitle = "Phase 1";
      phaseTargetTemp = params.step1Temp;
      phaseSpeed = params.step1Speed;
      phaseWait = params.step1Wait;
      // Pour Phase 1, on utilise 0 comme température de départ (approximation)
      phaseStartTemp = 0.0;
      break;
    case PHASE_2:
      phaseTitle = "Phase 2";
      phaseTargetTemp = params.step2Temp;
      phaseSpeed = params.step2Speed;
      phaseWait = params.step2Wait;
      phaseStartTemp = params.step1Temp;
      break;
    case PHASE_3:
      phaseTitle = "Phase 3";
      phaseTargetTemp = params.step3Temp;
      phaseSpeed = params.step3Speed;
      phaseWait = params.step3Wait;
      phaseStartTemp = params.step2Temp;
      break;
    case PHASE_4_COOLDOWN:
      phaseTitle = "Cool Down";
      phaseTargetTemp = params.step4Target;
      phaseSpeed = params.step4Speed;
      phaseWait = 0;
      phaseStartTemp = params.step3Temp;
      break;
    default:
      phaseTitle = "Unknown";
      break;
  }
  
  // Affichage du titre
  u8g2.drawStr(0, 10, phaseTitle);
  
  // Warning si actif
  if (tempFailActive) {
    u8g2.drawStr(90, 10, "WARN");
  }
  
  // Ligne suivante : résumé des valeurs de la phase
  u8g2.setFont(u8g2_font_5x7_tf);
  if (currentPhase == PHASE_4_COOLDOWN) {
    snprintf(sharedBuffer, 20, "%dC/h->%dC", phaseSpeed, phaseTargetTemp);
  } else {
    snprintf(sharedBuffer, 20, "%dC/h->%dC,%dm", phaseSpeed, phaseTargetTemp, phaseWait);
  }
  u8g2.drawStr(0, 20, sharedBuffer);
  
  // Temp Read (température de la sonde)
  if (isnan(currentTemp)) {
    snprintf(sharedBuffer, 20, "Temp Read:?C");
  } else {
    snprintf(sharedBuffer, 20, "Temp Read:%dC", (int)(currentTemp + 0.5));
  }
  u8g2.drawStr(0, 30, sharedBuffer);
  
  // Temp Target (température cible calculée)
  snprintf(sharedBuffer, 20, "Temp Target:%dC", (int)(targetTemp + 0.5));
  u8g2.drawStr(0, 40, sharedBuffer);
  
  // Heat Power (PowerHold)
  snprintf(sharedBuffer, 20, "Heat Power:%d%%", powerHold);
  u8g2.drawStr(0, 50, sharedBuffer);
  
  // Phase : pourcentage de progression de la phase en cours
  int phasePercent = 0;
  if (!isnan(currentTemp)) {
    if (currentPhase == PHASE_4_COOLDOWN) {
      // Cool Down : progression basée sur la descente en température
      float tempRange = phaseStartTemp - phaseTargetTemp;
      if (tempRange > 0) {
        float progress = (phaseStartTemp - currentTemp) / tempRange;
        phasePercent = (int)(progress * 100);
        if (phasePercent < 0) phasePercent = 0;
        if (phasePercent > 100) phasePercent = 100;
      }
    } else {
      // Phases 1, 2, 3 : progression basée sur la montée en température
      if (!plateauReached) {
        // Phase de montée : calculer le pourcentage par rapport à la cible
        float tempRange = phaseTargetTemp - phaseStartTemp;
        if (tempRange > 0) {
          float progress = (currentTemp - phaseStartTemp) / tempRange;
          phasePercent = (int)(progress * 100);
          if (phasePercent < 0) phasePercent = 0;
          if (phasePercent > 100) phasePercent = 100;
        } else if (currentTemp >= phaseTargetTemp) {
          phasePercent = 100;
        }
      } else {
        // Plateau atteint : afficher 100%
        phasePercent = 100;
      }
    }
  }
  snprintf(sharedBuffer, 20, "Phase:%d%%", phasePercent > 100 ? 100 : phasePercent);
  u8g2.drawStr(0, 60, sharedBuffer);
}

#ifdef ENABLE_GRAPH
void drawGraph() {
  // Graphique désactivé pour économiser la mémoire programme
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.drawStr(20, 32, "Graph disabled");
}
#endif
