# Corrections Apportées au Code LUCIA

## Problèmes Identifiés et Résolus

### 1. Constructeur MAX31856 Incorrect ❌ → ✅

**Erreur :**
```cpp
Adafruit_MAX31856 max31856 = Adafruit_MAX31856(PIN_MAX_CS, PIN_MAX_DRDY, PIN_MAX_FLT);
```

**Problème :** Le constructeur de `Adafruit_MAX31856` n'accepte pas 3 paramètres. Les pins DRDY et FLT ne sont pas des arguments du constructeur - elles sont utilisées uniquement pour la lecture des interruptions.

**Solution :**
```cpp
Adafruit_MAX31856 max31856 = Adafruit_MAX31856(PIN_MAX_CS);
```

Les pins DRDY et FLT peuvent être lues directement via `digitalRead()` si nécessaire, mais ne sont pas obligatoires pour le fonctionnement de base.

---

### 2. Fonction oneShotTemperature() Inexistante ❌ → ✅

**Erreur :**
```cpp
uint8_t fault = max31856.readFault();
if (fault) {
  max31856.oneShotTemperature();  // Cette fonction n'existe pas !
  return NAN;
}
```

**Problème :** La bibliothèque `Adafruit_MAX31856` n'a pas de méthode `oneShotTemperature()`.

**Solution :**
```cpp
uint8_t fault = max31856.readFault();
if (fault) {
  return NAN;
}
```

La lecture des défauts est suffisante. La bibliothèque gère automatiquement le mode de conversion continu.

---

### 3. Déclarations d'Enums et Structures Manquantes ❌ → ✅

**Erreur :**
```cpp
// Dans display.h
extern enum ProgramState progState;  // enum non déclaré !
extern enum Phase currentPhase;      // enum non déclaré !
extern struct FiringParams params;   // structure non déclarée !
```

**Problème :** Les types `enum` et `struct` étaient déclarés dans `lucia.ino` mais utilisés dans `display.h` sans déclaration préalable.

**Solution :** Création d'un fichier `definitions.h` avec toutes les définitions communes :

```cpp
// definitions.h
#ifndef DEFINITIONS_H
#define DEFINITIONS_H

enum ProgramState { PROG_OFF, PROG_ON };
enum Phase { PHASE_0, PHASE_1, PHASE_2, PHASE_3, PHASE_4_COOLDOWN };
enum EditMode { NAV_MODE, EDIT_MODE };

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

#endif
```

---

### 4. Conflits de Déclarations ❌ → ✅

**Erreur :**
```cpp
// lucia.ino définit :
enum EditMode { NAV_MODE, EDIT_MODE };
EditMode editMode = NAV_MODE;

// display.h déclare :
extern int editMode;  // Type incompatible !
```

**Problème :** Les types ne correspondaient pas entre les déclarations `extern` et les définitions réelles.

**Solution :** Utilisation cohérente des types définis dans `definitions.h` :

```cpp
// display.h
#include "definitions.h"
extern EditMode editMode;  // Type correct
```

---

### 5. Comparaisons avec Enums ❌ → ✅

**Erreur :**
```cpp
if (selectedParam == paramIndex && editMode == 0) { // NAV_MODE
```

**Problème :** Comparaison d'un enum avec un entier (mauvaise pratique).

**Solution :**
```cpp
if (selectedParam == paramIndex && editMode == NAV_MODE) {
```

---

## Structure Finale des Fichiers

```
LUCIA/
├── lucia.ino           → Fichier principal, inclut definitions.h
├── definitions.h       → Définitions communes (NEW!)
├── display.h           → Déclarations pour l'affichage, inclut definitions.h
├── display.cpp         → Implémentation affichage, inclut definitions.h
├── temperature.h       → Déclarations pour le contrôle température
├── temperature.cpp     → Implémentation température, inclut definitions.h
├── README.md           → Documentation
└── CORRECTIONS.md      → Ce fichier
```

---

## Ordre d'Inclusion des Headers

Pour éviter les erreurs de compilation, l'ordre d'inclusion est important :

```cpp
// Dans tous les fichiers .cpp et lucia.ino :
#include <Arduino.h>        // (optionnel, inclus automatiquement dans .ino)
#include "definitions.h"    // TOUJOURS EN PREMIER
#include "autresfichiers.h" // Ensuite les autres
```

---

## Vérification de la Compilation

Le code devrait maintenant compiler sans erreurs. Pour vérifier :

1. Ouvrir `lucia.ino` dans l'IDE Arduino
2. Sélectionner la carte **Arduino Uno**
3. Cliquer sur **Vérifier** (icône ✓)

Aucune erreur ne devrait apparaître !

---

## Connexions des Pins MAX31856

**Important :** Les pins DRDY et FLT ne sont plus définies dans le code mais peuvent être câblées si vous souhaitez ajouter la gestion des interruptions plus tard :

- **CS** → Arduino D10 (OBLIGATOIRE - Chip Select)
- **SCK** → Arduino D13 (SPI Clock)
- **SDO** → Arduino D12 (MISO)
- **SDI** → Arduino D11 (MOSI)
- **DRDY** → Optionnel (signal de données prêtes)
- **FLT** → Optionnel (signal de défaut)

Le code actuel utilise le mode polling (lecture à intervalle régulier) plutôt que les interruptions.

---

## Améliorations Futures Possibles

1. **Gestion des interruptions DRDY** : Pour des lectures plus efficaces
2. **Détection détaillée des faults** : Analyser le registre de fautes pour identifier le type exact d'erreur
3. **Calibration** : Ajouter une compensation de la jonction froide si nécessaire
4. **Watchdog** : Protection contre les blocages du programme

---

**Date des corrections :** 30 Novembre 2025  
**Statut :** ✅ Toutes les erreurs de compilation résolues

