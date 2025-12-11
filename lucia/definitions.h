/*
 * definitions.h - Définitions communes pour tous les fichiers
 */

#ifndef DEFINITIONS_H
#define DEFINITIONS_H

// ===== PROGRAM STATES =====
enum ProgramState { PROG_OFF, PROG_ON, SETTINGS };
enum Phase { PHASE_0, PHASE_1, PHASE_2, PHASE_3, PHASE_4_COOLDOWN };
enum EditMode { NAV_MODE, EDIT_MODE };

// ===== TIMING CONSTANTS =====
#define TEMP_READ_INTERVAL 500           // Intervalle de lecture température (ms)
#define DISPLAY_UPDATE_INTERVAL 100      // Intervalle de mise à jour de l'écran (ms)
#define ENCODER_CHECK_INTERVAL 20        // Intervalle de vérification encodeur (ms) = 50Hz
#define TEMP_FAIL_TIMEOUT 120000         // Timeout erreur thermocouple (ms) = 2 minutes
#define EEPROM_WRITE_MIN_INTERVAL 5000   // Intervalle minimum entre écritures EEPROM (ms)

// ===== GRAPH CONSTANTS =====
#define GRAPH_SIZE 96                    // Nombre de points dans le buffer du graphe

// ===== FIRING PARAMETERS STRUCTURE =====
struct FiringParams {
  int step1Temp;
  int step1Speed;
  int step1Wait;
  int step2Temp;
  int step2Speed;
  int step2Wait;
  int step3Temp;
  int step3Speed;
  int step3Wait;
  int step4Speed;
  int step4Target;
};

// ===== SETTINGS PARAMETERS STRUCTURE =====
struct SettingsParams {
  int pcycle;      // Cycle PWM en millisecondes
  float kp;        // Gain proportionnel PID
  float ki;        // Gain intégral PID
  float kd;        // Gain dérivé PID
  int maxDelta;    // Erreur max (°C) pour passer à phase suivante
};

#endif

