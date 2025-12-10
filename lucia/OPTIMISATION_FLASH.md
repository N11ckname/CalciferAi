# Solutions pour Réduire la Taille du Programme LUCIA

## 📊 Diagnostic de la Situation

### Problème Actuel
Le programme LUCIA consomme trop de mémoire Flash et ne permet pas d'ajouter de nouvelles fonctionnalités.

### Mémoire Arduino Uno
- **Flash (Programme)** : 32 256 octets (31.5 Ko) disponibles
- **RAM (Variables)** : 2 048 octets disponibles
- **EEPROM** : 1 024 octets disponibles

---

## 🎯 Solutions par Ordre d'Impact

### ⭐ **SOLUTION 1 : Optimiser la Bibliothèque U8g2 (Économie : 3-8 Ko)**

**Impact : TRÈS ÉLEVÉ**

La bibliothèque U8g2 est la plus grosse consommatrice de Flash. Voici comment l'optimiser :

#### Option 1A : Utiliser le Mode Buffer Minimal (DÉJÀ FAIT ✅)

```cpp
// Déjà implémenté
U8G2_SH1106_128X64_NONAME_2_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
// Mode 2-page : économise ~768 octets de RAM
```

#### Option 1B : Réduire le Nombre de Fontes (-2 à 4 Ko)

**Actuellement utilisé :**
```cpp
u8g2_font_9x15_tf      // Grande fonte (température)
u8g2_font_6x10_tf      // Fonte moyenne (texte principal)
u8g2_font_5x7_tf       // Petite fonte (détails)
```

**Solution : N'utiliser que 2 fontes**
```cpp
// Dans display.cpp et lucia.ino
// Remplacer u8g2_font_9x15_tf par u8g2_font_6x10_tf partout
// Remplacer u8g2_font_5x7_tf par u8g2_font_6x10_tf

// Résultat : Une seule fonte = économie 2-4 Ko
```

**Implémentation :**
```cpp
// Avant (3 fontes)
u8g2.setFont(u8g2_font_9x15_tf);  // Grande
u8g2.setFont(u8g2_font_6x10_tf);  // Moyenne
u8g2.setFont(u8g2_font_5x7_tf);   // Petite

// Après (1 fonte uniquement)
u8g2.setFont(u8g2_font_6x10_tf);  // Partout
```

**Économie estimée : 2-4 Ko**

---

### ⭐ **SOLUTION 2 : Simplifier l'Affichage (-1 à 2 Ko)**

**Impact : ÉLEVÉ**

#### Option 2A : Supprimer l'Écran Settings

L'écran Settings est peu utilisé et consomme beaucoup de code.

**À supprimer :**
- Fonction `drawSettingsScreen()` (display.cpp)
- Fonction `drawSettingParam()` (display.cpp)
- Fonction `editSetting()` (lucia.ino)
- État `SETTINGS` (definitions.h)
- Navigation vers Settings

**Configuration alternative :**
```cpp
// Paramètres PID réglables uniquement par modification du code
// OU : interface simplifiée avec 1 seul paramètre à la fois
```

**Économie estimée : 800-1200 octets**

#### Option 2B : Simplifier drawProgOnScreen()

Réduire les informations affichées pendant la cuisson :

**Avant :**
```
Phase 1
570C/h->1100C,20m
Temp Read:125C
Temp Target:130C
Heat Power:45%
Phase:67%
```

**Après (simplifié) :**
```
Phase 1
Read:125C Tgt:130C
Power:45% Prog:67%
```

**Économie estimée : 300-500 octets**

---

### ⭐ **SOLUTION 3 : Utiliser PROGMEM pour les Chaînes (-500 à 1000 octets)**

**Impact : MOYEN-ÉLEVÉ**

Stocker les chaînes constantes en Flash au lieu de la RAM/Programme.

#### Problème avec U8g2
U8g2 ne supporte pas directement `F()` macro, mais on peut utiliser `u8g2.drawStr_P()`.

**Implémentation :**

```cpp
// Créer un fichier strings.h
#ifndef STRINGS_H
#define STRINGS_H

#include <avr/pgmspace.h>

// Toutes les chaînes en PROGMEM
const char STR_P1[] PROGMEM = "P1:";
const char STR_P2[] PROGMEM = "P2:";
const char STR_P3[] PROGMEM = "P3:";
const char STR_COOL[] PROGMEM = "Cool:";
const char STR_PHASE_1[] PROGMEM = "Phase 1";
const char STR_PHASE_2[] PROGMEM = "Phase 2";
const char STR_PHASE_3[] PROGMEM = "Phase 3";
const char STR_COOLDOWN[] PROGMEM = "Cool Down";
const char STR_WARN[] PROGMEM = "WARN";
const char STR_ERROR[] PROGMEM = "ERROR!";
const char STR_TEMP_FAIL[] PROGMEM = "Temp fail 2min";
const char STR_HEAT_STOPPED[] PROGMEM = "Heat stopped";
const char STR_CHECK_SENSOR[] PROGMEM = "Check sensor";
const char STR_SETTINGS[] PROGMEM = "SETTINGS";
const char STR_EXIT[] PROGMEM = "Exit";

#endif
```

