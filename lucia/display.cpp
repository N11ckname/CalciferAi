/*
 * display.cpp - Gestion de l'affichage OLED
 */

#include "definitions.h"
#include "display.h"
#include "temperature.h"

// Buffer partagé pour économiser la RAM (utilisé par toutes les fonctions d'affichage)
static char sharedBuffer[20];

void drawParamInline(int x, int y, const char* label, int value, const char* unit, int paramIndex) {
  snprintf(sharedBuffer, 20, "%s%d%s", label, value, unit);
  int w = strlen(sharedBuffer) * 6;
  
  if (selectedParam == paramIndex) {
    if (editMode == EDIT_MODE) {
      u8g2.setDrawColor(1);
      u8g2.drawBox(x-1, y-9, w+2, 11);
      u8g2.setDrawColor(0);
      u8g2.drawStr(x, y, sharedBuffer);
      u8g2.setDrawColor(1);
    } else {
      u8g2.drawStr(x, y, sharedBuffer);
      u8g2.drawFrame(x-1, y-9, w+2, 11);
    }
  } else {
    u8g2.drawStr(x, y, sharedBuffer);
  }
}

void drawProgOffScreen() {
  // Écran principal : disposition en grille avec une seule fonte
  float currentTemp = getCurrentTemperature();
  
  // Une seule fonte partout : u8g2_font_6x10_tf
  u8g2.setFont(u8g2_font_6x10_tf);
  
  // Température actuelle en haut
  if (isnan(currentTemp)) {
    u8g2.drawStr(0, 10, "?C");
  } else {
    int tempInt = (int)(currentTemp + 0.5);
    snprintf(sharedBuffer, 20, "%dC", tempInt);
    u8g2.drawStr(0, 10, sharedBuffer);
  }
  
  // Phase 1 - Ligne 1
  u8g2.drawStr(0, 22, "P1:");
  drawParamInline(18, 22, "", params.step1Speed, "C/h", 1);
  drawParamInline(60, 22, ">", params.step1Temp, "C", 2);
  drawParamInline(105, 22, "", params.step1Wait, "m", 3);
  
  // Phase 2 - Ligne 2
  u8g2.drawStr(0, 34, "P2:");
  drawParamInline(18, 34, "", params.step2Speed, "C/h", 4);
  drawParamInline(60, 34, ">", params.step2Temp, "C", 5);
  drawParamInline(105, 34, "", params.step2Wait, "m", 6);
  
  // Phase 3 - Ligne 3
  u8g2.drawStr(0, 46, "P3:");
  drawParamInline(18, 46, "", params.step3Speed, "C/h", 7);
  drawParamInline(60, 46, ">", params.step3Temp, "C", 8);
  drawParamInline(105, 46, "", params.step3Wait, "m", 9);
  
  // Cooldown - Ligne 4
  u8g2.drawStr(0, 58, "Cool:");
  drawParamInline(30, 58, "", params.step4Speed, "C/h", 10);
  drawParamInline(80, 58, "<", params.step4Target, "C", 11);
  
  // Settings en haut à droite (maintenant index 0)
  if (tempFailActive) {
    u8g2.drawStr(90, 10, "WARN");
  } else {
    if (selectedParam == 0 && editMode == NAV_MODE) {
      // Cadre de sélection autour de "Settings"
      u8g2.drawFrame(73, 0, 55, 12);
      u8g2.drawStr(75, 10, "Settings");
    } else if (selectedParam == 0 && editMode == EDIT_MODE) {
      // Inversion vidéo pour "Settings"
      u8g2.setDrawColor(1);
      u8g2.drawBox(73, 0, 55, 12);
      u8g2.setDrawColor(0);
      u8g2.drawStr(75, 10, "Settings");
      u8g2.setDrawColor(1);
    } else {
      // Affichage normal de "Settings"
      u8g2.drawStr(75, 10, "Settings");
    }
  }
}

void updateSettingsScrollOffset() {
  const int PER_PAGE = 4;
  if (selectedSetting < settingsScrollOffset) settingsScrollOffset = selectedSetting;
  if (selectedSetting >= settingsScrollOffset + PER_PAGE) settingsScrollOffset = selectedSetting - PER_PAGE + 1;
  if (settingsScrollOffset < 0) settingsScrollOffset = 0;
  int max = NUM_SETTINGS - PER_PAGE;
  if (max < 0) max = 0;
  if (settingsScrollOffset > max) settingsScrollOffset = max;
}

