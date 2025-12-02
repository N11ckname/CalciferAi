# 🔥 CalciferAi - Simulateur de Four Céramique

Simulateur Python complet pour tester le programme Arduino de contrôle de four céramique sans matériel physique.

## 🎯 Fonctionnalités

### ✅ Simulation complète
- **Physique réaliste du four** : inertie thermique, chauffage, refroidissement, pertes
- **Contrôle PID** : identique au code Arduino (Kp=2.0, Ki=0.5)
- **Thermocouple virtuel** : avec délai et lissage réaliste
- **Relais SSR** : contrôle PWM logiciel (cycle 1 seconde)

### 🖥️ Interface graphique
- **Écran OLED virtuel** : émulation pixel par pixel du SH1106 (128x64)
- **Contrôles interactifs** : encodeur rotatif, boutons cliquables
- **Graphiques temps réel** : température et puissance
- **Panneau de debug** : toutes les informations de l'état interne

### ⚡ Vitesse de simulation
- Simulation accélérée jusqu'à 100x pour tester rapidement
- Ralentissement possible pour observer les détails
- Temps réel (1x) par défaut

## 📦 Installation

### Prérequis
- Python 3.8 ou supérieur
- pip (gestionnaire de paquets Python)

### Installation des dépendances

```bash
cd simulator
pip install -r requirements.txt
```

**Dépendances :**
- `pygame==2.5.2` : Interface graphique
- `matplotlib==3.8.2` : Graphiques
- `numpy==1.26.2` : Calculs numériques

## 🚀 Lancement

```bash
cd simulator
python main.py
```

Le simulateur démarre avec :
- Four à température ambiante (20°C)
- Paramètres par défaut chargés depuis l'EEPROM virtuel
- Mode PROG_OFF (arrêt)

## 🎮 Contrôles

### Souris
- **Boutons à l'écran** : Cliquez pour interagir
  - `◄ ►` : Rotation de l'encodeur
  - `CLICK` : Clic sur l'encodeur
  - `START/STOP` : Démarrer/arrêter le programme
  - `◄ ►` (orange) : Vitesse de simulation
  - `RESET` : Réinitialiser la simulation

### Clavier
- **Flèches ← →** : Rotation de l'encodeur
- **Entrée / Espace** : Clic sur l'encodeur
- **S** : Démarrer/Arrêter le programme
- **R** : Reset du four
- **+/-** : Augmenter/diminuer la vitesse
- **0** : Vitesse normale (1x)

## 📖 Guide d'utilisation

### 1. Configurer un programme de cuisson

En mode **PROG_OFF** :

1. **Naviguer** : Utilisez les flèches ← → pour sélectionner un paramètre
   - Le paramètre sélectionné apparaît en **inverse vidéo** (fond blanc, texte noir)

2. **Éditer** : Appuyez sur **Entrée** pour éditer le paramètre
   - Un **cadre** apparaît autour du paramètre en édition

3. **Modifier** : Utilisez les flèches ← → pour changer la valeur
   - Les valeurs changent par pas (10°C pour temp, 10°C/h pour vitesse, 5min pour durée)

4. **Valider** : Appuyez sur **Entrée** pour sauvegarder
   - Les paramètres sont automatiquement sauvegardés en EEPROM virtuelle

### 2. Lancer un programme

1. Configurez tous les paramètres désirés
2. Appuyez sur **S** ou cliquez sur **START/STOP**
3. Le four démarre automatiquement la Phase 1

### 3. Accélérer la simulation

Pour tester rapidement un cycle complet :

1. Appuyez sur **+** plusieurs fois (ou cliquez sur `►` orange)
2. La vitesse peut aller jusqu'à **100x**
3. Un programme de 10 heures prend seulement 6 minutes à 100x !

### 4. Observer le comportement

Pendant l'exécution :
- **Écran OLED** : Affiche l'état en temps réel
- **Graphique température** : Courbes réelle (rouge) vs cible (vert)
- **Graphique puissance** : Puissance de chauffage (%)
- **Panneau info** : État détaillé du système

## 📊 Structure du programme

### Phases de cuisson

1. **Phase 0 (PROG_OFF)** : Configuration
2. **Phase 1** : Montée à la première température + plateau
3. **Phase 2** : Montée à la deuxième température + plateau
4. **Phase 3** : Montée à la troisième température + plateau
5. **Phase 4 (Cooldown)** : Refroidissement contrôlé

### Paramètres par défaut

