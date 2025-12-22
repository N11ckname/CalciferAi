# CalciferAi - Notice d'Utilisation

## 🎯 Présentation

CalciferAi est un système de contrôle automatique pour four céramique. Il permet de programmer des cycles de cuisson en 3 phases de chauffe suivies d'un refroidissement contrôlé.

## 🎮 Les Commandes

### Boutons et Encodeur
- **Encodeur rotatif** : Naviguer et modifier les valeurs
- **Clic sur l'encodeur** : Valider/éditer un paramètre
- **Bouton poussoir** : Démarrer/Arrêter le programme

### LED Témoin
- **LED allumée** : Le four chauffe

## 📋 Écran Principal (Mode Arrêt)

L'écran affiche les paramètres de votre programme de cuisson :

**Simulation d'écran :**
```
┌────────────────────────────────┐
│ 25C              Settings      │
│ P1: 50C/h >100C 5m             │
│ P2: 250C/h >570C 15m           │
│ P3: 200C/h >1100C 20m          │
│ Cool: 150C/h <200C             │
└────────────────────────────────┘
```
*Note : Le paramètre sélectionné apparaît avec un cadre*

### Phase 1, 2 et 3 (Montée en température)
- **Vitesse** : Vitesse de chauffe en °C/h (10 à 1000)
- **Température** : Température cible en °C (0 à 1500)
- **Durée** : Temps de maintien au palier en minutes (0 à 999)

### Phase 4 (Refroidissement)
- **Vitesse** : Vitesse de refroidissement en °C/h (1 à 1000)
- **Température** : Température de fin de cycle en °C (0 à 1000)

## ⚙️ Configuration d'un Programme

### 1. Navigation entre les paramètres
1. Tournez l'encodeur pour sélectionner un paramètre (il s'affiche en vidéo inversé)
2. Les paramètres défilent dans l'ordre : Settings → vitesse → température → durée pour chaque phase


## 🚀 Démarrage d'un Programme

1. Vérifiez que tous vos paramètres sont corrects
2. **Appuyez sur le bouton poussoir** pour démarrer
3. L'écran passe en mode "EN MARCHE"
4. Le programme démarre automatiquement

**Simulation écran en cours de cuisson :**
```
┌────────────────────────────────┐
│ Phase 2                        │
│ 250C/h->570C, 15m              │
│────────────────────────────────│
│ Temp Read            365C      │
│ Temp Target          420C      │
│ Heat Power            78%      │
│ Phase                 45%      │
└────────────────────────────────┘
```

### Reprise à chaud
Si vous démarrez un programme alors que le four est déjà chaud, le système détecte automatiquement la phase appropriée et reprend à partir de la température actuelle.

## 🔥 Pendant la Cuisson (Mode En Marche)

### Affichage
- **Phase en cours** : Affichée clairement (exemple : "Phase 2 : 250°C/h→570C, 15m")
- **Température actuelle vs cible** : Affichées en temps réel
- **État du chauffage** : "Temp Read" et "Temp Target" affichés
- **Puissance** : "Heat Power" avec pourcentage (0-100%)

### Arrêt d'urgence
**Appuyez sur le bouton poussoir** à tout moment pour arrêter immédiatement le programme et couper le chauffage.

## 🔬 Fonctionnalités Optionnelles (Avancé)

CalciferAi propose deux fonctionnalités optionnelles qui peuvent être activées selon vos besoins :

### 📊 Le Graphique de Température (ENABLE_GRAPH)

**Description** : Affiche un graphique en temps réel pendant la cuisson montrant :
- La courbe de température prévue (profil programmé)
- La courbe de température réelle mesurée
- Permet de visualiser si le four suit correctement le programme

**Accès** : Pendant la cuisson, cliquez sur l'encodeur pour basculer entre l'écran principal et le graphique.

**Simulation écran graphique :**
```
┌────────────────────────────────┐
│P:12.5  ┌──────────────────┐    │
│I:8.3   │         ╱────────│1100C│
│        │       ╱          │    │
│        │     ╱            │    │
│        │   ╱··            │    │
│        │ ╱·               │    │
│        └──────────────────┘    │
│                       4h30      │
└────────────────────────────────┘
```
*Ligne continue = température cible*  
*Points = température mesurée*

**Consommation** : ~800 octets de RAM

### 📡 Le Logging Série (ENABLE_LOGGING)

