# 🔧 Dépannage Écran OLED - Pas d'Affichage

## 🚨 Problème : Écran Noir / Pas d'Affichage

### Étape 1 : Test Ultra-Simple

1. **Uploader le programme de diagnostic :**
   ```
   Ouvrir : Test_OLED_Simple.ino
   Upload sur Arduino Uno
   ```

2. **Ouvrir le Moniteur Série (9600 bauds)**
   - Regarder les messages de diagnostic
   - Noter l'adresse I2C détectée

### Étape 2 : Vérifier le Câblage

#### ✅ Checklist Connexions OLED

```
OLED Pin    →    Arduino Pin
────────────────────────────
VCC (ou VDD) →   5V  ✓ _____
GND          →   GND ✓ _____
SCL (ou SCK) →   A5  ✓ _____
SDA          →   A4  ✓ _____
```

**⚠️ Points Importants :**
- Certains écrans ont VCC et GND inversés ! Vérifier l'ordre des pins
- Écrans 3.3V peuvent fonctionner sur 5V (vérifier spécifications)
- Fils doivent être bien enfoncés

#### 🔍 Test Physique

1. **Alimentation :**
   - Écran doit être alimenté (parfois une LED s'allume au dos)
   - Mesurer tension entre VCC et GND (doit être ~5V)

2. **Connexions I2C :**
   - Vérifier continuité avec multimètre
   - Pas de faux contacts sur breadboard

### Étape 3 : Identifier le Type d'Écran

#### Votre écran est probablement :

**A) SH1106 (le plus courant pour 128x64)**
```cpp
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
```

**B) SSD1306 (alternative courante)**
```cpp
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
```

**Comment savoir ?**
- Regarder les marquages sur l'écran
- Essayer les deux configurations
- Consulter vendeur/documentation

### Étape 4 : Résultats du Scan I2C

#### Cas 1 : Adresse 0x3C Détectée ✅

**C'est bon signe !** L'écran est connecté mais peut-être mal configuré.

**Solutions :**

1. **Essayer SSD1306 au lieu de SH1106**
   
   Dans `Test_OLED_Simple.ino`, commenter ligne actuelle et décommenter :
   ```cpp
   // U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
   ```

2. **Forcer le mode Software I2C**
   ```cpp
   // U8G2_SH1106_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, A5, A4, U8X8_PIN_NONE);
   ```

#### Cas 2 : Adresse 0x3D Détectée

Certains écrans utilisent 0x3D. Essayer :
```cpp
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, 0x3D);
```

#### Cas 3 : Aucune Adresse Détectée ❌

**Problème de câblage !**

**Vérifier :**
1. ✓ VCC sur 5V (pas sur 3.3V)
2. ✓ GND sur GND
3. ✓ SDA sur A4 (PAS A5 !)
4. ✓ SCL sur A5 (PAS A4 !)
5. ✓ Breadboard fonctionne (tester avec LED)
6. ✓ Câbles ne sont pas coupés

**Test simple :**
```cpp
// Dans setup()
pinMode(A4, OUTPUT);
digitalWrite(A4, HIGH);
delay(500);
digitalWrite(A4, LOW);
```
→ Si aucun effet, problème de câble ou pin

### Étape 5 : Problèmes Courants

#### 🔴 Écran Reste Noir Mais I2C OK

**Causes possibles :**

1. **Contraste trop faible**
   ```cpp
   u8g2.setContrast(255); // Maximum
   ```

2. **Mauvais type d'écran**
   - Essayer SSD1306 si SH1106 ne marche pas
   - Essayer 128x32 au lieu de 128x64

3. **Écran défectueux**
   - Tester avec un autre écran si possible

#### 🔴 Écran Blanc Complet

**Succès partiel !** Communication OK mais affichage inversé.

**Solution :**
```cpp
u8g2.setFlipMode(1); // Inverser affichage
```

#### 🔴 Texte Inversé ou Miroir

```cpp
u8g2.setFlipMode(0); // ou 1, 2, 3
```

### Étape 6 : Tests Alternatifs

#### Test 1 : Bibliothèque Adafruit

Si U8g2 ne marche pas, essayer Adafruit :

```cpp
#include <Adafruit_SSD1306.h>

Adafruit_SSD1306 display(128, 64, &Wire, -1);

void setup() {
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("Erreur SSD1306");
    while(1);
  }
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("LUCIA");
  display.display();
}
```

#### Test 2 : Scanner I2C Standalone

```cpp
#include <Wire.h>

void setup() {
  Wire.begin();
  Serial.begin(9600);
  Serial.println("I2C Scanner");
}

void loop() {
  for(byte addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if(Wire.endTransmission() == 0) {
      Serial.print("0x");
      Serial.println(addr, HEX);
    }
  }
  delay(5000);
}
```

### 🎯 Checklist Finale

Avant de continuer, vérifier :

- [ ] Câblage correct (VCC, GND, SDA, SCL)
- [ ] Scan I2C détecte adresse 0x3C ou 0x3D
- [ ] Bon type d'écran (SH1106 ou SSD1306)
- [ ] Contraste au maximum (255)
- [ ] Test_OLED_Simple.ino fonctionne

### 📞 Si Rien Ne Marche

1. **Photographier votre montage** - Vérifier visuellement
2. **Tester avec un autre écran** - Éliminer écran défectueux
3. **Tester écran sur autre Arduino** - Éliminer Arduino défectueux
4. **Vérifier spécifications écran** - Tension, type, résolution

### ✅ Une Fois l'Écran Fonctionnel

Retourner à `TEST_MODE.ino` ou `LUCIA` avec la bonne configuration :

```cpp
// Utiliser le type qui a fonctionné dans Test_OLED_Simple.ino
U8G2_SH1106_128X64_NONAME_2_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
// ou
U8G2_SSD1306_128X64_NONAME_2_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
```

---

**90% des problèmes d'écran OLED sont dus au câblage !**  
**Prenez le temps de vérifier chaque connexion méthodiquement.** 🔍

