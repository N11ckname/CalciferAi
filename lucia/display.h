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
extern EditMode editMode;
extern FiringParams params;
extern ProgramState progState;
extern Phase currentPhase;
extern float targetTemp;
extern bool tempFailActive;
extern unsigned long tempFailStartTime;
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
void drawProgOnScreen();
void drawGraph();
float readTemperature();
int getPowerHold();

#endif

