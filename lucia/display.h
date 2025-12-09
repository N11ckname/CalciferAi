/*
 * display.h - Gestion de l'affichage OLED
 */

#ifndef DISPLAY_H
#define DISPLAY_H

#include <U8g2lib.h>
#include "definitions.h"

// External references
extern U8G2_SH1106_128X64_NONAME_2_HW_I2C u8g2;
extern int selectedParam;
extern int selectedSetting;
extern EditMode editMode;
extern FiringParams params;
extern SettingsParams settings;
extern ProgramState progState;
extern Phase currentPhase;
extern float targetTemp;
extern bool tempFailActive;
extern unsigned long tempFailStartTime;
extern unsigned long programStartTime;
extern unsigned long phaseStartTime;
extern unsigned long plateauStartTime;
extern bool plateauReached;
#ifdef ENABLE_GRAPH
extern float graphActual[];
extern float graphTarget[];
#endif
extern int graphIndex;

// Function declarations
void drawProgOffScreen();
void drawProgOnScreen(unsigned long currentMillis); // currentMillis pour éviter millis() dans la fonction
void drawSettingsScreen();
void drawGraph();
void updateDisplay(unsigned long currentMillis); // currentMillis pour éviter les appels millis() supplémentaires
// Note: readTemperature() et getPowerHold() sont déclarées dans temperature.h

#endif