**Description** : Envoie les données de cuisson via le port série USB vers un ordinateur :
- Température actuelle et cible en temps réel
- Valeurs PID (Proportionnel, Intégral)
- Puissance de chauffe (%)
- Erreur de température
- Une ligne de données toutes les 5 secondes

**Utilisation** : Connectez l'Arduino à un ordinateur, ouvrez le moniteur série (9600 bauds) pour voir et enregistrer les données.

**Installation des dépendances** (une seule fois) :
```bash
cd /chemin/vers/CalciferAi/Logger
pip3 install -r requirements_logger.txt
```

**Lancer le Logger avec graphique** :
```bash
cd /chemin/vers/CalciferAi
python3 Logger/arduino_logger.py
```

**Exemple de sortie série :**
```
=== LUCIA START ===
PID: Kp=2.50 Ki=0.03
Time(ms), Temp(C), Target(C), P, I, Power(%), Error(C)
---
>>> PROGRAMME DEMARRE <<<
Temperature initiale: 25.5C
Phase detectee: 1
---
5000, 28.3, 30.5, 5.5, 0.2, 15, 2.2
10000, 33.1, 35.2, 5.2, 0.8, 20, 2.1
15000, 38.7, 40.8, 5.2, 1.5, 28, 2.1
...
```

**Consommation** : ~250 octets de RAM

### ⚠️ Limitation Importante

**Vous ne pouvez PAS activer les deux en même temps !**

**Raison** : L'Arduino Uno dispose seulement de **2048 octets de RAM** au total. Les deux fonctionnalités ensemble consommeraient ~1050 octets, ce qui laisserait trop peu de mémoire pour le fonctionnement normal du système et provoquerait des plantages ou des comportements imprévisibles.

### 🔧 Comment Activer ces Fonctionnalités

**Fichier à modifier** : `lucia/definitions.h`

**Lignes 21-23** :
```cpp
// ===== FONCTIONNALITÉS OPTIONNELLES =====
// Décommentez pour activer (voir ACTIVATION_FONCTIONNALITES.md pour détails)
#define ENABLE_LOGGING  // Logging Serial (~250 octets) - Monitoring/Debug
//#define ENABLE_GRAPH    // Graphe température (~800 octets) - Visualisation
```

**Pour activer le LOGGING** (configuration par défaut) :
```cpp
#define ENABLE_LOGGING  // ← Ligne active (sans //)
//#define ENABLE_GRAPH    // ← Ligne désactivée (avec //)
```

**Pour activer le GRAPHIQUE** :
```cpp
//#define ENABLE_LOGGING  // ← Ligne désactivée (ajoutez //)
#define ENABLE_GRAPH    // ← Ligne active (retirez //)
```



**⚠️ Important** : Après modification, vous devez **recompiler et téléverser** le programme sur l'Arduino.

### 💡 Quel Mode Choisir ?

- **LOGGING** : Pour analyser et enregistrer les cuissons, créer des courbes sur ordinateur, déboguer
- **GRAPHIQUE** : Pour surveiller visuellement la cuisson directement sur l'écran OLED, sans ordinateur
- **Aucun** : Pour économiser de la RAM si vous rencontrez des problèmes de stabilité

## 🔧 Réglages Avancés (Menu Settings)

### Accès au menu Settings
1. En mode Arrêt, sélectionnez l'icône "Settings" en haut à droite
2. Cliquez sur l'encodeur pour entrer dans les réglages

**Simulation écran Settings :**
```
┌────────────────────────────────┐
│ SETTINGS             v01.0     │
│                                │
│ Heat Cycle             1000ms  │
│ Kp                      2.5    │
│ Ki                     0.030   │
│ Max delta                10C   │
│ Max Temp               1200C   │  ← Sécurité
│ Exit                    <--    │
└────────────────────────────────┘
```
*Note : L'élément sélectionné est entouré d'un cadre. Utilisez l'encodeur pour naviguer entre les paramètres.*

### Paramètres disponibles
- **Heat Cycle** : Durée du cycle PWM (100 à 10000 ms) - *Avancé*
- **Kp** : Gain proportionnel PID (0.0 à 10.0) - *Avancé*
- **Ki** : Gain intégral PID (0.0 à 1.0) - *Avancé*
- **Max delta** : Tolérance de fin de phase (1 à 50°C) - *Recommandé : 10°C*
- **Max Temp** : Température maximum du four (500 à 1500°C) - *🛡️ SÉCURITÉ*
- **Exit** : Sortir du menu Settings

