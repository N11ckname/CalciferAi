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

// Fonction interne : gestion du PWM logiciel (doit s'exécuter à chaque loop)
void updatePWM(unsigned long currentMillis) {
  unsigned long cycleElapsed = currentMillis - pwmCycleStart;
  
  if (cycleElapsed >= CYCLE_LENGTH) {
    // Début d'un nouveau cycle PWM
    pwmCycleStart = currentMillis;
    cycleElapsed = 0;
  }
  
  // Calculer le temps ON pour ce cycle (lastPowerHold est en 0-10000)
  unsigned long onTime = (lastPowerHold * CYCLE_LENGTH) / 10000;
  
  // Activer/désactiver le relais selon la position dans le cycle
  if (cycleElapsed < onTime && lastPowerHold > 0) {
    setRelay(true);
  } else {
    setRelay(false);
  }
}

void updateTemperatureControl(float currentTemp, float targetTemp, bool enabled) {
  unsigned long currentMillis = millis();
  
  // Si désactivé : arrêter le chauffage et réinitialiser le PID
  if (!enabled) {
    powerHold = 0;
    lastPowerHold = 0;
    setRelay(false);
    integralError = 0;
    lastError = 0;
    return;
  }
  
  // Le PWM doit toujours s'exécuter à chaque loop pour un contrôle précis
  updatePWM(currentMillis);
  
  // Le calcul PID ne s'exécute qu'à l'intervalle défini (1 seconde)
  // Pour un four céramique, c'est largement suffisant (inertie thermique élevée)
  if (currentMillis - lastPIDUpdate < PID_UPDATE_INTERVAL) {
    return;  // Pas encore le moment de recalculer le PID
  }
  
  // Calculer le delta temps en secondes
  float dt = (currentMillis - lastPIDUpdate) / 1000.0;
  lastPIDUpdate = currentMillis;
  
  // Calculer l'erreur (mise à l'échelle x100)
  int error = (int)((targetTemp - currentTemp) * 100);
  
  // Calcul PID (utilisant l'arithmétique entière où possible)
  float proportional = KP * (error / 100.0);
  integralError += (int)(error * dt);
  
  // Anti-windup : limiter le terme intégral
  int maxIntegral = (int)(10000.0 / KI);
  if (integralError > maxIntegral) integralError = maxIntegral;
  if (integralError < -maxIntegral) integralError = -maxIntegral;
  
  float integral = KI * (integralError / 100.0);
  
  // Calculer la nouvelle puissance (0-10000, échelle x100)
  int newPowerHoldScaled = (int)((proportional + integral) * 100);
  
  // Limiter le taux de changement de puissance (sécurité)
  int powerChange = newPowerHoldScaled - lastPowerHold;
  if (powerChange > MAX_POWER_CHANGE * 100) {
    newPowerHoldScaled = lastPowerHold + (int)(MAX_POWER_CHANGE * 100);
  } else if (powerChange < -MAX_POWER_CHANGE * 100) {
    newPowerHoldScaled = lastPowerHold - (int)(MAX_POWER_CHANGE * 100);
  }
  
  // Contraindre la sortie (0-10000)
  if (newPowerHoldScaled > 10000) newPowerHoldScaled = 10000;
  if (newPowerHoldScaled < 0) newPowerHoldScaled = 0;
  
  // Mettre à jour les variables globales
  powerHold = newPowerHoldScaled / 100.0;
  lastPowerHold = newPowerHoldScaled;
  lastError = error;
}

void setRelay(bool state) {
  powerON = state;
  digitalWrite(PIN_RELAY, state ? HIGH : LOW);
  digitalWrite(PIN_LED, state ? HIGH : LOW);
}

int getPowerHold() {
  return (int)powerHold;
}