void drawSettingsScrollIndicator() {
  if (NUM_SETTINGS <= 4) return;
  if (settingsScrollOffset > 0) u8g2.drawStr(120, 20, "^");
  if (settingsScrollOffset < NUM_SETTINGS - 4) u8g2.drawStr(120, 62, "v");
}

void drawSettingsItem(int itemIndex, int y) {
  const char* label;
  
  switch (itemIndex) {
    case 0: 
      label = "Heat Cycle"; 
      snprintf(sharedBuffer, 20, "%dms", settings.pcycle); 
      break;
    case 1: 
      label = "Kp"; 
      dtostrf(KP, 4, 1, sharedBuffer); 
      break;
    case 2: 
      label = "Ki"; 
      dtostrf(KI, 6, 3, sharedBuffer);  // 3 décimales pour incrément de 0.005
      break;
    case 3:  // Kd supprimé, Max delta devient case 3
      label = "Max delta"; 
      snprintf(sharedBuffer, 20, "%dC", settings.maxDelta); 
      break;
    case 4:  // Exit devient case 4
      label = "Exit"; 
      strcpy(sharedBuffer, "<--");
      break;
  }
  
  u8g2.drawStr(2, y, label);
  u8g2.drawStr(126 - strlen(sharedBuffer) * 6, y, sharedBuffer);
}

void drawSettingsScreen() {
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.drawStr(0, 10, "SETTINGS");
  u8g2.drawStr(98, 10, "v01.0");  // Version alignée à droite
  updateSettingsScrollOffset();
  
  for (int i = 0; i < 4; i++) {
    int idx = settingsScrollOffset + i;
    if (idx >= NUM_SETTINGS) break;
    
    int y = 23 + (i * 13);
    bool sel = (idx == selectedSetting);
    
    if (sel) {
      if (editMode == EDIT_MODE) {
        u8g2.setDrawColor(1);
        u8g2.drawBox(0, y-10, 128, 12);
        u8g2.setDrawColor(0);
      } else {
        u8g2.drawFrame(0, y-10, 128, 12);
      }
    }
    
    drawSettingsItem(idx, y);
    if (sel && editMode == EDIT_MODE) u8g2.setDrawColor(1);
  }
  
  drawSettingsScrollIndicator();
}

void drawProgOnScreen(unsigned long currentMillis) {
  // Écran de cuisson en cours : affiche les informations de la phase active
  // Une seule fonte pour uniformité : u8g2_font_6x10_tf
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
  
  // Ligne suivante : résumé des valeurs de la phase (fonte uniforme)
  if (currentPhase == PHASE_4_COOLDOWN) {
    snprintf(sharedBuffer, 20, "%dC/h->%dC", phaseSpeed, phaseTargetTemp);
  } else {
    snprintf(sharedBuffer, 20, "%dC/h->%dC,%dm", phaseSpeed, phaseTargetTemp, phaseWait);
  }
  u8g2.drawStr(0, 20, sharedBuffer);
  
  // Ligne de séparation horizontale
  u8g2.drawHLine(0, 22, 128);
  
  // Temp Read (température de la sonde)
  u8g2.drawStr(0, 31, "Temp Read");
  snprintf(sharedBuffer, 20, isnan(currentTemp) ? "?C" : "%dC", (int)(currentTemp + 0.5));
  u8g2.drawStr(128 - strlen(sharedBuffer) * 6, 31, sharedBuffer);
  
  // Temp Target (température cible calculée)
  u8g2.drawStr(0, 42, "Temp Target");
  snprintf(sharedBuffer, 20, "%dC", (int)(targetTemp + 0.5));
  u8g2.drawStr(128 - strlen(sharedBuffer) * 6, 42, sharedBuffer);
  
  // Heat Power (PowerHold)
  u8g2.drawStr(0, 53, "Heat Power");
  snprintf(sharedBuffer, 20, "%d%%", powerHold);
  u8g2.drawStr(128 - strlen(sharedBuffer) * 6, 53, sharedBuffer);
  
  // Phase : pourcentage
  int pp = 0;
  if (!isnan(currentTemp)) {
    float range = (currentPhase == PHASE_4_COOLDOWN) ? 
                  (phaseStartTemp - phaseTargetTemp) : (phaseTargetTemp - phaseStartTemp);
    if (range > 0) {
      float prog = (currentPhase == PHASE_4_COOLDOWN) ? 
                   (phaseStartTemp - currentTemp) : (currentTemp - phaseStartTemp);
      pp = (int)((prog / range) * 100);
      if (pp < 0) pp = 0;
      if (pp > 100) pp = 100;
    } else if (plateauReached || currentTemp >= phaseTargetTemp) {
      pp = 100;
    }
  }
  u8g2.drawStr(0, 63, "Phase");
  snprintf(sharedBuffer, 20, "%d%%", pp);
  u8g2.drawStr(128 - strlen(sharedBuffer) * 6, 63, sharedBuffer);
}

