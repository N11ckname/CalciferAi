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
float KP = 2.5;
float KI = 0.03;
// KD supprimé : non utilisé pour four céramique (inertie thermique élevée)

// PWM Parameters (variables modifiables)
unsigned int CYCLE_LENGTH = 1000;  // Cycle PWM de 1 seconde par défaut

// Global variables
float powerHold = 0;        // 0-100%
bool powerON = false;
unsigned long pwmCycleStart = 0;

// PID variables (optimized for RAM)
long integralError = 0;     // Scaled by 100 (long pour éviter overflow)
int lastError = 0;          // Scaled by 100
int lastPowerHold = 0;      // 0-10000 (scaled by 100)
unsigned long lastPIDUpdate = 0;

// PID Components (valeurs résultantes du dernier calcul)
float pidProportional = 0.0;
float pidIntegral = 0.0;
// pidDerivative supprimé : terme D non utilisé

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

// Fonction helper : réinitialise le cycle PWM si nécessaire
static void resetPWMCycleIfNeeded(unsigned long currentMillis) {
  unsigned long cycleElapsed = currentMillis - pwmCycleStart;
  if (cycleElapsed >= CYCLE_LENGTH) {
    pwmCycleStart = currentMillis;
  }
}

// Fonction interne : gestion du PWM logiciel (doit s'exécuter à chaque loop)
void updatePWM(unsigned long currentMillis) {
  // Cas spécial 100% : relais toujours ON
  if (lastPowerHold >= 10000) {
    setRelay(true);
    resetPWMCycleIfNeeded(currentMillis);
    return;
  }
  
  // Cas spécial 0% : relais toujours OFF
  if (lastPowerHold <= 0) {
    setRelay(false);
    resetPWMCycleIfNeeded(currentMillis);
    return;
  }
  
  // Cas normal : PWM entre 0% et 100%
  unsigned long cycleElapsed = currentMillis - pwmCycleStart;
  
  // Démarrer un nouveau cycle si nécessaire
  if (cycleElapsed >= CYCLE_LENGTH) {
    pwmCycleStart = currentMillis;
    cycleElapsed = 0;
  }
  
  // Calculer le temps ON pour ce cycle (lastPowerHold est scalé 0-10000 pour précision)
  // Exemple : lastPowerHold=5000 (50%), CYCLE_LENGTH=1000ms → onTime=500ms
  unsigned long onTime = ((unsigned long)lastPowerHold * (unsigned long)CYCLE_LENGTH) / 10000UL;
  
  // Contrôler le relais selon la position dans le cycle PWM
  if (cycleElapsed < onTime) {
    setRelay(true);
  } else {
    setRelay(false);
  }
}

void updateTemperatureControl(float currentTemp, float targetTemp, bool enabled, unsigned long currentMillis) {
  // Si le contrôle est désactivé : arrêt du chauffage et réinitialisation PID
  if (!enabled) {
    powerHold = 0;
    lastPowerHold = 0;
    setRelay(false);
    integralError = 0;
    lastError = 0;
    return;
  }
  
  // Le PWM s'exécute à chaque appel pour un contrôle précis du relais
  updatePWM(currentMillis);
  
  // Le calcul PID s'exécute à intervalle régulier (défini dans definitions.h)
  // L'inertie thermique élevée d'un four céramique ne nécessite pas un calcul plus fréquent
  if (currentMillis - lastPIDUpdate < PID_UPDATE_INTERVAL) {
    return;  // Attendre le prochain intervalle de calcul
  }
  
  // Calculer le delta de temps en secondes (limité pour assurer la stabilité)
  float dt = (currentMillis - lastPIDUpdate) / 1000.0;
  // Protection contre les valeurs aberrantes (pause, débordement, premier appel)
  // Si dt est trop grand ou trop petit, utiliser l'intervalle nominal
  if (dt < 0.5 || dt > 2.0) {
    dt = PID_UPDATE_INTERVAL / 1000.0;  // Utiliser la valeur nominale (1.0 seconde)
  }
  lastPIDUpdate = currentMillis;
  
  // Calculer l'erreur de température (scalée x100 pour optimisation)
  int error = (int)((targetTemp - currentTemp) * 100);
  
  // Calcul du terme proportionnel (P)
  pidProportional = KP * (error / 100.0);
  
  // Calcul du terme intégral (I) avec accumulation conditionnelle
  // Anti-windup conditionnel : ne pas accumuler si la puissance est déjà saturée
  // Cela évite l'accumulation inutile quand le four est déjà à 100% de puissance
  if (lastPowerHold < 10000) {  // Si puissance < 100%
    integralError += (int)(error * dt);
  }
  
  // Anti-windup : limiter l'accumulation du terme intégral pour éviter la saturation
  long maxIntegral = (long)(100000.0 / KI);  // Ajusté pour le nouveau scaling (x10)
  if (integralError > maxIntegral) integralError = maxIntegral;
  if (integralError < -maxIntegral) integralError = -maxIntegral;
  
  pidIntegral = KI * (integralError / 1000.0);  // Divisé par 10 pour réponse plus lente
  
  // Terme D (dérivé) supprimé : non nécessaire pour four céramique (inertie élevée)
  // Économie : ~8 bytes RAM + ~100 bytes Flash
  
  // Calculer la nouvelle puissance de sortie PID (PI seulement, pas D)
  int newPowerHoldScaled = (int)((pidProportional + pidIntegral) * 100);
  
  // Limiter le taux de changement de puissance (sécurité four)
  // Évite les variations brutales qui pourraient endommager les résistances
  int powerChange = newPowerHoldScaled - lastPowerHold;
  if (powerChange > MAX_POWER_CHANGE * 100) {
    newPowerHoldScaled = lastPowerHold + (int)(MAX_POWER_CHANGE * 100);
  } else if (powerChange < -MAX_POWER_CHANGE * 100) {
    newPowerHoldScaled = lastPowerHold - (int)(MAX_POWER_CHANGE * 100);
  }
  
  // Contraindre la sortie dans la plage valide (0-100%)
  if (newPowerHoldScaled > 10000) newPowerHoldScaled = 10000;
  if (newPowerHoldScaled < 0) newPowerHoldScaled = 0;
  
  // Mettre à jour les variables de sortie
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

// Getters pour les composantes PID (valeurs résultantes)
float getPIDProportional() {
  return pidProportional;
}

float getPIDIntegral() {
  return pidIntegral;
}

float getPIDError() {
  // Retourner l'erreur en degrés (lastError est scalé x100)
  return lastError / 100.0;
}

