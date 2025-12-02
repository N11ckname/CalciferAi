/*
 * display.cpp - Gestion de l'affichage OLED
 */

#include "definitions.h"
#include "display.h"

// Helper function to draw parameter with selection
void drawParam(int x, int y, const char* label, int value, const char* unit, int paramIndex, bool drawBox) {
  char buffer[16];  // Réduit de 20 à 16
  snprintf(buffer, 16, "%s%d%s", label, value, unit);
  
  // Note: la fonte est déjà définie dans la fonction appelante
  
  if (selectedParam == paramIndex && editMode == NAV_MODE) {
    // Frame for selection (cadre d'un pixel)
    int width = strlen(buffer) * 6;
    u8g2.drawStr(x, y, buffer);
    u8g2.drawFrame(x - 1, y - 9, width + 2, 11);
  } else if (selectedParam == paramIndex && editMode == EDIT_MODE) {
    // Inverted colors for editing (négatif)
    int width = strlen(buffer) * 6;
    u8g2.setDrawColor(1);
    u8g2.drawBox(x - 1, y - 9, width + 2, 11);
    u8g2.setDrawColor(0);
    u8g2.drawStr(x, y, buffer);
    u8g2.setDrawColor(1);
  } else {
    u8g2.drawStr(x, y, buffer);
  }
}

void drawProgOffScreen() {
  float currentTemp = readTemperature();
  int powerHold = getPowerHold();
  
  char buf[16];
  
  // Current temperature - Large at top
  u8g2.setFont(u8g2_font_9x15_tf);
  snprintf(buf, 16, "%.0fC", currentTemp);
  u8g2.drawStr(0, 12, buf);
  
  // Temperature warning if active
  u8g2.setFont(u8g2_font_6x10_tf);
  if (tempFailActive) {
    u8g2.drawStr(90, 10, "WARN");
  } else {
    // Relay status
    snprintf(buf, 16, "%d%%", powerHold);
    u8g2.drawStr(90, 10, buf);
  }
  
  // Phase 1 - Ordre: gauche à droite
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.drawStr(0, 24, "P1:");
  drawParam(18, 24, "", params.step1Speed, "C/h", 0, false);   // Gauche
  drawParam(60, 24, ">", params.step1Temp, "C", 1, false);     // Centre
  drawParam(105, 24, "", params.step1Wait, "m", 2, false);     // Droite
  
  // Phase 2 - Ordre: gauche à droite
  u8g2.drawStr(0, 36, "P2:");
  drawParam(18, 36, "", params.step2Speed, "C/h", 3, false);   // Gauche
  drawParam(60, 36, ">", params.step2Temp, "C", 4, false);     // Centre
  drawParam(105, 36, "", params.step2Wait, "m", 5, false);     // Droite
  
  // Phase 3 - Ordre: gauche à droite
  u8g2.drawStr(0, 48, "P3:");
  drawParam(18, 48, "", params.step3Speed, "C/h", 6, false);   // Gauche
  drawParam(60, 48, ">", params.step3Temp, "C", 7, false);     // Centre
  drawParam(105, 48, "", params.step3Wait, "m", 8, false);     // Droite
  
  // Cooldown - Ordre: gauche à droite
  u8g2.drawStr(0, 60, "Cool:");
  drawParam(30, 60, "", params.step4Speed, "C/h", 9, false);   // Gauche
  drawParam(80, 60, "<", params.step4Target, "C", 10, false);  // Droite
}