#ifdef ENABLE_GRAPH
float uint8ToTempDisplay(uint8_t value) {
  return (float)value * 1280.0 / 255.0;
}

float calculateTheoreticalTarget(uint16_t t) {
  const float T0 = 20.0;
  // Calcul durées (secondes)
  uint16_t r1 = params.step1Speed ? (params.step1Temp - T0) * 3600UL / params.step1Speed : 0;
  uint16_t p1 = params.step1Wait * 60;
  uint16_t e1 = r1 + p1;
  
  uint16_t r2 = params.step2Speed ? (params.step2Temp - params.step1Temp) * 3600UL / params.step2Speed : 0;
  uint16_t p2 = params.step2Wait * 60;
  uint16_t e2 = e1 + r2 + p2;
  
  uint16_t r3 = params.step3Speed ? (params.step3Temp - params.step2Temp) * 3600UL / params.step3Speed : 0;
  uint16_t p3 = params.step3Wait * 60;
  
  // Déterminer température
  if (t <= r1) return r1 ? T0 + (params.step1Temp - T0) * t / r1 : params.step1Temp;
  if (t <= e1) return params.step1Temp;
  if (t <= e1 + r2) return r2 ? params.step1Temp + (params.step2Temp - params.step1Temp) * (t - e1) / r2 : params.step2Temp;
  if (t <= e2) return params.step2Temp;
  if (t <= e2 + r3) return r3 ? params.step2Temp + (params.step3Temp - params.step2Temp) * (t - e2) / r3 : params.step3Temp;
  return params.step3Temp;
}

