# 🔍 TEST OLED SIMPLE

Programme ultra-simple pour diagnostiquer les problèmes d'affichage OLED.

## 🎯 Objectif

Tester uniquement l'écran OLED pour identifier le problème.

## 🚀 Utilisation

### Étape 1 : Upload
```
1. Fermer tous les programmes Arduino
2. Ouvrir TEST_OLED_SIMPLE/TEST_OLED_SIMPLE.ino
3. Upload sur Arduino Uno
```

### Étape 2 : Moniteur Série
```
Ouvrir le moniteur série à 9600 bauds
Lire les messages de diagnostic
```

### Étape 3 : Observer

**Sur le Moniteur Série, vous verrez :**

```
=================================
   TEST OLED - DIAGNOSTIC
=================================

1. Scan bus I2C...
Recherche peripheriques I2C...
Peripherique trouve a l'adresse 0x3C (OLED SSD1306/SH1106)
Nombre de peripheriques trouves: 1

2. Initialisation ecran...
OK: Ecran initialise

3. Contraste regle a 255
=================================
```

**Sur l'Écran OLED, vous devriez voir :**
- Flash blanc
- Texte "LUCIA"
- Texte "Ecran OK!"
- Compteur qui augmente

## 📊 Résultats Possibles

### ✅ Cas 1 : Adresse 0x3C Trouvée + Écran Affiche

**PARFAIT !** Votre écran fonctionne.

→ Retourner à TEST_MODE ou LUCIA

### ⚠️ Cas 2 : Adresse 0x3C Trouvée + Écran Noir

**Mauvais type d'écran configuré !**

**Solution :** Changer de configuration :

1. Dans `TEST_OLED_SIMPLE.ino` ligne 12, **commenter** :
   ```cpp
   // U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
   ```

2. Ligne 17, **décommenter** :
   ```cpp
   U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
   ```

3. Re-upload et tester

### ❌ Cas 3 : Aucune Adresse Trouvée

**Problème de câblage !**

```
AUCUN peripherique I2C trouve!

VERIFIER:
  - Connexion SDA (A4)
  - Connexion SCL (A5)
  - Alimentation VCC (5V)
  - Connexion GND
```

**Checklist :**
- [ ] VCC → 5V (vérifier avec multimètre)
- [ ] GND → GND
- [ ] SDA → A4 (PAS A5!)
- [ ] SCL → A5 (PAS A4!)
- [ ] Breadboard connecté à Arduino
- [ ] Écran allumé (parfois LED au dos)

## 🔧 Configurations à Tester

Si la configuration 1 ne marche pas :

### Configuration 2 : SSD1306
```cpp
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
```

### Configuration 3 : SH1106 Software I2C
```cpp
U8G2_SH1106_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, A5, A4, U8X8_PIN_NONE);
```

### Configuration 4 : SSD1306 Software I2C
```cpp
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, A5, A4, U8X8_PIN_NONE);
```

## 📝 Une Fois Fonctionnel

Quand l'écran affiche correctement :

1. **Noter quelle configuration fonctionne**
2. **Utiliser la même dans LUCIA** :
   
   Modifier dans `LUCIA/lucia.ino` ligne 20 :
   ```cpp
   // Remplacer par la configuration qui marche
   U8G2_SH1106_128X64_NONAME_2_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
   ```
   
   Changer `_F_` en `_2_` pour économiser RAM, mais garder le même type (SH1106 ou SSD1306).

## 🆘 Si Rien Ne Marche

1. Vérifier les fils (les changer si possible)
2. Tester avec un autre écran OLED
3. Vérifier spécifications écran (3.3V vs 5V)
4. Consulter `TEST_MODE/DEPANNAGE_ECRAN.md`

---

**Ce programme est conçu pour être le plus simple possible.**  
**Si ça ne marche pas ici, c'est un problème matériel, pas logiciel.**

