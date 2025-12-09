/*
 * temperature.cpp - Gestion de la température et du contrôle PID
 */

#include <Arduino.h>
#include "definitions.h"
#include "temperature.h"

// Pin definitions
#define PIN_RELAY 6
#define PIN_LED A1

// PID Parameters (variables modifiables)
float KP = 2.0;
float KI = 0.5;
float KD = 0.0;

// PWM Parameters (variables modifiables)
unsigned int CYCLE_LENGTH = 1000;  // Cycle PWM de 1 seconde par défaut

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
  pwmCycleStart = millis(); // Initialiser le cycle PWM au démarrage
  // Initialiser dans le passé pour forcer le premier calcul PID immédiat
  lastPIDUpdate = millis() - PID_UPDATE_INTERVAL;
}

float readTemperature() {
  // Lire d'abord la température (la bibliothèque gère le timing SPI)
  float temp = max31856.readThermocoupleTemperature();
  
  // Vérifier si la lecture est valide
  if (isnan(temp)) {
    return NAN;
  }
  
  // Vérifier les limites raisonnables
  if (temp < -200 || temp > 2000) {
    return NAN;
  }
  
  return temp;
}

void resetPID() {
  integralError = 0;
  lastError = 0;
  lastPowerHold = 0;
  pwmCycleStart = millis(); // Réinitialiser le cycle PWM
  // Initialiser dans le passé pour forcer le premier calcul PID immédiat
  lastPIDUpdate = millis() - PID_UPDATE_INTERVAL;
}

// Fonction interne : gestion du PWM logiciel (doit s'exécuter à chaque loop)
void updatePWM(unsigned long currentMillis) {
  // Gérer le cas spécial 100% : relais toujours ON
  if (lastPowerHold >= 10000) {
    setRelay(true);
    // Réinitialiser le cycle périodiquement même à 100% pour éviter le dépassement
    unsigned long cycleElapsed = currentMillis - pwmCycleStart;
    if (cycleElapsed >= CYCLE_LENGTH) {
      pwmCycleStart = currentMillis;
    }
    return;
  }
  
  // Gérer le cas spécial 0% : relais toujours OFF
  if (lastPowerHold <= 0) {
    setRelay(false);
    // Réinitialiser le cycle périodiquement même à 0% pour éviter le dépassement
    unsigned long cycleElapsed = currentMillis - pwmCycleStart;
    if (cycleElapsed >= CYCLE_LENGTH) {
      pwmCycleStart = currentMillis;
    }
    return;
  }
  
  // Cas normal : PWM entre 0% et 100%
  // Calculer le temps écoulé dans le cycle actuel
  unsigned long cycleElapsed = currentMillis - pwmCycleStart;
  
  // Vérifier si on doit commencer un nouveau cycle
  if (cycleElapsed >= CYCLE_LENGTH) {
    // Début d'un nouveau cycle PWM
    pwmCycleStart = currentMillis;
    cycleElapsed = 0;
  }
  
  // Calculer le temps ON pour ce cycle (lastPowerHold est en 0-10000)
  // Exemple : lastPowerHold=5000 (50%), CYCLE_LENGTH=1000ms → onTime=500ms
  unsigned long onTime = ((unsigned long)lastPowerHold * (unsigned long)CYCLE_LENGTH) / 10000UL;
  
  // Activer/désactiver le relais selon la position dans le cycle
  // Si cycleElapsed est dans la période ON, allumer le relais
  if (cycleElapsed < onTime) {
    setRelay(true);
  } else {
    setRelay(false);
  }
}

void updateTemperatureControl(float currentTemp, float targetTemp, bool enabled, unsigned long currentMillis) {
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
  
  // Calculer le delta temps en secondes (limité pour éviter les sauts)
  float dt = (currentMillis - lastPIDUpdate) / 1000.0;
  if (dt > 2.0) dt = 1.0;  // Limiter dt pour le premier appel ou après une pause
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

