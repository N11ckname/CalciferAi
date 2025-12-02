# 🎯 Premier Lancement - Guide Visuel

## Ce que vous allez voir

### 1️⃣ Au démarrage

Quand vous lancez `python3 main.py`, voici ce qui s'affiche dans le terminal :

```
🔥 Initialisation du simulateur CalciferAi...
✅ Simulateur prêt!

============================================================
CONTRÔLES:
  Flèches ← → : Rotation encodeur
  Entrée/Espace : Clic encodeur
  S : Bouton Start/Stop
  R : Reset du four
  +/- : Vitesse de simulation
  0 : Vitesse normale (1x)
============================================================
```

### 2️⃣ La fenêtre du simulateur

Une fenêtre de 1200x800 pixels s'ouvre avec 4 zones :

```
┌─────────────────────────────────────────────────────────────┐
│                                                               │
│  🖥️ ÉCRAN OLED (en haut à gauche)                           │
│  ┌──────────────────────┐                                   │
│  │ OFF                  │                                   │
│  │ P1:50C/h >100C, 5m   │                                   │
│  │ P2:250C/h >570C,15m  │                                   │
│  │ P3:200C/h >1100C,20m │                                   │
│  │ Cool:150C/h <200C    │                                   │
│  │ T:20C                │                                   │
│  │ OFF,0%               │                                   │
│  └──────────────────────┘                                   │
│                                                               │
│  🎮 CONTRÔLES (en bas à gauche)                              │
│  [◄] [►] [CLICK] [START/STOP]                               │
│  [◄] [►] [1x]  Vitesse: 1.0x                                │
│  [RESET]                                                     │
│                                                               │
│  📊 GRAPHIQUES (à droite)                                    │
│  ┌─────────────────────────┐                                │
│  │ Température vs Temps    │                                │
│  │   🔴 Réelle             │                                │
│  │   🟢 Consigne           │                                │
│  └─────────────────────────┘                                │
│  ┌─────────────────────────┐                                │
│  │ Puissance vs Temps      │                                │
│  │   🟠 Puissance PWM      │                                │
│  └─────────────────────────┘                                │
│                                                               │
│  ℹ️ INFO (en bas à droite)                                   │
│  État: ARRÊTÉ                                                │
│  Phase: Arrêt (Phase 0)                                      │
│  Température actuelle: 20.0°C                                │
│  Température cible: 20.0°C                                   │
│  Puissance chauffage: 0%                                     │
│  Relais: OFF                                                 │
│                                                               │
└─────────────────────────────────────────────────────────────┘
```

---

## 🎮 Essayez maintenant !

### Test 1 : Naviguer dans les paramètres (30 secondes)

1. **Appuyez sur →** (flèche droite)
   - Le premier paramètre devient **surligné** (fond blanc, texte noir)
   - C'est "100C" dans "P1:50C/h >100C, 5m"

2. **Appuyez encore sur →**
   - Le paramètre suivant est sélectionné
   - C'est "50C/h"

3. **Continuez avec →**
   - Vous naviguez à travers tous les paramètres
   - Il y en a 11 au total

4. **Appuyez sur ←**
   - Vous revenez en arrière

**✅ Vous savez maintenant naviguer !**

---

### Test 2 : Modifier un paramètre (1 minute)

1. **Sélectionnez** le premier paramètre avec → (température Phase 1 : "100C")

2. **Appuyez sur Entrée**
   - Un **cadre** apparaît autour du paramètre
   - Mode édition activé

3. **Appuyez sur →** plusieurs fois
   - La valeur augmente (110, 120, 130...)
   - Par pas de 10°C

4. **Appuyez sur ←**
   - La valeur diminue

5. **Appuyez sur Entrée** pour valider
   - Le cadre disparaît
   - La valeur est sauvegardée en EEPROM virtuelle

**✅ Vous savez maintenant modifier les paramètres !**

---

### Test 3 : Lancer un programme (2 minutes)

#### Accélérer d'abord

1. **Appuyez sur +** (ou cliquez sur le bouton ► orange)
   - La vitesse passe à 1.5x
   
2. **Appuyez encore sur +** plusieurs fois
   - 2.3x → 3.4x → 5.1x → 7.7x → 11.5x → 17.2x → 25.9x
   
3. **Continuez jusqu'à 50x ou 100x**
   - Vous verrez "Vitesse: 50.0x" ou "100.0x"

#### Démarrer le programme

4. **Appuyez sur S** (ou cliquez sur START/STOP vert)
   - L'écran OLED affiche "ON" au lieu de "OFF"
   - Le bouton START/STOP devient rouge
   - Le panneau info affiche "EN MARCHE"

#### Observer

5. **Regardez les graphiques**
   - La courbe 🔴 rouge (température réelle) commence à monter
   - La courbe 🟢 verte (consigne) monte aussi
   - Le graphique 🟠 orange (puissance) monte à ~80-100%

6. **Regardez l'écran OLED**
   - "T:25->30C" (température actuelle → cible)
   - "ON,85%" (puissance)
   - Les phases en blanc/grisé selon où vous êtes

