# 🔥 Guide du Simulateur CalciferAi

## 📋 Vue d'ensemble

Le simulateur CalciferAi est un **outil complet de développement et test** pour votre programme de contrôle de four céramique Arduino. Il vous permet de tester virtuellement votre configuration matérielle avant de la déployer sur l'Arduino physique.

## ✨ Ce que vous pouvez faire

### 🎮 Tester votre programme sans matériel
- Lancez des cycles de cuisson complets virtuellement
- Testez différents profils de température
- Validez la logique de votre programme
- **Aucun risque** pour le matériel ou la céramique

### ⚡ Accélérer le développement
- Testez un cycle de **10 heures en 6 minutes** (vitesse 100x)
- Itérez rapidement sur vos paramètres
- Détectez les bugs avant le déploiement
- Économisez du temps et de l'électricité

### 📊 Visualiser en temps réel
- **Graphiques de température** : consigne vs réalité
- **Graphique de puissance** : voir l'activité du relais
- **Panneau debug** : toutes les variables internes
- **Écran OLED virtuel** : émulation pixel par pixel

### 🔧 Ajuster finement
- Réglez les paramètres PID (Kp, Ki)
- Testez différentes vitesses de montée
- Optimisez les durées de paliers
- Validez les transitions entre phases

## 🚀 Démarrage rapide

### Installation (5 minutes)

```bash
# 1. Aller dans le dossier simulateur
cd /Users/nicolasregentete/Documents/GitHub/CalciferAi/simulator

# 2. Installer les dépendances
pip3 install -r requirements.txt

# 3. Lancer le simulateur
python3 main.py
```

### Premier test (2 minutes)

