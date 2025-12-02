# 🚀 Démarrage Rapide - Simulateur CalciferAi

## Installation Express (5 minutes)

### 1. Installer les dépendances

```bash
cd /Users/nicolasregentete/Documents/GitHub/CalciferAi/simulator
pip3 install pygame matplotlib numpy
```

ou avec le fichier requirements.txt :

```bash
pip3 install -r requirements.txt
```

### 2. Lancer le simulateur

```bash
python3 main.py
```

## Premier test (2 minutes)

### Scénario 1 : Test rapide d'un cycle complet

1. **Lancez le simulateur** : `python3 main.py`

2. **Accélérez la simulation** :
   - Appuyez sur `+` plusieurs fois
   - Montez à **50x** ou **100x**

3. **Démarrez le programme** :
   - Appuyez sur `S` (ou cliquez sur START/STOP)

4. **Observez** :
   - L'écran OLED montre "ON" et les phases
   - Les graphiques montrent la montée en température
   - Le panneau info affiche l'état en temps réel

5. **Résultat** :
   - À 100x, un programme de 10h prend seulement **6 minutes** !
   - Vous voyez toutes les phases défiler

### Scénario 2 : Modifier les paramètres

1. **En mode OFF** (état initial)

2. **Naviguer** :
   - Flèches ← → pour sélectionner un paramètre
   - Le paramètre sélectionné est **surligné**

3. **Éditer** :
   - Appuyez sur **Entrée**
   - Le paramètre a un **cadre**

4. **Modifier** :
   - Flèches ← → pour changer la valeur
   - Exemple : Changez Step1Temp de 100°C à 150°C

5. **Valider** :
   - Appuyez sur **Entrée**
   - Les paramètres sont sauvegardés automatiquement

6. **Tester** :
   - Appuyez sur `S` pour lancer
   - Le four monte maintenant à 150°C pour la phase 1

## Raccourcis essentiels

| Touche | Action |
|--------|--------|
| **←** **→** | Rotation encodeur (navigation/édition) |
| **Entrée** | Clic encodeur (valider/éditer) |
| **S** | Start/Stop le programme |
| **+** **-** | Vitesse simulation |
| **0** | Vitesse normale (1x) |
| **R** | Reset du four |

## Comprendre l'écran OLED

### Mode OFF (configuration)
```
OFF                         # État
P1:50C/h >100C, 5m         # Phase 1
P2:250C/h >570C, 15m       # Phase 2
P3:200C/h >1100C, 20m      # Phase 3
Cool:150C/h <200C          # Refroidissement
T:20C                       # Température actuelle
OFF,0%                      # Relais et puissance
```

### Mode ON (en marche)
```
ON              02:15       # État + temps écoulé
P1:50>100,5m               # Phase 1 (grisé si terminé)
P2:250>570,15m             # Phase 2 (blanc si actif)
P3:200>1100,20m            # Phase 3 (grisé si futur)
Cool:150<200C              # Refroidissement
T:350->355C                # Actuel->Cible
ON,75%                     # Relais et puissance
```

## Exemples de tests

### Test 1 : Cycle rapide de biscuit
```
Phase 1: 50°C/h → 100°C, attente 5min
Phase 2: 250°C/h → 600°C, attente 10min
Phase 3: 200°C/h → 980°C, attente 15min
Refroidissement: 150°C/h → 200°C
```
**Durée réelle** : ~7h  
**À 100x** : ~4 minutes

### Test 2 : Cycle complet de grès
```
Phase 1: 50°C/h → 150°C, attente 30min
Phase 2: 150°C/h → 800°C, attente 15min
Phase 3: 100°C/h → 1280°C, attente 30min
Refroidissement: 100°C/h → 300°C
```
**Durée réelle** : ~18h  
**À 100x** : ~11 minutes

### Test 3 : Test rapide PID
```
Phase 1: 200°C/h → 100°C, attente 1min
(puis arrêt manuel avec S)
```
**But** : Observer la montée en température et le comportement du PID

## Que observer ?

### ✅ Comportement normal

1. **Montée progressive** : La température suit la consigne avec un léger retard (inertie)
2. **PID stable** : Pas d'oscillations importantes
3. **Puissance variable** : S'adapte automatiquement (0-100%)
4. **Plateaux maintenus** : Température stable pendant les paliers
5. **Refroidissement** : Le four peut chauffer légèrement pour ralentir la descente

### ⚠️ À surveiller

1. **Écart trop grand** : Si l'écart réel/consigne dépasse 50°C constamment
   - → Ajuster le PID (Kp, Ki dans config.py)
   
2. **Oscillations** : Si la température oscille beaucoup
   - → Réduire Kp
   
3. **Montée trop lente** : Si le four ne suit pas la consigne
   - → Augmenter MAX_HEATING_POWER dans config.py

## Dépannage express

### Problème : Fenêtre noire
**Solution** : C'est normal, attendez 1-2 secondes le chargement

### Problème : "ModuleNotFoundError: pygame"
**Solution** : 
```bash
pip3 install pygame matplotlib numpy
```

### Problème : Le four ne chauffe pas
**Solution** : Vérifiez que vous avez appuyé sur **S** pour démarrer

### Problème : C'est trop lent
**Solution** : Appuyez sur **+** pour accélérer (jusqu'à 100x)

### Problème : Je ne vois pas les modifications
**Solution** : Appuyez sur **Entrée** pour valider après modification

## Prochaines étapes

1. ✅ Tester tous les scénarios de cuisson
2. ✅ Ajuster les paramètres PID si nécessaire
3. ✅ Vérifier que toutes les phases fonctionnent
4. ✅ Tester les cas d'erreur (simulation de panne)
5. ✅ Transférer le code Arduino vers le vrai matériel

## Support

- **Documentation complète** : Voir `README.md`
- **Configuration** : Voir `config.py`
- **Code Arduino original** : Voir `../lucia/`

---

**Profitez du simulateur ! 🔥**

