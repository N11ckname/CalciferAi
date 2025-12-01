# TEST_MODE - Programme de Test LUCIA

Programme de diagnostic pour tester tous les composants du contrôleur de four céramique LUCIA.

## 🎯 Objectif

Vérifier que tous les composants sont correctement câblés et fonctionnels avant d'utiliser le programme principal.

## 📦 Composants Testés

1. **Écran OLED SH1106** - Affichage
2. **Encodeur Rotatif** - Rotation et clic
3. **Boutons** - Encodeur SW + Push Button
4. **MAX31856** - Lecture température thermocouple
5. **Relais** - Contrôle sortie
6. **LED** - Indicateur visuel

## 🔌 Câblage Requis

### Écran OLED SH1106
- VCC → 5V
- GND → GND
- SCL → A5
- SDA → A4

### MAX31856
- VIN → 5V
- GND → GND
- SCK → D13
- SDO → D12
- SDI → D11
- CS → D10

### Encodeur Rotatif
- GND → GND
- VCC → 5V
- CLK → D2
- DT → D3
- SW → D4

### Bouton Poussoir
- Une broche → D5
- Autre broche → GND

### Relais Statique
- Positif → D6
- Négatif → GND

### LED de Contrôle
- Anode (+) → A1
- Cathode (-) → GND

## 🚀 Utilisation

### 1. Upload du Programme

1. Ouvrir `TEST_MODE.ino` dans l'IDE Arduino
2. Sélectionner **Arduino Uno**
3. Uploader le programme

### 2. Navigation entre Tests

**Encodeur Clic :** Passer au test suivant (6 tests au total)

**Bouton Poussoir :** Toggle Relais/LED ON/OFF

### 3. Description des Tests

#### Test 1/6 : ÉCRAN OLED
- Affiche du texte
- **Résultat attendu :** Texte lisible et net
- **Si OK :** Écran fonctionne correctement

#### Test 2/6 : ENCODEUR
- Affiche la position de l'encodeur
- **Action :** Tournez l'encodeur dans les deux sens
- **Résultat attendu :** La position change (+ ou -)
- **Si OK :** Encodeur fonctionne

#### Test 3/6 : BOUTONS
- Affiche l'état des boutons
- **Action :** Appuyez sur chaque bouton
- **Résultat attendu :** État change de OFF à ON
- **Si OK :** Boutons fonctionnent

#### Test 4/6 : THERMOCOUPLE
- Affiche la température mesurée
- Affiche la température de jonction froide (CJ)
- **Résultat attendu :** 
  - Température proche de l'ambiante (~20-30°C)
  - CJ proche de la température Arduino
  - Pas de code d'erreur
- **Si erreur :** Vérifier connexion thermocouple

#### Test 5/6 : RELAIS & LED
- Contrôle du relais et de la LED
- **Action :** Appuyez sur bouton poussoir
- **Résultat attendu :** 
  - LED s'allume/s'éteint
  - Relais clique (si audible)
  - État affiché change
- **Si OK :** Relais et LED fonctionnent

#### Test 6/6 : RÉSUMÉ COMPLET
- Affiche toutes les valeurs en temps réel
- **Résultat attendu :** Toutes les valeurs sont cohérentes
- **Si OK :** Système complet opérationnel ✅

## 📊 Moniteur Série

Ouvrir le moniteur série (9600 bauds) pour voir les messages de debug :

```
MAX31856 OK
TEST MODE READY
Encoder click: next test
Push button: toggle relay/LED
Test mode: 0
Test mode: 1
Relay/LED: ON
Relay/LED: OFF
```

## ⚠️ Dépannage

### L'écran ne s'allume pas
- Vérifier alimentation 5V et GND
- Vérifier connexions I2C (A4, A5)
- Vérifier contraste de l'écran

### Encodeur ne répond pas
- Vérifier connexions D2 et D3
- Vérifier GND et VCC
- Essayer d'inverser CLK et DT

### MAX31856 ERROR
- Vérifier connexion CS (D10)
- Vérifier connexions SPI (D11, D12, D13)
- Vérifier alimentation

### Température incorrecte
- Vérifier polarité thermocouple (+ et -)
- Vérifier type de thermocouple (Type S)
- Tester avec un autre thermocouple

### Relais ne clique pas
- Vérifier connexion D6
- Vérifier GND du relais
- Mesurer tension sur D6 (devrait être 5V quand ON)

### LED ne s'allume pas
- Vérifier polarité (anode sur A1, cathode sur GND)
- Essayer une autre LED
- Vérifier si LED grillée

## ✅ Validation Finale

Si tous les tests passent :
1. Tous les composants sont bien câblés ✅
2. Toutes les bibliothèques sont correctement installées ✅
3. Le système est prêt pour le programme LUCIA ✅

Vous pouvez maintenant uploader le programme principal `LUCIA/lucia.ino` en toute confiance !

## 📝 Notes

- Utilisation RAM : ~1200 octets (58%)
- Utilisation Flash : ~22000 octets (68%)
- Compatible Arduino Uno

## 🔗 Bibliothèques Requises

Les mêmes que le programme principal :
- **U8g2** (par oliver)
- **Adafruit MAX31856** (par Adafruit)
- **Encoder** (par Paul Stoffregen)