1. **Lancez** : `python3 main.py`
2. **Accélérez** : Appuyez sur `+` plusieurs fois (jusqu'à 50x ou 100x)
3. **Démarrez** : Appuyez sur `S`
4. **Observez** : Les graphiques montrent la montée en température

🎉 **Félicitations !** Vous venez de simuler votre premier cycle de cuisson.

## 📚 Documentation complète

Le simulateur est livré avec une documentation exhaustive :

### 🏃 Pour démarrer
- **`simulator/QUICK_START.md`** : Guide de démarrage rapide (5 min)
- **`simulator/README.md`** : Documentation complète

### 🔍 Pour comprendre
- **`simulator/ARCHITECTURE.md`** : Architecture technique détaillée
- **`simulator/COMPARISON.md`** : Différences simulateur vs Arduino réel

### 📝 Référence
- **`simulator/CHANGELOG.md`** : Historique des versions
- **`simulator/config.py`** : Tous les paramètres configurables

## 🎯 Cas d'usage

### 1. Développer un nouveau profil de cuisson

**Objectif** : Créer un profil pour grès haute température (1280°C)

**Étapes** :
1. Lancez le simulateur
2. Configurez les paramètres (températures, vitesses, durées)
3. Testez à vitesse 50x pour voir le cycle complet (~10 minutes)
4. Observez les graphiques et ajustez si nécessaire
5. Sauvegardez (automatique en EEPROM virtuelle)
6. Transférez vers l'Arduino quand satisfait

### 2. Régler le PID

**Objectif** : Optimiser la régulation de température

**Étapes** :
1. Lancez un cycle simple (Phase 1 seulement)
2. Vitesse normale (1x) pour observer en détail
3. Regardez le graphique de température
4. Si oscillations : réduire Kp dans `config.py`
5. Si montée trop lente : augmenter Kp
6. Si dépassement important : réduire Kp ou augmenter Ki
7. Relancez pour valider

### 3. Former un utilisateur

**Objectif** : Apprendre à utiliser l'interface sans risque

**Étapes** :
1. Démarrez le simulateur
2. Montrez la navigation avec les flèches ← →
3. Éditez un paramètre (Entrée, modifier, Entrée)
4. Lancez un programme (touche S)
5. Montrez l'arrêt d'urgence (touche S pendant exécution)
6. L'utilisateur s'entraîne sans risque matériel

### 4. Valider un long cycle

**Objectif** : Tester un cycle de 24 heures complet

**Étapes** :
1. Configurez votre cycle complet
2. Accélérez à 100x (24h → 14 minutes)
3. Lancez et observez
4. Vérifiez que toutes les phases se déroulent
5. Vérifiez les temps de maintien des plateaux
6. Si OK → transférez vers Arduino

## 🎮 Contrôles

### Clavier (recommandé)
- **← →** : Rotation encodeur
- **Entrée** : Clic encodeur
- **S** : Start/Stop
- **+ -** : Vitesse simulation
- **R** : Reset
- **0** : Vitesse 1x

### Souris
- Cliquez sur les boutons à l'écran
- Pratique pour la démonstration

## 📊 Comprendre l'interface

```
┌─────────────────────────────────────────────────────────────┐
│                                                               │
│  ┌──────────────┐         ┌──────────────────────┐          │
│  │ Écran OLED   │         │  Graphiques          │          │
│  │ (128x64)     │         │  - Température       │          │
│  │              │         │  - Puissance         │          │
│  └──────────────┘         └──────────────────────┘          │
│                                                               │
│  ┌──────────────┐         ┌──────────────────────┐          │
│  │ Contrôles    │         │  Panneau Debug       │          │
│  │ - Encodeur   │         │  - État              │          │
│  │ - Start/Stop │         │  - Températures      │          │
│  │ - Vitesse    │         │  - Puissance         │          │
│  └──────────────┘         └──────────────────────┘          │
│                                                               │
└─────────────────────────────────────────────────────────────┘
```

## ⚙️ Configuration avancée

### Modifier la physique du four

Éditez `simulator/config.py` :

```python
# Capacité thermique (J/°C)
# Plus élevé = four plus lent à chauffer/refroidir
KILN_THERMAL_MASS = 50000.0

# Puissance maximale (Watts)
# Votre four réel : vérifiez la plaque signalétique
MAX_HEATING_POWER = 3000.0

# Pertes thermiques (W/°C)
# Plus élevé = refroidit plus vite
HEAT_LOSS_COEFFICIENT = 15.0
```

### Ajuster le PID

```python
KP = 2.0   # Gain proportionnel (réactivité)
KI = 0.5   # Gain intégral (correction erreur persistante)
KD = 0.0   # Gain dérivé (non utilisé)
```

## 🔬 Différences avec l'Arduino réel

### Identique ✅
- Logique du programme (100%)
- Interface utilisateur (100%)
- Contrôle PID (100%)
- Gestion des erreurs (100%)

### Différent ⚠️
- Physique simplifiée (modèle mathématique)
- Pas de bruit sur les mesures
- Pas de problèmes matériels (I2C, SPI, etc.)

### En plus ✨
- Graphiques temps réel
- Simulation accélérée
- Panneau debug
- Sauvegarde JSON (vs EEPROM binaire)

**→ Voir `simulator/COMPARISON.md` pour les détails**

## 🛠️ Dépannage

### Problème : "Command not found: python"
**Solution** : Utilisez `python3` au lieu de `python`

### Problème : "No module named 'pygame'"
**Solution** : 
```bash
pip3 install pygame matplotlib numpy
```

### Problème : Le four ne chauffe pas dans le simulateur
**Vérifications** :
1. Avez-vous appuyé sur **S** pour démarrer ?
2. Le panneau info affiche-t-il "EN MARCHE" ?
3. La puissance est-elle > 0% ?

### Problème : Les modifications ne sont pas prises en compte
**Solution** : Appuyez sur **Entrée** pour quitter le mode édition (sauvegarde auto)

### Problème : Le simulateur est lent
**Solution** : 
- Fermez d'autres applications
- Les graphiques matplotlib sont coûteux en CPU
- C'est normal si votre ordinateur est ancien

## 📈 Prochaines étapes

Une fois satisfait du simulateur :

1. ✅ **Vérifiez** que tous vos profils fonctionnent
2. 📝 **Notez** les paramètres finaux
3. 🔧 **Transférez** vers l'Arduino
   - Le code est déjà dans `lucia/lucia.ino`
   - Vos paramètres sont sauvegardés
4. 🧪 **Testez** sur four réel avec précaution
5. 🎯 **Comparez** les résultats simulés vs réels
6. ⚙️ **Ajustez** si nécessaire

## 💡 Astuces pro

### Astuce 1 : Tests rapides
Créez des profils courts pour tester rapidement :
```
Phase 1: 200°C/h → 100°C, 1min
Phase 2: Désactivée (0°C)
Phase 3: Désactivée (0°C)
```
En 30 secondes vous voyez le comportement du PID.

### Astuce 2 : Comparer plusieurs runs
Lancez plusieurs fois avec des paramètres différents, notez les résultats dans un tableau.

### Astuce 3 : Utiliser le Reset
Appuyez sur **R** pour recommencer instantanément (four à 20°C).

### Astuce 4 : Raccourcis
Apprenez les raccourcis clavier, c'est plus rapide que la souris.

## 🎓 Ressources

### Documentation
- `simulator/README.md` - Guide complet
- `simulator/QUICK_START.md` - Démarrage rapide
- `simulator/ARCHITECTURE.md` - Technique
- `simulator/COMPARISON.md` - Simulateur vs Réel

### Code source
- `simulator/main.py` - Point d'entrée
- `simulator/arduino_core.py` - Logique principale
- `simulator/config.py` - Configuration

### Outils
- `simulator/check_installation.py` - Vérifier installation
- `simulator/launcher.sh` - Lanceur interactif

## 🤝 Support

### Questions fréquentes
Consultez d'abord `simulator/README.md` section "Dépannage"

### Bugs
Si vous trouvez un bug :
1. Notez exactement les étapes pour le reproduire
2. Notez les messages d'erreur
3. Vérifiez dans `simulator/CHANGELOG.md` les problèmes connus

### Améliorations
Le simulateur est extensible ! Vous pouvez :
- Ajouter de nouveaux graphiques
- Modifier la physique
- Créer de nouveaux modes d'affichage

Voir `simulator/ARCHITECTURE.md` pour comprendre le code.

## 🎉 Conclusion

Le simulateur CalciferAi est un **outil professionnel** pour développer et tester vos programmes de cuisson céramique en toute sécurité.

**Avantages** :
- ⚡ Développement **10 à 100 fois plus rapide**
- 🔒 **Zéro risque** matériel
- 📊 **Visualisation complète**
- 🎯 **Mise au point précise**

**Limitations** :
- ⚠️ Validation finale sur four réel requise
- ⚠️ Physique simplifiée

**Workflow recommandé** :
```
Développer sur simulateur → Valider sur Arduino → Tester sur four réel
```

---

**Bon développement ! 🔥🏺**

*Simulateur créé pour le projet CalciferAi*  
*Version 1.0.0 - Décembre 2025*

