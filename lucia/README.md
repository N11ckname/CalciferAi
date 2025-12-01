# LUCIA - Contrôleur de Four Céramique

Système de thermostat intelligent pour four électrique de céramique avec contrôle PID et interface utilisateur OLED.

## Fichiers du Projet

- **lucia.ino** - Fichier principal Arduino (initialisation et boucle principale)
- **definitions.h** - Définitions communes (enums, structures)
- **display.h/cpp** - Gestion de l'affichage OLED SH1106
- **temperature.h/cpp** - Gestion de la température et contrôle PID/PWM
- **README.md** - Ce fichier

## Bibliothèques Requises

Installez les bibliothèques suivantes via le Gestionnaire de bibliothèques Arduino :

1. **U8g2** (par oliver) - pour l'écran OLED SH1106
2. **Adafruit MAX31856** (par Adafruit) - pour le module thermocouple
3. **Encoder** (par Paul Stoffregen) - pour l'encodeur rotatif

## Matériel Requis

- Arduino Uno
- Écran OLED SH1106 128x64 pixels (I2C, 4 fils)
- Adafruit Universal Thermocouple Amplifier MAX31856
- Sonde de température Type S
- Encodeur rotatif cliquable (5 fils)
- Bouton poussoir
- LED de contrôle
- Relais statique (entrée 2 fils)
- Mini breadboard pour distribution d'alimentation

## Connexions

### Écran OLED SH1106 (I2C)
- VCC → Rail 5V du breadboard
- GND → Rail GND du breadboard
- SCL → Arduino A5
- SDA → Arduino A4

### Amplificateur Thermocouple MAX31856
- VIN → Rail 5V du breadboard
- GND → Rail GND du breadboard
- SCK → Arduino D13
- SDO → Arduino D12
- SDI → Arduino D11
- CS → Arduino D10
- DRDY → Arduino D9
- FLT → Arduino D8

### Thermocouple Type S
- Fil positif (+) → Terminal T+ du MAX31856
- Fil négatif (-) → Terminal T- du MAX31856

### Encodeur Rotatif Cliquable
- GND → Rail GND du breadboard
- VCC → Rail 5V du breadboard
- CLK (ou A) → Arduino D2
- DT (ou B) → Arduino D3
- SW (bouton) → Arduino D4

### Bouton Poussoir
- Une broche → Arduino D5
- Autre broche → Rail GND

### LED de Contrôle
- Anode (+) → Arduino A1
- Cathode (-) → Rail GND du breadboard

### Relais Statique
- Terminal positif → Arduino D6
- Terminal négatif → Rail GND du breadboard

### Alimentation du Breadboard
- Arduino 5V → Rail positif du breadboard
- Arduino GND → Rail négatif du breadboard

## Utilisation

### Mode PROG_OFF (Four arrêté)

1. **Navigation** : Tournez l'encodeur pour sélectionner un paramètre (surligné en inverse)
2. **Édition** : Cliquez sur l'encodeur pour entrer en mode édition (contour fin)
3. **Modification** : Tournez l'encodeur pour changer la valeur
4. **Validation** : Cliquez à nouveau pour sauvegarder (écriture automatique en EEPROM)
5. **Démarrage** : Appuyez sur le bouton poussoir pour démarrer le programme

### Mode PROG_ON (Programme en cours)

- Le programme suit automatiquement les 4 phases de cuisson
- L'écran affiche la phase en cours (texte blanc) et grise les phases complétées/futures
- Le temps écoulé et le temps restant sont affichés
- **Arrêt d'urgence** : Appuyez sur le bouton poussoir
- **Graphique** : Appui long (>1s) sur l'encodeur pour voir le graphique température vs temps

### Phases du Programme

1. **Phase 1** : Montée à Step1Temp avec Step1Speed, puis plateau de Step1Wait minutes
2. **Phase 2** : Montée à Step2Temp avec Step2Speed, puis plateau de Step2Wait minutes
3. **Phase 3** : Montée à Step3Temp avec Step3Speed, puis plateau de Step3Wait minutes
4. **Phase 4 (Refroidissement)** : Descente à Step4Target avec Step4Speed

## Paramètres PID

- **Kp** (Gain proportionnel) : 2.0
- **Ki** (Gain intégral) : 0.5
- **Kd** (Gain dérivé) : 0.0
- **Limitation de changement** : 10% maximum par cycle PWM
- **Cycle PWM** : 1000ms (1 seconde)

## Sécurité

- Le relais est automatiquement éteint en mode PROG_OFF
- En cas d'échec de lecture de température pendant 2 minutes, le chauffage s'arrête automatiquement
- Message d'erreur critique affiché avec arrêt du chauffage
- Le bouton poussoir permet un arrêt d'urgence à tout moment

## Sauvegarde des Paramètres

Tous les paramètres de cuisson sont automatiquement sauvegardés en EEPROM lors de la sortie du mode édition. Les valeurs sont restaurées au démarrage.

## Taille du Programme

Le code est optimisé pour rester en dessous de 31 232 octets de mémoire programme.

## Notes Techniques

- Utilisation de `millis()` pour tous les timings (pas de `delay()`)
- Contrôle PID avec anti-windup
- PWM logiciel pour contrôle proportionnel du chauffage
- Gestion des rebonds des boutons (debouncing)
- Affichage optimisé à 500ms de rafraîchissement

## Auteur

Système LUCIA pour contrôle de four céramique