void drawProgOnScreen() {
  float currentTemp = readTemperature();
  int powerHold = getPowerHold();
  
  u8g2.setFont(u8g2_font_5x7_tf);
  
  // Title
  u8g2.drawStr(0, 7, "ON");
  
  // Temperature warning if active
  if (tempFailActive) {
    u8g2.drawStr(50, 7, "WARN");
  }
  
  // Calculate elapsed time in current phase
  unsigned long elapsed = millis() - phaseStartTime;
  int minutes = elapsed / 60000;
  int seconds = (elapsed % 60000) / 1000;
  char timeStr[8];  // Réduit de 10 à 8
  if (minutes < 60) {
    snprintf(timeStr, 8, "%02d:%02d", minutes, seconds);
  } else {
    int hours = minutes / 60;
    minutes = minutes % 60;
    snprintf(timeStr, 8, "%02d:%02d", hours, minutes);
  }
  
  u8g2.drawStr(75, 7, timeStr);
  
  // Phase 1
  char buf[20];  // Réduit de 25 à 20
  if (currentPhase > PHASE_1) u8g2.setDrawColor(0); // Dim completed
  snprintf(buf, 20, "P1:%d>%d,%dm", params.step1Speed, params.step1Temp, params.step1Wait);
  u8g2.drawStr(0, 17, buf);
  if (currentPhase > PHASE_1) u8g2.setDrawColor(1);
  
  // Phase 2
  if (currentPhase == PHASE_1 || currentPhase > PHASE_2) u8g2.setDrawColor(0);
  snprintf(buf, 20, "P2:%d>%d,%dm", params.step2Speed, params.step2Temp, params.step2Wait);
  u8g2.drawStr(0, 27, buf);
  if (currentPhase == PHASE_1 || currentPhase > PHASE_2) u8g2.setDrawColor(1);
  
  // Phase 3
  if (currentPhase < PHASE_3 || currentPhase > PHASE_3) u8g2.setDrawColor(0);
  snprintf(buf, 20, "P3:%d>%d,%dm", params.step3Speed, params.step3Temp, params.step3Wait);
  u8g2.drawStr(0, 37, buf);
  if (currentPhase < PHASE_3 || currentPhase > PHASE_3) u8g2.setDrawColor(1);
  
  // Cooldown
  if (currentPhase != PHASE_4_COOLDOWN) u8g2.setDrawColor(0);
  snprintf(buf, 20, "Cool:%d<%dC", params.step4Speed, params.step4Target);
  u8g2.drawStr(0, 47, buf);
  if (currentPhase != PHASE_4_COOLDOWN) u8g2.setDrawColor(1);
  
  // Current temperature and target (in green - using normal draw)
  snprintf(buf, 20, "T:%.0f->%.0fC", currentTemp, targetTemp);
  u8g2.drawStr(0, 57, buf);
  
  // Relay status (in red - using normal draw)
  snprintf(buf, 20, "ON,%d%%", powerHold);
  u8g2.drawStr(90, 57, buf);
  
  // Calculate total remaining time
  unsigned long totalRemaining = 0;
  
  // Add remaining time for current phase
  if (currentPhase == PHASE_1) {
    float tempToGo = params.step1Temp - currentTemp;
    if (tempToGo > 0) {
      totalRemaining += (tempToGo / params.step1Speed) * 3600000;
    }
    if (plateauReached) {
      unsigned long plateauElapsed = millis() - plateauStartTime;
      unsigned long plateauRemaining = params.step1Wait * 60000UL - plateauElapsed;
      if (plateauRemaining > 0) totalRemaining += plateauRemaining;
    } else {
      totalRemaining += params.step1Wait * 60000UL;
    }
    // Add phases 2, 3, 4
    totalRemaining += ((params.step2Temp - params.step1Temp) / (float)params.step2Speed) * 3600000;
    totalRemaining += params.step2Wait * 60000UL;
    totalRemaining += ((params.step3Temp - params.step2Temp) / (float)params.step3Speed) * 3600000;
    totalRemaining += params.step3Wait * 60000UL;
    totalRemaining += ((params.step3Temp - params.step4Target) / (float)params.step4Speed) * 3600000;
  } else if (currentPhase == PHASE_2) {
    float tempToGo = params.step2Temp - currentTemp;
    if (tempToGo > 0) {
      totalRemaining += (tempToGo / params.step2Speed) * 3600000;
    }
    if (plateauReached) {
      unsigned long plateauElapsed = millis() - plateauStartTime;
      unsigned long plateauRemaining = params.step2Wait * 60000UL - plateauElapsed;
      if (plateauRemaining > 0) totalRemaining += plateauRemaining;
    } else {
      totalRemaining += params.step2Wait * 60000UL;
    }
    // Add phases 3, 4
    totalRemaining += ((params.step3Temp - params.step2Temp) / (float)params.step3Speed) * 3600000;
    totalRemaining += params.step3Wait * 60000UL;
    totalRemaining += ((params.step3Temp - params.step4Target) / (float)params.step4Speed) * 3600000;
  } else if (currentPhase == PHASE_3) {
    float tempToGo = params.step3Temp - currentTemp;
    if (tempToGo > 0) {
      totalRemaining += (tempToGo / params.step3Speed) * 3600000;
    }
    if (plateauReached) {
      unsigned long plateauElapsed = millis() - plateauStartTime;
      unsigned long plateauRemaining = params.step3Wait * 60000UL - plateauElapsed;
      if (plateauRemaining > 0) totalRemaining += plateauRemaining;
    } else {
      totalRemaining += params.step3Wait * 60000UL;
    }
    // Add phase 4
    totalRemaining += ((params.step3Temp - params.step4Target) / (float)params.step4Speed) * 3600000;
  } else if (currentPhase == PHASE_4_COOLDOWN) {
    float tempToGo = currentTemp - params.step4Target;
    if (tempToGo > 0) {
      totalRemaining += (tempToGo / params.step4Speed) * 3600000;
    }
  }
  
  int hoursRemaining = totalRemaining / 3600000;
  int minutesRemaining = (totalRemaining % 3600000) / 60000;
  char remainingStr[12];  // Réduit de 15 à 12
  snprintf(remainingStr, 12, "R:%dh%02d", hoursRemaining, minutesRemaining);
  u8g2.drawStr(0, 64, remainingStr);
}