**Utilisation :**

```cpp
// Au lieu de :
u8g2.drawStr(0, 24, "P1:");

// Utiliser :
char buf[4];
strcpy_P(buf, STR_P1);
u8g2.drawStr(0, 24, buf);

// OU avec buffer partagé :
strcpy_P(sharedBuffer, STR_P1);
u8g2.drawStr(0, 24, sharedBuffer);
```

**Économie estimée : 500-1000 octets**

---

### ⭐ **SOLUTION 4 : Réduire le Code des Commentaires (-200 à 500 octets)**

**Impact : FAIBLE-MOYEN**

Les commentaires ne sont pas compilés, MAIS les chaînes littérales dans le code le sont.

**À faire :**
- Supprimer les commentaires trop longs (ne prennent pas de place en Flash)
- Raccourcir les noms de variables longues
- Fusionner les fonctions similaires

**Économie estimée : 0 octets (commentaires) + 200-300 octets (optimisations code)**

---

### ⭐ **SOLUTION 5 : Remplacer la Bibliothèque Encoder (-1 à 2 Ko)**

**Impact : MOYEN**

La bibliothèque Encoder.h est relativement lourde. On peut la remplacer par un code simple.

**Code de remplacement :**

```cpp
// Supprimer : #include <Encoder.h>
// Supprimer : Encoder encoder(PIN_ENCODER_CLK, PIN_ENCODER_DT);

// Ajouter dans lucia.ino :
volatile int encoderPos = 0;
int lastEncoderCLK = HIGH;

void updateEncoder() {
  int clk = digitalRead(PIN_ENCODER_CLK);
  int dt = digitalRead(PIN_ENCODER_DT);
  
  if (clk != lastEncoderCLK && clk == LOW) {
    if (dt != clk) {
      encoderPos++;
    } else {
      encoderPos--;
    }
  }
  lastEncoderCLK = clk;
}

int readEncoder() {
  updateEncoder();
  return encoderPos / 4;  // Diviser par 4 comme la bibliothèque
}
```

**Dans handleEncoder() :**
```cpp
// Avant :
long newPosition = encoder.read() / 4;

// Après :
long newPosition = readEncoder();
```

**Économie estimée : 1000-2000 octets**

---

### ⭐ **SOLUTION 6 : Optimiser les Calculs Mathématiques (-100 à 300 octets)**

**Impact : FAIBLE**

Réduire l'utilisation des float et des opérations complexes.

#### Option 6A : Utiliser des Entiers pour les Températures

```cpp
// Au lieu de float (4 octets) :
float currentTemp = 125.5;

// Utiliser int (2 octets) :
int currentTemp = 1255;  // Température × 10
```

**Attention :** Change toute la logique de calcul.

#### Option 6B : Éviter les Divisions par des Constantes

```cpp
// Avant :
unsigned long tempIncrease = (unsigned long)speed * elapsed / 3600000UL;

// Après (précalculer) :
#define SPEED_DIVISOR 3600000UL
unsigned long tempIncrease = ((unsigned long)speed * elapsed) / SPEED_DIVISOR;
```

**Économie estimée : 100-300 octets**

---

### ⭐ **SOLUTION 7 : Fusionner les Fonctions Similaires (-200 à 500 octets)**

**Impact : FAIBLE-MOYEN**

Certaines fonctions font des choses très similaires.

#### Exemple : saveToEEPROM() et saveSettingsToEEPROM()

**Avant (2 fonctions) :**
```cpp
void saveToEEPROM() { /* 20 lignes */ }
void saveSettingsToEEPROM() { /* 20 lignes */ }
```

**Après (1 fonction) :**
```cpp
void saveToEEPROM(bool saveSettings) {
  // Protection temporelle commune
  // ...
  
  EEPROM.put(EEPROM_ADDR_MAGIC, EEPROM_MAGIC);
  EEPROM.put(EEPROM_ADDR_PARAMS, params);
  
  if (saveSettings) {
    EEPROM.put(EEPROM_ADDR_PARAMS + sizeof(FiringParams), settings);
  }
}
```

**Économie estimée : 200-500 octets**

---

### ⭐ **SOLUTION 8 : Désactiver les Fonctionnalités Rarement Utilisées (-500 à 1000 octets)**

