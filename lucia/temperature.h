/*
 * temperature.h - Gestion de la température et du contrôle PID
 */

#ifndef TEMPERATURE_H
#define TEMPERATURE_H

#include <Adafruit_MAX31856.h>

// External references
extern Adafruit_MAX31856 max31856;

// PID Parameters
#define KP 2.0
#define KI 0.5
#define KD 0.0
#define MAX_POWER_CHANGE 10.0

// PWM Parameters
#define CYCLE_LENGTH 1000  // 1 second in milliseconds

// Function declarations
void initTemperatureControl();
float readTemperature();
void updateTemperatureControl(float currentTemp, float targetTemp, bool enabled);
void setRelay(bool state);
int getPowerHold();
void resetPID();

#endif