7. **Regardez le panneau info**
   - "État: EN MARCHE"
   - "Phase: Phase 1 - Chauffage"
   - Les températures qui montent
   - Le relais qui s'allume/s'éteint (PWM)

#### À 100x

**En 30 secondes** vous verrez :
- Montée de 20°C à 100°C
- Maintien du plateau 5 minutes
- Début de la Phase 2

**En 6 minutes** vous verrez :
- Tout le cycle jusqu'au refroidissement !

#### Arrêter

8. **Appuyez sur S** à nouveau
   - Le programme s'arrête immédiatement
   - Retour en mode OFF
   - Le relais s'éteint

**✅ Vous avez testé votre premier cycle de cuisson !**

---

### Test 4 : Reset (10 secondes)

1. **Appuyez sur R**
   - Le four retourne à 20°C instantanément
   - Tous les graphiques sont effacés
   - Vous pouvez recommencer

**✅ Vous savez réinitialiser le simulateur !**

---

## 🎯 Ce que vous devriez observer

### Comportement normal ✅

#### Au démarrage du programme
- ✅ La température monte progressivement
- ✅ La puissance est élevée au début (80-100%)
- ✅ Puis diminue quand on approche la cible
- ✅ Le relais clignote (PWM)

#### Pendant un plateau
- ✅ La température oscille légèrement autour de la cible (±2°C)
- ✅ La puissance varie (20-60% typiquement)
- ✅ Le temps de plateau se compte

#### Pendant le refroidissement
- ✅ La température descend
- ✅ La puissance est faible (0-30%)
- ✅ Parfois le four chauffe un peu pour ralentir la descente

### Comportements étranges ⚠️

#### Si la température monte trop vite
- ⚠️ Vérifiez MAX_HEATING_POWER dans config.py
- ⚠️ Peut-être trop élevé pour votre simulation

#### Si elle oscille beaucoup
- ⚠️ Le PID est peut-être trop agressif
- ⚠️ Réduisez Kp dans config.py

#### Si elle ne suit pas la consigne
- ⚠️ Le PID est peut-être trop mou
- ⚠️ Augmentez Kp dans config.py

---

## 🎨 Comprendre les couleurs

### Écran OLED
- **Blanc** = Phase en cours
- **Grisé** = Phase terminée ou future
- **Inverse vidéo** = Paramètre sélectionné
- **Cadre** = Paramètre en édition

### Boutons
- **Bleu** = Contrôles encodeur
- **Vert** = START (quand arrêté)
- **Rouge** = STOP (quand en marche)
- **Orange** = Vitesse simulation

### Graphiques
- **🔴 Rouge** = Température réelle
- **🟢 Vert** = Température consigne
- **🟠 Orange** = Puissance de chauffage

### Panneau info
- **Vert** = Informations importantes
- **Blanc** = Informations normales

---

## 💡 Astuces pour débuter

### Astuce 1 : Commencez lentement
- Testez d'abord à vitesse 1x
- Observez bien le comportement
- Puis accélérez progressivement

### Astuce 2 : Testez des cycles courts
- Configurez Phase 1 seulement (ex: 100°C, 1min)
- Mettez Phase 2 et 3 à 0°C
- C'est rapide à tester même à 1x

### Astuce 3 : Utilisez Reset souvent
- Pour recommencer rapidement
- Pour comparer différents paramètres
- Pour effacer les graphiques

### Astuce 4 : Observez les graphiques
- Ils montrent le comportement réel du PID
- Regardez si la courbe rouge suit bien la verte
- Regardez comment la puissance s'adapte

---

## 📚 Après le premier test

Une fois que vous êtes à l'aise :

1. ✅ Lisez **QUICK_START.md** pour les scénarios de test
2. ✅ Lisez **README.md** pour toutes les fonctionnalités
3. ✅ Configurez vos propres profils de cuisson
4. ✅ Ajustez les paramètres PID si nécessaire
5. ✅ Testez à 100x pour valider tout le cycle

---

## ❓ Questions fréquentes

### Q: Pourquoi la température monte par paliers ?
**R:** C'est normal, le simulateur calcule par pas de temps. À vitesse élevée (100x), les pas sont plus visibles.

### Q: Pourquoi le relais clignote ?
**R:** C'est le PWM logiciel. Le relais s'allume/s'éteint selon le % de puissance.

### Q: Puis-je modifier les paramètres pendant l'exécution ?
**R:** Non, seulement en mode OFF. Arrêtez d'abord avec S.

### Q: Les modifications sont-elles sauvegardées ?
**R:** Oui, automatiquement dans `simulator_eeprom.json` quand vous sortez du mode édition.

### Q: Comment revenir aux valeurs par défaut ?
**R:** Supprimez le fichier `simulator_eeprom.json` et relancez.

---

## 🎉 Bravo !

Vous savez maintenant utiliser le simulateur CalciferAi !

**Prochaines étapes :**
- 📖 Lire la documentation complète
- 🧪 Tester vos profils de cuisson
- ⚙️ Ajuster les paramètres PID
- ✅ Valider avant transfert Arduino

**Bon développement ! 🔥**

