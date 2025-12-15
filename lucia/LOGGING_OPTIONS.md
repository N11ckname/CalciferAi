# Options de Logging - CalciferAi

## 📊 État Actuel

**Par défaut : Logging DÉSACTIVÉ**
- Taille programme : ~31500 octets (98%)
- RAM utilisée : 1793 octets (87%)
- ✅ Compile sur Arduino Uno

## 🔧 Activation du Logging

### Option 1 : Logging Ultra-Minimal (RECOMMANDÉ)

**Configuration actuelle** - Ajoute ~250 octets

Dans `definitions.h`, décommentez :
```cpp
#define ENABLE_LOGGING
```

**Format de sortie :**
```
Démarrage:
S,2.0,0.5,0.0

Données (toutes les 5s):
D,5000,125.3,130.0,10.5,2.3,75
D,10000,132.1,135.0,8.2,3.1,80
```

**Légende :**
- S = Start (Kp, Ki, Kd)
- D = Data (temps_ms, temp_réelle, temp_cible, P, I, puissance%)

**Taille finale : ~31750 octets (98.4%)** ✅ Devrait compiler

---

### Option 2 : Logging Complet (DÉSACTIVÉ)

Si vous avez besoin d'un logging plus détaillé avec tous les paramètres des phases, il faudra :

1. **Optimisations supplémentaires nécessaires** (~200 octets)
2. Ou utiliser un **Arduino Mega** (32 Ko Flash)

**Format complet :**
```
START,KP:2.0,KI:0.5,KD:0.0,P1:570/250/15,P2:1100/200/20,P3:1100/200/20,COOL:150/200
DATA,5000,125.3,130.0,10.5,2.3,0.0,75
```

**Taille : ~32200 octets (99.8%)** ⚠️ Risque de dépassement

---

## 💾 Économie de Mémoire

### Si le Logging est Désactivé

**Économie totale : ~600 octets**
- Code Serial.print() non compilé
- Buffers non alloués
- Fonctions de formatage éliminées

### Pour Désactiver le Logging

Dans `definitions.h`, commentez :
```cpp
//#define ENABLE_LOGGING  // Désactivé
```

---

## 🎯 Recommandation

### Pour Arduino Uno (32 Ko)
✅ **Logging Ultra-Minimal activé** (Option 1)
- Suffisant pour monitoring et debug
- Compile sans problème
- Données essentielles préservées

### Pour Arduino Mega (256 Ko)
✅ **Logging Complet activé**
- Toutes les données disponibles
- Aucune contrainte de mémoire
- Ajouter `#define LOGGING_VERBOSE` pour version détaillée

---

## 📈 Utilisation des Données

### Script Python pour Lecture (Exemple)

```python
import serial

ser = serial.Serial('/dev/ttyUSB0', 9600)

while True:
    line = ser.readline().decode('utf-8').strip()
    
    if line.startswith('S,'):
        # Startup: S,Kp,Ki,Kd
        _, kp, ki, kd = line.split(',')
        print(f"PID: Kp={kp}, Ki={ki}, Kd={kd}")
    
    elif line.startswith('D,'):
        # Data: D,time,temp,target,P,I,power
        _, t, temp, target, p, i, power = line.split(',')
        print(f"t={t}ms T={temp}°C → {target}°C P={p} I={i} Power={power}%")
```

---

## ⚠️ Important

- Le logging Serial utilise ~9600 bauds
- Transmission : ~1 Ko de données par minute de cuisson
- Pour logs longs (>24h), utiliser un logger externe ou SD card

---

## 🔍 Vérification Taille Programme

Après compilation, vérifiez :
```
Le croquis utilise XXXXX octets (XX%) de l'espace de stockage
```

**Objectif :** < 32256 octets (100%)
- Sans logging : ~31500 octets ✅
- Avec logging minimal : ~31750 octets ✅
- Avec logging complet : ~32200 octets ⚠️