**Impact : MOYEN**

#### Option 8A : Supprimer la Phase 3 (4 phases → 3 phases)

Si Phase 3 est peu utilisée :
- Supprimer `PHASE_3` de l'enum
- Supprimer les paramètres step3*
- Simplifier updateProgram()

**Économie estimée : 300-500 octets**

#### Option 8B : Supprimer le Temps de Palier (Wait)

Ne conserver que la montée en température, sans palier.

**Économie estimée : 200-400 octets**

#### Option 8C : Simplifier la Gestion d'Erreur

Supprimer le retry du MAX31856, juste afficher l'erreur.

**Économie estimée : 100-200 octets**

---

## 📋 Plan d'Action Recommandé

### 🥇 PHASE 1 : Gains Rapides (4-6 Ko) - PRIORITAIRE

1. ✅ **Réduire à 1 seule fonte** (-2 à 4 Ko)
2. ✅ **Utiliser PROGMEM pour les chaînes** (-500 à 1000 octets)
3. ✅ **Remplacer bibliothèque Encoder** (-1 à 2 Ko)

**Économie totale : 3500-7000 octets**

### 🥈 PHASE 2 : Gains Moyens (1-3 Ko) - SI NÉCESSAIRE

4. ✅ **Supprimer l'écran Settings** (-800 à 1200 octets)
5. ✅ **Simplifier drawProgOnScreen()** (-300 à 500 octets)
6. ✅ **Fusionner fonctions EEPROM** (-200 à 500 octets)

**Économie totale : 1300-2200 octets**

### 🥉 PHASE 3 : Gains Mineurs (500-1000 octets) - DERNIER RECOURS

7. ✅ **Optimiser calculs mathématiques** (-100 à 300 octets)
8. ✅ **Désactiver fonctionnalités peu utilisées** (-400 à 700 octets)

**Économie totale : 500-1000 octets**

---

## 🔧 Implémentation Pratique

### Option Conservative (Recommandée)

**Appliquer PHASE 1 uniquement :**
- Économie : 3.5 à 7 Ko
- Impact utilisateur : Minimal (une seule fonte)
- Difficulté : Moyenne (2-3 heures de travail)

### Option Agressive

**Appliquer PHASE 1 + PHASE 2 :**
- Économie : 4.8 à 9.2 Ko
- Impact utilisateur : Moyen (pas de settings UI)
- Difficulté : Élevée (4-6 heures de travail)

### Option Maximale

**Appliquer TOUTES les phases :**
- Économie : 5.3 à 10.2 Ko
- Impact utilisateur : Élevé (fonctionnalités réduites)
- Difficulté : Très élevée (6-10 heures de travail)

---

## 💡 Alternative : Passer à Arduino Mega 2560

### Avantages
- **Flash** : 256 Ko (8× plus)
- **RAM** : 8 Ko (4× plus)
- **EEPROM** : 4 Ko (4× plus)
- **Pins** : Plus nombreuses
- **Prix** : ~15-20€

### Inconvénients
- Carte plus grande
- Coût supplémentaire
- Nécessite adaptation du câblage (pins différentes)

### Verdict
✅ **Recommandé si budget disponible**

Le Mega résout TOUS les problèmes de mémoire sans compromis sur les fonctionnalités.

---

## 📊 Tableau Récapitulatif

| Solution | Économie Flash | Difficulté | Impact Utilisateur |
|----------|----------------|------------|-------------------|
| 1 fonte au lieu de 3 | 2-4 Ko | Facile | Faible |
| PROGMEM chaînes | 500-1000 o | Moyenne | Aucun |
| Remplacer Encoder | 1-2 Ko | Moyenne | Aucun |
| Supprimer Settings | 800-1200 o | Moyenne | Moyen |
| Simplifier affichage | 300-500 o | Facile | Faible |
| Fusionner fonctions | 200-500 o | Moyenne | Aucun |
| Optimiser maths | 100-300 o | Difficile | Aucun |
| Supprimer Phase 3 | 300-500 o | Moyenne | Élevé |
| **Arduino Mega** | **Illimité** | **Facile** | **Aucun** |

---

## ✅ Recommandation Finale

### Pour Production Immédiate
**Appliquer PHASE 1 :**
- Une seule fonte
- PROGMEM pour chaînes
- Remplacer Encoder

**Gain : 3.5 à 7 Ko**  
**Temps : 2-3 heures**  
**Impact : Minimal**

### Pour Solution Long Terme
**Investir dans Arduino Mega 2560**
- Résout tous les problèmes
- Permet futures extensions
- ROI excellent (temps vs coût)

---

**Voulez-vous que j'implémente l'une de ces solutions ?**