⚠️ **Notes importantes** :
- Ne modifiez les paramètres PID (Kp, Ki) que si vous comprenez leur fonctionnement. Les valeurs par défaut sont optimisées.

### 🛡️ Protection Max Temp (IMPORTANT)

**Max Temp** est un paramètre de sécurité crucial qui limite la température maximum programmable :

- **Valeur par défaut** : 1200°C (four céramique standard)
- **Plage de réglage** : 500°C à 1500°C
- **Modification** : Par pas de 10°C

**Rôle de protection** :
1. Empêche de programmer des températures supérieures aux capacités du four
2. Protège contre les erreurs de manipulation (ex: 1800°C au lieu de 180°C)
3. Permet d'adapter le système à différents types de fours
4. Les températures des phases 1, 2 et 3 sont automatiquement limitées à cette valeur

**Exemples de réglage** :
- Four Raku : 1000-1100°C
- Four grès/porcelaine : 1200-1300°C
- Four haute température : 1400-1500°C

⚠️ **Avant de modifier** : Vérifiez la température maximum supportée par votre four dans sa documentation technique !

## ⚠️ Messages d'Erreur

### "Temp fail 2min" / "Heat stopped"
**Cause** : Le capteur de température ne fonctionne pas correctement depuis plus de 2 minutes.

**Simulation écran d'erreur :**
```
┌────────────────────────────────┐
│ ERROR!                         │
│                                │
│ Temp fail 2min                 │
│ Heat stopped                   │
│ Check sensor                   │
│                                │
└────────────────────────────────┘
```

**Action** :
1. Vérifiez les connexions du thermocouple
2. Vérifiez que le thermocouple n'est pas endommagé
3. Appuyez sur le bouton pour réinitialiser

### "MAX31856 Error!" / "Check wiring"
**Cause** : Le module de lecture de température n'est pas détecté au démarrage.

**Simulation écran d'erreur :**
```
┌────────────────────────────────┐
│                                │
│ MAX31856 Error!                │
│                                │
│ Check wiring                   │
│ Press to retry                 │
│                                │
└────────────────────────────────┘
```

**Action** :
1. Vérifiez toutes les connexions du module MAX31856
2. Redémarrez le système
3. Appuyez sur le bouton pour tenter une reconnexion

## 🛡️ Consignes de Sécurité

### ⚠️ IMPORTANT
1. **Ne laissez JAMAIS un four en chauffe sans surveillance**
2. Le relais se coupe automatiquement en mode Arrêt
3. En cas d'erreur température > 2 minutes, le chauffage s'arrête automatiquement
4. Le bouton d'arrêt fonctionne à tout moment (arrêt immédiat)

### Recommandations
- Testez toujours votre programme à vide avant une vraie cuisson
- Notez vos programmes de cuisson réussis pour les réutiliser
- Surveillez les premières minutes après le démarrage pour vérifier le bon fonctionnement
- N'ouvrez pas le four pendant la cuisson (sauf urgence)

## 📊 Exemple de Programme Typique

### Cuisson Raku (Exemple)
- **Phase 1** : 100°C/h → 150°C, maintien 60 min (séchage)
- **Phase 2** : 150°C/h → 600°C, maintien 10 min (préchauffage)
- **Phase 3** : 600°C/h → 980°C, maintien 10 min (cuisson)
- **Refroidissement** : 150°C/h → 500°C (refroidissement contrôlé)

### Cuisson Grès (Exemple)
- **Phase 1** : 50°C/h → 100°C, maintien 5 min
- **Phase 2** : 250°C/h → 570°C, maintien 15 min
- **Phase 3** : 200°C/h → 1100°C, maintien 20 min
- **Refroidissement** : 150°C/h → 200°C

## 💾 Sauvegarde des Paramètres

Les paramètres de votre programme sont **automatiquement sauvegardés** à chaque modification. Ils sont conservés même après une coupure de courant.

## 📞 En Cas de Problème

1. **Le four ne chauffe pas** : Vérifiez que le programme est démarré (bouton poussoir)
2. **La température ne monte pas** : Vérifiez le relais SSR et le câblage du four
3. **L'écran est noir** : Vérifiez l'alimentation et les connexions de l'écran OLED
4. **Température aberrante** : Vérifiez le thermocouple (Type S requis)

---

**Version** : 1.0  
**Système** : CalciferAi - Contrôleur intelligent de four céramique

