# 🚀 Guide de Démarrage - Projet LUCIA

Contrôleur de four céramique avec Arduino Uno

## 📁 Structure du Projet

```
CalciferAi/
├── LUCIA/              ← Programme principal
│   ├── lucia.ino       (Fichier principal)
│   ├── definitions.h   (Définitions communes)
│   ├── display.h/cpp   (Gestion affichage)
│   ├── temperature.h/cpp (Contrôle PID)
│   ├── README.md       (Documentation)
│   └── OPTIMISATION_RAM.md
│
├── TEST_MODE/          ← Programme de test (COMMENCER ICI!)
│   ├── TEST_MODE.ino   (Tests composants)
│   ├── README.md       (Guide détaillé)
│   ├── QUICK_START.md  (Démarrage rapide)
│   └── WIRING_DIAGRAM.txt (Schéma câblage)
│
└── Documentation/
    ├── Prompt_CalciferAi_EN.md (Spécifications)
    └── BUILD_GUIDE.md
```

## 🎯 Étapes d'Installation

### Étape 1 : Installer les Bibliothèques Arduino

Dans l'IDE Arduino → Gestionnaire de bibliothèques :

1. **U8g2** (par oliver) - Écran OLED
2. **Adafruit MAX31856** (par Adafruit) - Thermocouple
3. **Encoder** (par Paul Stoffregen) - Encodeur rotatif

### Étape 2 : Câbler les Composants

Suivre le schéma dans `TEST_MODE/WIRING_DIAGRAM.txt`

**Résumé rapide :**
- Écran OLED → I2C (A4, A5)
- MAX31856 → SPI (D10-D13)
- Encodeur → D2, D3, D4
- Bouton → D5
- Relais → D6
- LED → A1

### Étape 3 : Tester le Matériel (IMPORTANT!)

**Avant d'utiliser LUCIA, testez tous les composants :**

```bash
1. Ouvrir TEST_MODE/TEST_MODE.ino
2. Uploader sur Arduino Uno
3. Suivre le guide TEST_MODE/QUICK_START.md
4. Vérifier que tous les tests passent ✅
```

**Durée : ~3 minutes**

### Étape 4 : Uploader LUCIA

Une fois tous les tests validés :

```bash
1. Ouvrir LUCIA/lucia.ino
2. Uploader sur Arduino Uno
3. Le four est prêt à fonctionner!
```

## ⚡ Démarrage Rapide (Si Matériel Déjà Testé)

```bash
1. Câbler selon WIRING_DIAGRAM.txt
2. Uploader LUCIA/lucia.ino
3. Configurer les paramètres de cuisson
4. Appuyer sur le bouton pour démarrer
```

## 📊 Utilisation de LUCIA

### Mode PROG_OFF (Four arrêté)

- **Tourner encodeur** : Sélectionner paramètre
- **Clic encodeur** : Éditer paramètre
- **Tourner** : Modifier valeur
- **Clic** : Sauvegarder
- **Bouton poussoir** : Démarrer programme

### Mode PROG_ON (Cuisson en cours)

- **Écran** : Affiche phase actuelle, temps restant, température
- **Bouton poussoir** : Arrêt d'urgence

### Paramètres Configurables

- **Phase 1** : Température, vitesse, durée palier
- **Phase 2** : Température, vitesse, durée palier
- **Phase 3** : Température, vitesse, durée palier
- **Refroidissement** : Vitesse, température cible

## ⚠️ Sécurité

✅ Le relais s'éteint automatiquement en cas d'erreur  
✅ Arrêt si défaut thermocouple pendant 2 minutes  
✅ Bouton d'arrêt d'urgence toujours actif  
✅ Paramètres sauvegardés automatiquement  

## 🔧 Dépannage

### Erreur de Compilation "Mémoire insuffisante"

Le programme est optimisé pour Arduino Uno (2 Ko RAM). Si erreur :
- Vérifier que le graphique est désactivé (par défaut)
- Consulter `LUCIA/OPTIMISATION_RAM.md`

### Composant ne Fonctionne Pas

1. Lancer `TEST_MODE/TEST_MODE.ino`
2. Identifier quel test échoue
3. Consulter `TEST_MODE/README.md` section Dépannage

### Température Incorrecte

- Vérifier type thermocouple (Type S) dans code
- Vérifier polarité thermocouple
- Vérifier connexions MAX31856

## 📚 Documentation Complète

- **LUCIA/README.md** : Guide utilisateur complet
- **TEST_MODE/README.md** : Guide de test détaillé
- **Prompt_CalciferAi_EN.md** : Spécifications techniques complètes

## 💾 Caractéristiques Techniques

- **Microcontrôleur** : Arduino Uno (ATmega328P)
- **RAM utilisée** : ~1300 octets (64%)
- **Flash utilisée** : ~25800 octets (80%)
- **Contrôle** : PID avec PWM logiciel (1s)
- **Rafraîchissement écran** : 500ms
- **Sauvegarde** : EEPROM automatique

## 🎓 Pour Commencer

**Nouveau projet ?** → Commencez par `TEST_MODE`  
**Déjà testé ?** → Passez directement à `LUCIA`  
**Problème ?** → Consultez les README respectifs  

## 📞 Support

En cas de problème :
1. Vérifier le câblage
2. Lancer TEST_MODE
3. Consulter la documentation
4. Vérifier le moniteur série (9600 bauds)

---

**Bonne cuisson ! 🔥**

