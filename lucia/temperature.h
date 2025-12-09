/*
 * temperature.h - Gestion de la température et du contrôle PID
 */

#ifndef TEMPERATURE_H
#define TEMPERATURE_H

#include <Adafruit_MAX31856.h>

// External references
extern Adafruit_MAX31856 max31856;

// PID Parameters (maintenant des variables externes modifiables)
extern float KP;
extern float KI;
extern float KD;
#define MAX_POWER_CHANGE 10.0

// PWM Parameters
extern unsigned int CYCLE_LENGTH; // Cycle PWM en millisecondes (modifiable)
#define PID_UPDATE_INTERVAL 1000  // Calcul PID toutes les 1 seconde (suffisant pour un four)

// Function declarations
void initTemperatureControl();
float readTemperature();
float getCurrentTemperature(); // Retourne la température mise en cache (lue toutes les 500ms)
void updateTemperatureControl(float currentTemp, float targetTemp, bool enabled, unsigned long currentMillis);
void setRelay(bool state);
int getPowerHold();
void resetPID();

#endif

