/*
 * display.cpp - Gestion de l'affichage OLED
 */

#include "definitions.h"
#include "display.h"
#include "temperature.h"

// Buffer partagé pour économiser la RAM (utilisé par toutes les fonctions d'affichage)
static char sharedBuffer[20];

// Fonction helper : affiche un paramètre avec effet de sélection (disposition grille)
void drawParamInline(int x, int y, const char* label, int value, const char* unit, int paramIndex) {
  snprintf(sharedBuffer, 20, "%s%d%s", label, value, unit);
  
  if (selectedParam == paramIndex && editMode == NAV_MODE) {
    // Cadre pour la navigation
    int width = strlen(sharedBuffer) * 6;
    u8g2.drawStr(x, y, sharedBuffer);
    u8g2.drawFrame(x - 1, y - 9, width + 2, 11);
  } else if (selectedParam == paramIndex && editMode == EDIT_MODE) {
    // Inversion vidéo pour l'édition
    int width = strlen(sharedBuffer) * 6;
    u8g2.setDrawColor(1);
    u8g2.drawBox(x - 1, y - 9, width + 2, 11);
    u8g2.setDrawColor(0);
    u8g2.drawStr(x, y, sharedBuffer);
    u8g2.setDrawColor(1);
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
  drawParamInline(18, 22, "", params.step1Speed, "C/h", 0);
  drawParamInline(60, 22, ">", params.step1Temp, "C", 1);
  drawParamInline(105, 22, "", params.step1Wait, "m", 2);
  
  // Phase 2 - Ligne 2
  u8g2.drawStr(0, 34, "P2:");
  drawParamInline(18, 34, "", params.step2Speed, "C/h", 3);
  drawParamInline(60, 34, ">", params.step2Temp, "C", 4);
  drawParamInline(105, 34, "", params.step2Wait, "m", 5);
  
  // Phase 3 - Ligne 3
  u8g2.drawStr(0, 46, "P3:");
  drawParamInline(18, 46, "", params.step3Speed, "C/h", 6);
  drawParamInline(60, 46, ">", params.step3Temp, "C", 7);
  drawParamInline(105, 46, "", params.step3Wait, "m", 8);
  
  // Cooldown - Ligne 4
  u8g2.drawStr(0, 58, "Cool:");
  drawParamInline(30, 58, "", params.step4Speed, "C/h", 9);
  drawParamInline(80, 58, "<", params.step4Target, "C", 10);
  
  // Settings en haut à droite
  if (tempFailActive) {
    u8g2.drawStr(90, 10, "WARN");
  } else {
    if (selectedParam == 11 && editMode == NAV_MODE) {
      // Cadre de sélection autour de "Settings"
      u8g2.drawFrame(73, 0, 55, 12);
      u8g2.drawStr(75, 10, "Settings");
    } else if (selectedParam == 11 && editMode == EDIT_MODE) {
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

// Fonction helper : calcule l'offset de défilement pour Settings
void updateSettingsScrollOffset() {
  const int SETTINGS_ITEMS_PER_PAGE = 4; // 4 lignes visibles (optimisé pour écran 64px)
  
  // Si l'élément sélectionné est au-dessus de la fenêtre visible
  if (selectedSetting < settingsScrollOffset) {
    settingsScrollOffset = selectedSetting;
  }
  
  // Si l'élément sélectionné est en dessous de la fenêtre visible
  if (selectedSetting >= settingsScrollOffset + SETTINGS_ITEMS_PER_PAGE) {
    settingsScrollOffset = selectedSetting - SETTINGS_ITEMS_PER_PAGE + 1;
  }
  
  // Contraindre l'offset dans les limites
  if (settingsScrollOffset < 0) {
    settingsScrollOffset = 0;
  }
  
  int maxOffset = NUM_SETTINGS - SETTINGS_ITEMS_PER_PAGE;
  if (maxOffset < 0) maxOffset = 0;
  if (settingsScrollOffset > maxOffset) {
    settingsScrollOffset = maxOffset;
  }
}

// Fonction helper : dessine un indicateur de scroll pour Settings
void drawSettingsScrollIndicator() {
  const int SETTINGS_ITEMS_PER_PAGE = 4;
  
  if (NUM_SETTINGS <= SETTINGS_ITEMS_PER_PAGE) {
    return; // Pas besoin si tout tient
  }
  
  // Flèche haut
  if (settingsScrollOffset > 0) {
    u8g2.drawStr(120, 20, "^");
  }
  
  // Flèche bas
  if (settingsScrollOffset < NUM_SETTINGS - SETTINGS_ITEMS_PER_PAGE) {
    u8g2.drawStr(120, 62, "v");
  }
}

// Fonction helper : dessine un élément de settings
void drawSettingsItem(int itemIndex, int y) {
  const char* label = "";
  // Déclarer les variables float avant le switch pour éviter les problèmes de scope
  float kpValue, kiValue, kdValue;
  
  switch (itemIndex) {
    case 0: // Heat Cycle
      label = "Heat Cycle";
      snprintf(sharedBuffer, 20, "%dms", settings.pcycle);
      break;
      
    case 1: // Kp
      label = "Kp";
      kpValue = KP;
      if (isnan(kpValue) || kpValue < 0.0) kpValue = 2.0;
      dtostrf(kpValue, 4, 1, sharedBuffer);
      break;
      
    case 2: // Ki
      label = "Ki";
      kiValue = KI;
      if (isnan(kiValue) || kiValue < 0.0) kiValue = 0.5;
      dtostrf(kiValue, 4, 1, sharedBuffer);
      break;
      
    case 3: // Kd
      label = "Kd";
      kdValue = KD;
      if (isnan(kdValue) || kdValue < 0.0) kdValue = 0.0;
      dtostrf(kdValue, 4, 1, sharedBuffer);
      break;
      
    case 4: // Max delta
      label = "Max delta";
      snprintf(sharedBuffer, 20, "%dC", settings.maxDelta);
      break;
      
    case 5: // Exit
      label = "Exit";
      sharedBuffer[0] = '\0'; // Pas de valeur pour Exit
      break;
  }
  
  // Dessiner le label
  u8g2.drawStr(2, y, label);
  
  // Dessiner la valeur (si elle existe)
  if (sharedBuffer[0] != '\0') {
    int valueWidth = strlen(sharedBuffer) * 6;
    int valueX = 126 - valueWidth; // Aligné à droite avant la barre de scroll
    u8g2.drawStr(valueX, y, sharedBuffer);
  }
}

void drawSettingsScreen() {
  // Écran Settings avec défilement (liste scrollable)
  u8g2.setFont(u8g2_font_6x10_tf);
  
  // Titre en haut
  u8g2.drawStr(0, 10, "SETTINGS");
  
  // Calculer le défilement
  updateSettingsScrollOffset();
  
  // Afficher la liste scrollable (4 éléments visibles pour tenir dans 64px)
  const int SETTINGS_ITEMS_PER_PAGE = 4;
  int lineHeight = 13;
  int startY = 23; // Après le titre
  
  for (int i = 0; i < SETTINGS_ITEMS_PER_PAGE; i++) {
    int itemIndex = settingsScrollOffset + i;
    
    // Vérifier si l'élément existe
    if (itemIndex >= NUM_SETTINGS) break;
    
    int itemY = startY + (i * lineHeight);
    bool isSelected = (itemIndex == selectedSetting);
    
    // Effet de sélection
    if (isSelected && editMode == NAV_MODE) {
      // Cadre pour la navigation (toute la largeur de l'écran)
      u8g2.drawFrame(0, itemY - 10, 128, 12);
    } else if (isSelected && editMode == EDIT_MODE) {
      // Inversion vidéo pour l'édition (toute la largeur de l'écran)
      u8g2.setDrawColor(1);
      u8g2.drawBox(0, itemY - 10, 128, 12);
      u8g2.setDrawColor(0);
    }
    
    // Dessiner l'élément
    drawSettingsItem(itemIndex, itemY);
    
    // Restaurer la couleur normale
    if (isSelected && editMode == EDIT_MODE) {
      u8g2.setDrawColor(1);
    }
  }
  
  // Dessiner l'indicateur de scroll
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
  if (isnan(currentTemp)) {
    snprintf(sharedBuffer, 20, "?C");
  } else {
    snprintf(sharedBuffer, 20, "%dC", (int)(currentTemp + 0.5));
  }
  int tempReadWidth = strlen(sharedBuffer) * 6;
  u8g2.drawStr(128 - tempReadWidth, 31, sharedBuffer);
  
  // Temp Target (température cible calculée)
  u8g2.drawStr(0, 42, "Temp Target");
  snprintf(sharedBuffer, 20, "%dC", (int)(targetTemp + 0.5));
  int tempTargetWidth = strlen(sharedBuffer) * 6;
  u8g2.drawStr(128 - tempTargetWidth, 42, sharedBuffer);
  
  // Heat Power (PowerHold)
  u8g2.drawStr(0, 53, "Heat Power");
  snprintf(sharedBuffer, 20, "%d%%", powerHold);
  int heatPowerWidth = strlen(sharedBuffer) * 6;
  u8g2.drawStr(128 - heatPowerWidth, 53, sharedBuffer);
  
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
  u8g2.drawStr(0, 63, "Phase");
  snprintf(sharedBuffer, 20, "%d%%", phasePercent > 100 ? 100 : phasePercent);
  int phaseWidth = strlen(sharedBuffer) * 6;
  u8g2.drawStr(128 - phaseWidth, 63, sharedBuffer);
}

// Convertit un uint8_t en température float (pour affichage)
float uint8ToTempDisplay(uint8_t value) {
  return (float)value * 1280.0 / 255.0;
}

// Calcule la température de consigne théorique à un temps donné (en secondes)
float calculateTheoreticalTarget(uint16_t timeSeconds) {
  // Recalculer les durées théoriques de chaque phase (avec paliers)
  float startTemp = 20.0; // Température de départ (approximation)
  
  // Phase 1 : montée + palier
  float phase1Ramp = 0;
  if (params.step1Temp > startTemp && params.step1Speed > 0) {
    phase1Ramp = (params.step1Temp - startTemp) * 3600.0 / params.step1Speed;
  }
  float phase1Plateau = params.step1Wait * 60.0; // Conversion minutes → secondes
  float phase1Total = phase1Ramp + phase1Plateau;
  
  // Phase 2 : montée + palier
  float phase2Ramp = 0;
  if (params.step2Temp > params.step1Temp && params.step2Speed > 0) {
    phase2Ramp = (params.step2Temp - params.step1Temp) * 3600.0 / params.step2Speed;
  }
  float phase2Plateau = params.step2Wait * 60.0;
  float phase2Total = phase2Ramp + phase2Plateau;
  
  // Phase 3 : montée + palier
  float phase3Ramp = 0;
  if (params.step3Temp > params.step2Temp && params.step3Speed > 0) {
    phase3Ramp = (params.step3Temp - params.step2Temp) * 3600.0 / params.step3Speed;
  }
  float phase3Plateau = params.step3Wait * 60.0;
  float phase3Total = phase3Ramp + phase3Plateau;
  
  // Points de transition temporels
  float endPhase1Ramp = phase1Ramp;
  float endPhase1 = phase1Total;
  float endPhase2Ramp = endPhase1 + phase2Ramp;
  float endPhase2 = endPhase1 + phase2Total;
  float endPhase3Ramp = endPhase2 + phase3Ramp;
  float endPhase3 = endPhase2 + phase3Total;
  
  // Déterminer la température à ce temps
  if (timeSeconds <= endPhase1Ramp) {
    // Phase 1 - Rampe de montée
    if (phase1Ramp > 0) {
      float progress = timeSeconds / phase1Ramp;
      return startTemp + (params.step1Temp - startTemp) * progress;
    }
    return params.step1Temp;
  } else if (timeSeconds <= endPhase1) {
    // Phase 1 - Palier
    return params.step1Temp;
  } else if (timeSeconds <= endPhase2Ramp) {
    // Phase 2 - Rampe de montée
    if (phase2Ramp > 0) {
      float progress = (timeSeconds - endPhase1) / phase2Ramp;
      return params.step1Temp + (params.step2Temp - params.step1Temp) * progress;
    }
    return params.step2Temp;
  } else if (timeSeconds <= endPhase2) {
    // Phase 2 - Palier
    return params.step2Temp;
  } else if (timeSeconds <= endPhase3Ramp) {
    // Phase 3 - Rampe de montée
    if (phase3Ramp > 0) {
      float progress = (timeSeconds - endPhase2) / phase3Ramp;
      return params.step2Temp + (params.step3Temp - params.step2Temp) * progress;
    }
    return params.step3Temp;
  } else if (timeSeconds <= endPhase3) {
    // Phase 3 - Palier
    return params.step3Temp;
  } else {
    // Au-delà de la phase 3
    return params.step3Temp;
  }
}

void drawGraph() {
  // Écran de graphe : affiche la courbe de consigne et les points mesurés
  u8g2.setFont(u8g2_font_6x10_tf);
  
  // Titre
  u8g2.drawStr(0, 8, "Graph");
  
  // Définir la zone de graphe : 10px de marge en haut pour titre, 54px de hauteur
  const int GRAPH_X = 10;
  const int GRAPH_Y = 10;
  const int GRAPH_WIDTH = 118;
  const int GRAPH_HEIGHT = 50;
  
  // Dessiner le cadre du graphe
  u8g2.drawFrame(GRAPH_X - 1, GRAPH_Y - 1, GRAPH_WIDTH + 2, GRAPH_HEIGHT + 2);
  
  // Afficher les valeurs résultantes du PID verticalement, décalés de 3px vers la droite
  // Position X : GRAPH_X + 1 = 11 (décalé de 3px depuis x=8)
  const int PID_X = GRAPH_X + 1;
  
  // Cache des valeurs PID résultantes avec mise à jour toutes les secondes
  static float cachedP = 0.0;
  static float cachedI = 0.0;
  static float cachedD = 0.0;
  static unsigned long lastPidUpdate = 0;
  unsigned long currentMillis = millis();
  
  // Mettre à jour le cache toutes les 1000ms
  if (currentMillis - lastPidUpdate >= 1000) {
    cachedP = getPIDProportional();
    cachedI = getPIDIntegral();
    cachedD = getPIDDerivative();
    lastPidUpdate = currentMillis;
  }
  
  // Buffer pour conversion float → string (dtostrf pour compatibilité Arduino)
  char floatBuf[8];
  char pidBuf[12];
  
  // Terme P sur la première ligne (Y = 20)
  dtostrf(cachedP, 4, 1, floatBuf); // 4 caractères total, 1 décimale (ex: "12.5" ou "-3.2")
  snprintf(pidBuf, 12, "P:%s", floatBuf);
  u8g2.drawStr(PID_X, 20, pidBuf);
  
  // Terme I sur la deuxième ligne (Y = 30)
  dtostrf(cachedI, 4, 1, floatBuf);
  snprintf(pidBuf, 12, "I:%s", floatBuf);
  u8g2.drawStr(PID_X, 30, pidBuf);
  
  // Terme D sur la troisième ligne (Y = 40)
  dtostrf(cachedD, 4, 1, floatBuf);
  snprintf(pidBuf, 12, "D:%s", floatBuf);
  u8g2.drawStr(PID_X, 40, pidBuf);
  
  // Calculer la durée totale théorique du programme (AVEC les temps d'attente/paliers)
  float startTemp = 20.0; // Température de départ approximative
  
  // Phase 1 : montée + palier
  float phase1Ramp = 0;
  if (params.step1Temp > startTemp && params.step1Speed > 0) {
    phase1Ramp = (params.step1Temp - startTemp) * 3600.0 / params.step1Speed;
  }
  float phase1Plateau = params.step1Wait * 60.0; // Minutes → secondes
  float phase1Total = phase1Ramp + phase1Plateau;
  
  // Phase 2 : montée + palier
  float phase2Ramp = 0;
  if (params.step2Temp > params.step1Temp && params.step2Speed > 0) {
    phase2Ramp = (params.step2Temp - params.step1Temp) * 3600.0 / params.step2Speed;
  }
  float phase2Plateau = params.step2Wait * 60.0;
  float phase2Total = phase2Ramp + phase2Plateau;
  
  // Phase 3 : montée + palier
  float phase3Ramp = 0;
  if (params.step3Temp > params.step2Temp && params.step3Speed > 0) {
    phase3Ramp = (params.step3Temp - params.step2Temp) * 3600.0 / params.step3Speed;
  }
  float phase3Plateau = params.step3Wait * 60.0;
  float phase3Total = phase3Ramp + phase3Plateau;
  
  // Durée totale incluant tous les paliers
  float totalDuration = phase1Total + phase2Total + phase3Total;
  if (totalDuration < 60) totalDuration = 60; // Minimum 1 minute pour éviter division par zéro
  
  // Limites du graphe : toujours de 0 à la durée totale du programme
  uint16_t minTime = 0;
  uint16_t maxTime = (uint16_t)totalDuration;
  
  // Échelle température : 0°C à step3Temp (température max du programme)
  float tempMin = 0;
  float tempMax = params.step3Temp;
  if (tempMax < 100) tempMax = 100; // Minimum 100°C pour l'échelle
  
  // Dessiner la courbe de consigne théorique complète (ligne continue sur toute la durée)
  int lastX = -1, lastY = -1;
  for (int px = 0; px < GRAPH_WIDTH; px++) {
    // Calculer le temps correspondant à ce pixel (de 0 à durée totale)
    float timeAtPixel = minTime + (float)(maxTime - minTime) * px / GRAPH_WIDTH;
    
    // Calculer la température théorique à ce temps
    float theoreticalTemp = calculateTheoreticalTarget((uint16_t)timeAtPixel);
    
    // Mapper la température sur l'axe Y
    int y = GRAPH_Y + GRAPH_HEIGHT - 1 - (int)((theoreticalTemp - tempMin) * (GRAPH_HEIGHT - 1) / (tempMax - tempMin));
    
    // Limiter aux bornes du graphe
    if (y < GRAPH_Y) y = GRAPH_Y;
    if (y > GRAPH_Y + GRAPH_HEIGHT - 1) y = GRAPH_Y + GRAPH_HEIGHT - 1;
    
    int x = GRAPH_X + px;
    
    // Dessiner la ligne
    if (lastX >= 0) {
      u8g2.drawLine(lastX, lastY, x, y);
    }
    
    lastX = x;
    lastY = y;
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
      
      // Dessiner un petit carré 2×2 pour le point
      u8g2.drawBox(x - 1, y - 1, 2, 2);
    }
  }
  
  // Afficher les valeurs de temps en bas (0 et durée totale)
  u8g2.drawStr(GRAPH_X, 64, "0");
  
  // Afficher la durée totale (en heures si > 60min, sinon en minutes)
  if (maxTime >= 3600) {
    snprintf(sharedBuffer, 20, "%dh%02d", maxTime / 3600, (maxTime % 3600) / 60);
  } else {
    snprintf(sharedBuffer, 20, "%dm", maxTime / 60);
  }
  int maxTimeStrWidth = strlen(sharedBuffer) * 6;
  u8g2.drawStr(GRAPH_X + GRAPH_WIDTH - maxTimeStrWidth, 64, sharedBuffer);
  
  // Afficher la température max en haut à droite du graphe
  snprintf(sharedBuffer, 20, "%dC", (int)tempMax);
  int tempMaxStrWidth = strlen(sharedBuffer) * 6;
  u8g2.drawStr(GRAPH_X + GRAPH_WIDTH - tempMaxStrWidth + 10, GRAPH_Y + 8, sharedBuffer);
}