void drawGraph() {
  // Écran de graphe : affiche la courbe de consigne et les points mesurés
  u8g2.setFont(u8g2_font_6x10_tf);
  
  // Définir la zone de graphe : agrandie pour utiliser tout l'écran
  const int GRAPH_X = 2;
  const int GRAPH_Y = 2;
  const int GRAPH_WIDTH = 124;
  const int GRAPH_HEIGHT = 60;
  
  // Dessiner le cadre du graphe
  u8g2.drawFrame(GRAPH_X - 1, GRAPH_Y - 1, GRAPH_WIDTH + 2, GRAPH_HEIGHT + 2);
  
  // Afficher les valeurs résultantes du PID verticalement, décalés de 3px vers la droite
  // Position X : GRAPH_X + 1 = 11 (décalé de 3px depuis x=8)
  const int PID_X = GRAPH_X + 2;
  
  // Cache des valeurs PID résultantes avec mise à jour toutes les secondes
  static float cachedP = 0.0;
  static float cachedI = 0.0;
  // cachedD supprimé : terme D non utilisé
  static unsigned long lastPidUpdate = 0;
  unsigned long currentMillis = millis();
  
  // Mettre à jour le cache toutes les 1000ms
  if (currentMillis - lastPidUpdate >= 1000) {
    cachedP = getPIDProportional();
    cachedI = getPIDIntegral();
    // getPIDDerivative() supprimé
    lastPidUpdate = currentMillis;
  }
  
  // Buffer pour conversion float → string (dtostrf pour compatibilité Arduino)
  char floatBuf[8];
  char pidBuf[12];
  
  // Terme P sur la première ligne (Y = 10)
  dtostrf(cachedP, 4, 1, floatBuf); // 4 caractères total, 1 décimale (ex: "12.5" ou "-3.2")
  snprintf(pidBuf, 12, "P:%s", floatBuf);
  u8g2.drawStr(PID_X, 10, pidBuf);
  
  // Terme I sur la deuxième ligne (Y = 20)
  dtostrf(cachedI, 4, 1, floatBuf);
  snprintf(pidBuf, 12, "I:%s", floatBuf);
  u8g2.drawStr(PID_X, 20, pidBuf);
  
  // Terme D supprimé : non utilisé pour four céramique
  
  // Calcul durée totale optimisé
  uint32_t d = 0;
  if (params.step1Speed) d += ((uint32_t)(params.step1Temp - 20) * 3600UL) / params.step1Speed;
  d += (uint32_t)params.step1Wait * 60;
  if (params.step2Speed) d += ((uint32_t)(params.step2Temp - params.step1Temp) * 3600UL) / params.step2Speed;
  d += (uint32_t)params.step2Wait * 60;
  if (params.step3Speed) d += ((uint32_t)(params.step3Temp - params.step2Temp) * 3600UL) / params.step3Speed;
  d += (uint32_t)params.step3Wait * 60;
  uint16_t totalDuration = (d < 60) ? 60 : d;
  
  // Limites du graphe : toujours de 0 à la durée totale du programme
  uint16_t minTime = 0;
  uint16_t maxTime = (uint16_t)totalDuration;
  
  // Échelle température : 0°C à step3Temp (température max du programme)
  float tempMin = 0;
  float tempMax = params.step3Temp;
  if (tempMax < 100) tempMax = 100; // Minimum 100°C pour l'échelle
  
  // Courbe théorique (échantillonnage tous les 2 pixels pour réduire les calculs)
  int lastX = -1, lastY = -1;
  for (int px = 0; px < GRAPH_WIDTH; px += 2) {
    uint16_t t = minTime + (uint32_t)(maxTime - minTime) * px / GRAPH_WIDTH;
    float temp = calculateTheoreticalTarget(t);
    int y = GRAPH_Y + GRAPH_HEIGHT - 1 - (int)((temp - tempMin) * (GRAPH_HEIGHT - 1) / (tempMax - tempMin));
    if (y < GRAPH_Y) y = GRAPH_Y;
    if (y > GRAPH_Y + GRAPH_HEIGHT - 1) y = GRAPH_Y + GRAPH_HEIGHT - 1;
    int x = GRAPH_X + px;
    if (lastX >= 0) u8g2.drawLine(lastX, lastY, x, y);
    lastX = x; lastY = y;
  }
  
  // Dessiner les points de mesure réels (petits carrés) seulement s'il y en a
  if (graphCount > 0) {
    for (uint8_t i = 0; i < graphCount; i++) {
      uint8_t idx = i;
      if (graphCount == GRAPH_SIZE) {
        // Buffer plein : commencer à partir de graphIndex (le plus ancien)
        idx = (graphIndex + i) % GRAPH_SIZE;
      }
      
      uint16_t pointTime = graphTimeStamps[idx];
      float pointTemp = uint8ToTempDisplay(graphTempRead[idx]);
      
      // Mapper sur les axes (sur toute la durée du programme)
      int x = GRAPH_X + (int)((float)pointTime * GRAPH_WIDTH / maxTime);
      int y = GRAPH_Y + GRAPH_HEIGHT - 1 - (int)((pointTemp - tempMin) * (GRAPH_HEIGHT - 1) / (tempMax - tempMin));
      
      // Limiter aux bornes
      if (x < GRAPH_X) x = GRAPH_X;
      if (x > GRAPH_X + GRAPH_WIDTH - 1) x = GRAPH_X + GRAPH_WIDTH - 1;
      if (y < GRAPH_Y) y = GRAPH_Y;
      if (y > GRAPH_Y + GRAPH_HEIGHT - 1) y = GRAPH_Y + GRAPH_HEIGHT - 1;
      
      
      u8g2.drawBox(x - 1, y - 1, 2, 2);  // Carré de 2×2 pixels
    }
  }
  
  // Afficher la durée totale en bas à gauche (en heures si > 60min, sinon en minutes)
  if (maxTime >= 3600) {
    snprintf(sharedBuffer, 20, "%dh%02d", maxTime / 3600, (maxTime % 3600) / 60);
  } else {
    snprintf(sharedBuffer, 20, "%dm", maxTime / 60);
  }
  int maxTimeStrWidth = strlen(sharedBuffer) * 6;
  u8g2.drawStr(GRAPH_X + GRAPH_WIDTH - maxTimeStrWidth, 64, sharedBuffer);
  
  // Afficher la température max en haut à droite (aligné dynamiquement)
  snprintf(sharedBuffer, 20, "%dC", (int)tempMax);
  int tempMaxStrWidth = strlen(sharedBuffer) * 6;
  u8g2.drawStr(128 - tempMaxStrWidth, GRAPH_Y + 8, sharedBuffer);
}
#endif
