# Changelog - Simulateur CalciferAi

## Version 1.0.0 (Décembre 2025)

### 🎉 Première version complète

#### ✨ Fonctionnalités principales
- **Simulation physique réaliste** du four céramique
  - Inertie thermique configurable
  - Pertes par conduction, convection et radiation
  - Thermocouple virtuel avec délai et lissage
  
- **Contrôle PID complet**
  - Identique au code Arduino (Kp=2.0, Ki=0.5, Kd=0)
  - Anti-windup de l'intégrale
  - Limitation du taux de changement (10%/cycle)
  - PWM logiciel (1 seconde)

- **Interface utilisateur complète**
  - Émulation pixel par pixel de l'écran OLED SH1106 (128x64)
  - Encodeur rotatif virtuel
  - Boutons virtuels (encodeur + push)
  - Raccourcis clavier pour tous les contrôles

- **Visualisation avancée**
  - Graphiques temps réel (température, puissance)
  - Panneau debug avec toutes les variables
  - Historique complet des données

- **Gestion des programmes de cuisson**
  - 4 phases configurables (3 chauffages + refroidissement)
  - Navigation et édition des paramètres
  - Sauvegarde automatique en EEPROM virtuelle (JSON)
  - Calcul du temps restant

#### 🚀 Simulation accélérée
- Vitesse variable de 0.1x à 100x
- Test d'un cycle de 10h en 6 minutes (à 100x)
- Contrôles interactifs : +/- ou boutons

#### 📚 Documentation
- README complet avec guide d'utilisation
- QUICK_START pour démarrage rapide
- ARCHITECTURE pour comprendre le code
- Commentaires détaillés dans tous les fichiers

#### 🛠️ Outils
- Script de vérification d'installation
- Lanceur interactif (launcher.sh)
- Fichier .gitignore configuré

#### ✅ Fidélité au code Arduino
- Transposition ligne par ligne de lucia.ino
- Même logique de phases et transitions
- Même interface utilisateur (écran OLED)
- Même gestion d'erreurs (défaillance température)
- Même sauvegarde EEPROM

#### 🎨 Interface
- Fenêtre 1200x800 pixels
- Écran OLED agrandi x6
- Thème sombre pour confort visuel
- 60 FPS pour fluidité

### 📦 Modules créés
- `main.py` - Point d'entrée et orchestration
- `arduino_core.py` - Logique Arduino (lucia.ino)
- `temperature.py` - Contrôle PID (temperature.cpp)
- `display.py` - Rendu OLED (display.cpp)
- `kiln_physics.py` - Simulation physique
- `ui.py` - Interface Pygame
- `config.py` - Configuration centralisée

### 🧪 Tests effectués
- ✅ Compilation Python (syntaxe)
- ✅ Vérification structure des fichiers
- ✅ Script de vérification d'installation

### 📝 À venir (futures versions)
- [ ] Tests unitaires automatisés
- [ ] Enregistrement et replay de sessions
- [ ] Export des données en CSV
- [ ] Simulation de pannes (relais, thermocouple)
- [ ] Mode comparaison (plusieurs runs)
- [ ] Profils de cuisson prédéfinis
- [ ] Interface web optionnelle

### 🐛 Problèmes connus
- Aucun pour l'instant (première version)

---

## Compatibilité

- **Python** : 3.8+
- **OS** : macOS, Linux, Windows
- **Dépendances** : 
  - pygame 2.5.2
  - matplotlib 3.8.2
  - numpy 1.26.2

## Installation

```bash
cd simulator
pip3 install -r requirements.txt
python3 main.py
```

## Support

Pour toute question ou bug, référez-vous à :
- README.md pour la documentation complète
- QUICK_START.md pour un démarrage rapide
- ARCHITECTURE.md pour comprendre le code

