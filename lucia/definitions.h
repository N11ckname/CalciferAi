/*
 * definitions.h - Définitions communes pour tous les fichiers
 */

#ifndef DEFINITIONS_H
#define DEFINITIONS_H

// ===== PROGRAM STATES =====
enum ProgramState { PROG_OFF, PROG_ON, SETTINGS };
enum Phase { PHASE_0, PHASE_1, PHASE_2, PHASE_3, PHASE_4_COOLDOWN };
enum EditMode { NAV_MODE, EDIT_MODE };

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
  float kp;        // Gain proportionnel PID (x100 pour stockage en int)
  float ki;        // Gain intégral PID (x100 pour stockage en int)
  float kd;        // Gain dérivé PID (x100 pour stockage en int)
};

#endif