| Paramètre | Valeur | Description |
|-----------|--------|-------------|
| Step1Temp | 100°C | Température phase 1 |
| Step1Speed | 50°C/h | Vitesse de montée phase 1 |
| Step1Wait | 5 min | Durée du plateau phase 1 |
| Step2Temp | 570°C | Température phase 2 |
| Step2Speed | 250°C/h | Vitesse de montée phase 2 |
| Step2Wait | 15 min | Durée du plateau phase 2 |
| Step3Temp | 1100°C | Température phase 3 |
| Step3Speed | 200°C/h | Vitesse de montée phase 3 |
| Step3Wait | 20 min | Durée du plateau phase 3 |
| Step4Speed | 150°C/h | Vitesse de refroidissement |
| Step4Target | 200°C | Température finale |

## 🔧 Configuration

### Modifier les paramètres physiques

Éditez `config.py` :

```python
# Masse thermique du four (J/°C)
KILN_THERMAL_MASS = 50000.0  # Four moyen

# Puissance de chauffage (Watts)
MAX_HEATING_POWER = 3000.0

# Coefficient de perte thermique (W/°C)
HEAT_LOSS_COEFFICIENT = 15.0
```

### Modifier les paramètres PID

```python
KP = 2.0  # Gain proportionnel
KI = 0.5  # Gain intégral
KD = 0.0  # Gain dérivé (non utilisé)
MAX_POWER_CHANGE = 10.0  # Changement max par cycle (%)
```

## 🐛 Dépannage

### Le simulateur ne démarre pas

```bash
# Vérifier les dépendances
pip install -r requirements.txt --upgrade

# Vérifier la version de Python
python --version  # Doit être >= 3.8
```

### L'écran OLED est noir

- C'est normal au démarrage
- En mode PROG_OFF, l'écran affiche les paramètres
- Vérifiez que le texte blanc est visible sur fond noir

### Le four ne chauffe pas

- Vérifiez que vous êtes en mode **PROG_ON** (appuyez sur S)
- Le relais doit être **ON** (visible dans le panneau info)
- La puissance doit être > 0%

### La simulation est trop lente

- Augmentez la vitesse avec **+** ou le bouton orange `►`
- La vitesse actuelle s'affiche en jaune (ex: "10.0x")

## 📁 Structure des fichiers

```
simulator/
├── main.py              # Point d'entrée
├── arduino_core.py      # Logique Arduino (lucia.ino)
├── temperature.py       # Contrôle PID (temperature.cpp)
├── display.py          # Rendu OLED (display.cpp)
├── kiln_physics.py     # Simulation physique du four
├── ui.py               # Interface Pygame
├── config.py           # Configuration
├── requirements.txt    # Dépendances
├── README.md           # Ce fichier
└── simulator_eeprom.json  # EEPROM virtuelle (créé auto)
```

## 🎓 Cas d'usage

### Test rapide d'un programme complet

1. Configurez votre programme de cuisson
2. Accélérez à 50x ou 100x
3. Appuyez sur S pour démarrer
4. Observez les graphiques
5. Vérifiez que toutes les phases se déroulent correctement

### Réglage fin du PID

1. Vitesse normale (1x)
2. Lancez une phase
3. Observez la courbe de température
4. Ajustez Kp et Ki dans `config.py`
5. Relancez pour comparer

### Test de sécurité

Le simulateur teste automatiquement :
- Défaillance du thermocouple (après 2 minutes → arrêt)
- Dépassement de température (le PID régule)
- Changements brusques de consigne

## 🔬 Différences avec le matériel réel

### Identique
- ✅ Logique du programme
- ✅ Contrôle PID
- ✅ Interface utilisateur
- ✅ Affichage OLED
- ✅ Gestion des erreurs

### Simplifié
- ⚠️ Physique du four (modèle simplifié mais réaliste)
- ⚠️ Pas de bruit sur les mesures
- ⚠️ Pas de variations du secteur

### Amélioré
- ✨ Graphiques en temps réel
- ✨ Simulation accélérée
- ✨ Panneau de debug
- ✨ Historique complet

## 💡 Conseils

1. **Testez d'abord à vitesse élevée** pour voir le cycle complet
2. **Puis ralentissez** pour analyser les transitions
3. **Utilisez Reset** pour recommencer rapidement
4. **Les paramètres sont sauvegardés** automatiquement en EEPROM
5. **Les graphiques** montrent l'écart entre consigne et réalité

## 🤝 Contribution

Ce simulateur est conçu pour être facilement extensible :
- Ajoutez de nouveaux graphiques dans `ui.py`
- Modifiez la physique dans `kiln_physics.py`
- Personnalisez l'affichage dans `display.py`

## 📝 Licence

Ce simulateur accompagne le projet CalciferAi.
Voir le README principal du projet pour plus d'informations.

---

**Bon test ! 🔥**