#ifdef ENABLE_GRAPH
void drawGraph() {
  if (graphIndex < 2) {
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(20, 32, "Pas de donnees");
    return;
  }
  
  // Calculate elapsed time since program start
  unsigned long totalElapsed = millis() - programStartTime;
  int hours = totalElapsed / 3600000;
  int minutes = (totalElapsed % 3600000) / 60000;
  
  // Header: Phase + Time elapsed
  u8g2.setFont(u8g2_font_6x10_tf);
  char header[20];
  const char* phaseNames[] = {"", "P1", "P2", "P3", "Cool"};
  snprintf(header, 20, "%s T+%dh%02d", phaseNames[currentPhase], hours, minutes);
  u8g2.drawStr(0, 8, header);
  
  // Current temperatures
  float currentTemp = readTemperature();
  char tempStr[20];
  snprintf(tempStr, 20, "%.0f->%.0fC", currentTemp, targetTemp);
  u8g2.drawStr(70, 8, tempStr);
  
  // Find min and max temperatures for scaling
  float minTemp = graphActual[0];
  float maxTemp = graphActual[0];
  
  for (int i = 0; i < graphIndex; i++) {
    if (graphActual[i] < minTemp) minTemp = graphActual[i];
    if (graphActual[i] > maxTemp) maxTemp = graphActual[i];
    if (graphTarget[i] < minTemp) minTemp = graphTarget[i];
    if (graphTarget[i] > maxTemp) maxTemp = graphTarget[i];
  }
  
  // Add 10% margin
  float tempRange = maxTemp - minTemp;
  minTemp -= tempRange * 0.1;
  maxTemp += tempRange * 0.1;
  if (tempRange < 10) {
    minTemp -= 5;
    maxTemp += 5;
    tempRange = maxTemp - minTemp;
  }
  
  // Graph area: y from 14 to 62 (48 pixels)
  int graphHeight = 48;
  int graphTop = 14;
  int graphLeft = 20;
  int graphWidth = 107;  // 127 - 20
  
  // Draw axes
  u8g2.drawLine(graphLeft, graphTop, graphLeft, graphTop + graphHeight);
  u8g2.drawLine(graphLeft, graphTop + graphHeight, 127, graphTop + graphHeight);
  
  // Draw temperature labels
  u8g2.setFont(u8g2_font_5x7_tf);
  char tempLabel[6];
  snprintf(tempLabel, 6, "%d", (int)maxTemp);
  u8g2.drawStr(0, graphTop + 5, tempLabel);
  snprintf(tempLabel, 6, "%d", (int)minTemp);
  u8g2.drawStr(0, graphTop + graphHeight - 2, tempLabel);
  
  // Draw data points
  int pointsToShow = graphIndex;
  if (pointsToShow > graphWidth) pointsToShow = graphWidth;
  
  int startIndex = graphIndex - pointsToShow;
  if (startIndex < 0) startIndex = 0;
  
  for (int i = 0; i < pointsToShow - 1; i++) {
    int index = startIndex + i;
    int x1 = graphLeft + 1 + i;
    int x2 = x1 + 1;
    
    // Target temperature (dotted line)
    int y1_target = graphTop + graphHeight - ((graphTarget[index] - minTemp) / tempRange * graphHeight);
    int y2_target = graphTop + graphHeight - ((graphTarget[index + 1] - minTemp) / tempRange * graphHeight);
    
    if (i % 3 == 0) { // Dotted
      u8g2.drawLine(x1, y1_target, x2, y2_target);
    }
    
    // Actual temperature (solid line)
    int y1_actual = graphTop + graphHeight - ((graphActual[index] - minTemp) / tempRange * graphHeight);
    int y2_actual = graphTop + graphHeight - ((graphActual[index + 1] - minTemp) / tempRange * graphHeight);
    
    u8g2.drawLine(x1, y1_actual, x2, y2_actual);
  }
}
#endif
