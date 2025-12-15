/*
 * temperature.h - Gestion de la température et du contrôle PID
 */

#ifndef TEMPERATURE_H
#define TEMPERATURE_H

#include <Adafruit_MAX31856.h>

// External references
extern Adafruit_MAX31856 max31856;

// PID Parameters (variables externes modifiables via l'interface Settings)
extern float KP;  // Gain proportionnel : réaction immédiate à l'erreur
extern float KI;  // Gain intégral : correction de l'erreur résiduelle
// KD supprimé : non utilisé pour four céramique (inertie thermique élevée)
#define MAX_POWER_CHANGE 10.0  // Limitation sécurité : max 10% de changement par cycle

// PWM Parameters (contrôle de la fréquence de chauffage)
extern unsigned int CYCLE_LENGTH; // Cycle PWM en millisecondes (modifiable)
#define PID_UPDATE_INTERVAL 1000  // Fréquence de calcul PID : 1 seconde (1 Hz)
                                   // Adapté à l'inertie thermique d'un four céramique

// Function declarations
void initTemperatureControl();
float readTemperature();
float getCurrentTemperature(); // Retourne la température mise en cache (lue toutes les 500ms)
void updateTemperatureControl(float currentTemp, float targetTemp, bool enabled, unsigned long currentMillis);
void setRelay(bool state);
int getPowerHold();
void resetPID();

// PID Components Getters (valeurs résultantes du calcul PID)
float getPIDProportional();  // Retourne la valeur du terme P
float getPIDIntegral();      // Retourne la valeur du terme I
// getPIDDerivative() supprimé : terme D non utilisé
float getPIDError();         // Retourne l'erreur actuelle (Target - Sonde) en degrés

#endif

