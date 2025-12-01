/*
 * temperature.cpp - Gestion de la température et du contrôle PID
 */

#include <Arduino.h>
#include "definitions.h"
#include "temperature.h"

// Pin definitions
#define PIN_RELAY 6
#define PIN_LED A1

// Global variables
float powerHold = 0;        // 0-100%
bool powerON = false;
unsigned long pwmCycleStart = 0;

// PID variables (optimized for RAM)
int integralError = 0;      // Scaled by 100
int lastError = 0;          // Scaled by 100
int lastPowerHold = 0;      // 0-10000 (scaled by 100)
unsigned long lastPIDUpdate = 0;

void initTemperatureControl() {
  pinMode(PIN_RELAY, OUTPUT);
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_RELAY, LOW);
  digitalWrite(PIN_LED, LOW);
  powerHold = 0;
  powerON = false;
  integralError = 0;
  lastError = 0;
  lastPowerHold = 0;
  lastPIDUpdate = millis();
}

float readTemperature() {
  float temp = max31856.readThermocoupleTemperature();
  
  // Check for faults
  uint8_t fault = max31856.readFault();
  if (fault) {
    return NAN;
  }
  
  return temp;
}

void resetPID() {
  integralError = 0;
  lastError = 0;
  lastPowerHold = 0;
  lastPIDUpdate = millis();
}

void updateTemperatureControl(float currentTemp, float targetTemp, bool enabled) {
  unsigned long currentMillis = millis();
  
  if (!enabled) {
    powerHold = 0;
    setRelay(false);
    integralError = 0;
    lastError = 0;
    return;
  }
  
  // Calculate error (scaled by 100)
  int error = (int)((targetTemp - currentTemp) * 100);
  
  // Calculate time delta in seconds
  float dt = (currentMillis - lastPIDUpdate) / 1000.0;
  if (dt <= 0) dt = 0.001;
  lastPIDUpdate = currentMillis;
  
  // PID calculation (using integer math where possible)
  float proportional = KP * (error / 100.0);
  integralError += (int)(error * dt);
  
  // Anti-windup: limit integral term (scaled)
  int maxIntegral = (int)(10000.0 / KI);
  if (integralError > maxIntegral) integralError = maxIntegral;
  if (integralError < -maxIntegral) integralError = -maxIntegral;
  
  float integral = KI * (integralError / 100.0);
  
  // Calculate new power hold (0-10000 scaled)
  int newPowerHoldScaled = (int)((proportional + integral) * 100);
  
  // Limit power change rate (scaled)
  int powerChange = newPowerHoldScaled - lastPowerHold;
  if (powerChange > MAX_POWER_CHANGE * 100) {
    newPowerHoldScaled = lastPowerHold + (int)(MAX_POWER_CHANGE * 100);
  } else if (powerChange < -MAX_POWER_CHANGE * 100) {
    newPowerHoldScaled = lastPowerHold - (int)(MAX_POWER_CHANGE * 100);
  }
  
  // Constrain output (scaled 0-10000)
  if (newPowerHoldScaled > 10000) newPowerHoldScaled = 10000;
  if (newPowerHoldScaled < 0) newPowerHoldScaled = 0;
  
  powerHold = newPowerHoldScaled / 100.0;
  lastPowerHold = newPowerHoldScaled;
  lastError = error;
  
  // Software PWM control
  unsigned long cycleElapsed = currentMillis - pwmCycleStart;
  
  if (cycleElapsed >= CYCLE_LENGTH) {
    // Start new cycle
    pwmCycleStart = currentMillis;
    cycleElapsed = 0;
  }
  
  // Calculate ON time for this cycle
  unsigned long onTime = (newPowerHoldScaled * CYCLE_LENGTH) / 10000;
  
  // Set relay state based on position in cycle
  if (cycleElapsed < onTime && newPowerHoldScaled > 0) {
    setRelay(true);
  } else {
    setRelay(false);
  }
}

void setRelay(bool state) {
  powerON = state;
  digitalWrite(PIN_RELAY, state ? HIGH : LOW);
  digitalWrite(PIN_LED, state ? HIGH : LOW);
}

int getPowerHold() {
  return (int)powerHold;
}

