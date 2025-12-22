# Guide de Construction CalciferAi

## 1. Liste de Courses (~106€)

### Composants Principaux
| Composant | Prix |
|-----------|------|
| Arduino Uno | 5€ |
| Écran OLED SH1106 128x64 (I2C) | 3€ |
| Adafruit MAX31856 | 7€ |
| Encodeur rotatif KY-040 | 3€ |
| Bouton poussoir | 0,50€ |
| LED de contrôle 5mm | 2€ |
| Relais SSR-25DA | 7€ |
| Breadboard 170 points | 1€ |
| Transistor 2N2222A | 0,30€ |
| Résistances 1kΩ (x10) | 0,50€ |
| Résistances 10kΩ (x10) | 0,50€ |
| Fils Dupont (kit 40 pcs) | 3€ |
| Alimentation 9V pour l'Arduino | 2€ |
| Boîtier aluminium | 18€ |


### Thermocouple
| Composant | Prix |
|-----------|------|
| Thermocouple Type S | 20€ |
| Fil thermocouple | 5€ |


---

## 2. Câblage

### Alimentation (Breadboard)
- Arduino **5V** → Rail + breadboard
- Arduino **GND** → Rail - breadboard

### Écran OLED
| Écran | Arduino |
|-------|---------|
| VCC | 5V (rail +) |
| GND | GND (rail -) |
| SCL | A5 |
| SDA | A4 |

### MAX31856 (Thermocouple)
| MAX31856 | Arduino |
|----------|---------|
| VIN | 5V (rail +) |
| GND | GND (rail -) |
| SCK | D13 |
| SDO | D12 |
| SDI | D11 |
| CS | D10 |
| DRDY | D9 |
| FLT | D8 |
| T+ | Thermocouple + |
| T- | Thermocouple - |

### Encodeur Rotatif
| Encodeur | Arduino |
|----------|---------|
| VCC | 5V (rail +) |
| GND | GND (rail -) |
| CLK | D2 |
| DT | D3 |
| SW | D4 |

### Bouton Poussoir
- Une patte → **D5**
- Autre patte → **GND** (rail -)

### LED de Contrôle
- Anode (+) → **A1**
- Cathode (-) → **GND** (rail -)

### Relais SSR (avec protection transistor 2N2222A)
```
+5V Arduino
     │
     ▼
SSR (+) input
     │
SSR (-) input
     │
     ▼
Collecteur (C)
     │
     │
   Base (B)
   Transistor NPN  ─── [R1 1kΩ]── D6 Arduino
     │              │
     │       [R_pulldown 10kΩ]
     │              │
Émetteur (E)        │
     │              │
     └──────────────┘
     │
     ▼
   GND Arduino
```

**⚠️ Pinout 2N2222A** : E-B-C (Émetteur-Base-Collecteur) - Vérifier la datasheet !

---

## 3. Téléverser le Programme

### Étape 0 : Choisir les Fonctionnalités (optionnel)
Dans `lucia/definitions.h`, activer/désactiver en ajoutant ou retirant `//` :
```
#define ENABLE_LOGGING  // Logging série (~250 octets)
#define ENABLE_GRAPH    // Graphe température (~800 octets)
```
⚠️ **La mémoire de l'Arduino est limitée** : activez une seule option à la fois si le programme ne compile pas.

### Étape 1 : Installer Arduino IDE
1. Télécharger sur [arduino.cc/software](https://www.arduino.cc/software)
2. Installer et lancer

### Étape 2 : Installer les Bibliothèques
Dans Arduino IDE : **Outils → Gérer les bibliothèques**

Chercher et installer :
- `U8g2` (pour l'écran OLED)
- `Adafruit MAX31856` (pour le thermocouple)
- `Encoder` (pour l'encodeur rotatif)

### Étape 3 : Configurer la Carte
- **Outils → Type de carte** → Arduino Uno
- **Outils → Port** → Sélectionner le port de l'Arduino (ex: COM3 ou /dev/ttyUSB0)

### Étape 4 : Ouvrir et Téléverser
1. **Fichier → Ouvrir** → Naviguer vers `lucia/lucia.ino`
2. Cliquer sur **→** (Téléverser) ou Ctrl+U
3. Attendre "Téléversement terminé"

---

**C'est prêt !** L'écran devrait s'allumer et afficher le menu principal.

