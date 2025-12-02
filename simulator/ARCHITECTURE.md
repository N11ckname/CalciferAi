# 🏗️ Architecture du Simulateur CalciferAi

## Vue d'ensemble

Le simulateur est une transposition fidèle du code Arduino en Python avec ajout de fonctionnalités de visualisation et de debug.

## Structure modulaire

```
┌─────────────────────────────────────────────────────────┐
│                       main.py                            │
│              (Point d'entrée et orchestration)          │
└──────────────┬──────────────────────────────────────────┘
               │
       ┌───────┴────────┐
       │                │
       ▼                ▼
┌─────────────┐  ┌─────────────┐
│ arduino_core│  │     ui.py   │
│   (Logique) │  │  (Interface)│
└──────┬──────┘  └──────┬──────┘
       │                │
   ┌───┴────┬──────────┴────────┐
   │        │                   │
   ▼        ▼                   ▼
┌──────┐ ┌─────────┐      ┌─────────┐
│temp  │ │kiln_    │      │display  │
│ctrl  │ │physics  │      │renderer │
└──────┘ └─────────┘      └─────────┘
```

## Modules détaillés

### 1. main.py
**Rôle** : Chef d'orchestre

**Responsabilités** :
- Initialiser tous les composants
- Gérer la boucle principale
- Calculer le delta time et appliquer la vitesse de simulation
- Dispatcher les événements utilisateur
- Coordonner le rendu

**Flux d'exécution** :
```python
while running:
    dt = calcul_delta_time() * simulation_speed
    actions = ui.handle_events()
    arduino.loop(dt)
    render()
```

### 2. arduino_core.py
**Rôle** : Cerveau du système (logique Arduino)

**Transposition de** : `lucia.ino`

**Responsabilités** :
- Gestion de l'état du programme (PROG_OFF / PROG_ON)
- Navigation et édition des paramètres
- Calcul des températures cibles
- Transition entre les phases
- Sauvegarde/chargement EEPROM virtuelle

**Variables d'état principales** :
```python
prog_state          # PROG_OFF / PROG_ON
current_phase       # PHASE_0 à PHASE_4_COOLDOWN
params              # Dict des paramètres de cuisson
target_temp         # Température cible calculée
plateau_reached     # Booléen pour gérer les paliers
```

**Méthodes importantes** :
- `loop(dt)` : Boucle principale (équivalent de `loop()`)
- `update_program()` : Met à jour les phases
- `calculate_target_temp()` : Calcule la consigne
- `check_phase_complete()` : Détecte fin de phase

### 3. temperature.py
**Rôle** : Contrôle de température

**Transposition de** : `temperature.cpp` / `temperature.h`

**Responsabilités** :
- Implémentation du PID (Proportionnel-Intégral)
- PWM logiciel (cycle 1 seconde)
- Anti-windup de l'intégrale
- Limitation du taux de changement

**Algorithme PID** :
```python
error = target - current
proportional = Kp * error
integral += Ki * error * dt
output = proportional + integral
output = limit_rate_of_change(output)
output = constrain(output, 0, 100)
```

**PWM logiciel** :
```python
cycle_time = time % CYCLE_LENGTH
on_time = (output / 100) * CYCLE_LENGTH
relay = (cycle_time < on_time)
```

### 4. kiln_physics.py
**Rôle** : Simulation physique réaliste

**Modèle thermique** :
```
dT/dt = (P_heating - P_loss) / thermal_mass

où:
- P_heating = power% * MAX_POWER
- P_loss = k * (T_kiln - T_ambient) + radiation_loss
- thermal_mass = capacité thermique du four
```

**Pertes thermiques** :
- **Conduction/Convection** : Proportionnelle à (T - T_ambient)
- **Radiation** : Proportionnelle à T^4 (loi de Stefan-Boltzmann simplifiée)

**Thermocouple virtuel** :
- Buffer circulaire pour simuler le délai de réponse
- Lissage par moyenne mobile
- Délai typique : 2 secondes

### 5. display.py
**Rôle** : Rendu de l'écran OLED

**Transposition de** : `display.cpp` / `display.h`

**Émulation pixel par pixel** :
- Buffer 128x64 (True/False pour chaque pixel)
- Police bitmap 5x7 pour les caractères
- Primitives graphiques : pixel, ligne, rectangle, texte

**Méthodes de rendu** :
- `draw_prog_off_screen()` : Écran de configuration
- `draw_prog_on_screen()` : Écran d'exécution
- `draw_critical_error()` : Écran d'erreur

**Différences avec Arduino** :
- Pas de mode 2-page (pas de contrainte RAM)
- Pas de vraies couleurs (monochrome)
- Rendu instantané vers Pygame

### 6. ui.py
**Rôle** : Interface utilisateur graphique

**Composants** :
1. **Affichage OLED agrandi** (x6)
2. **Boutons de contrôle** virtuels
3. **Graphiques temps réel** (matplotlib)
4. **Panneau debug** avec infos système

**Gestion des événements** :
- Souris : Clics sur boutons
- Clavier : Raccourcis
- Retourne un dict d'actions à `main.py`

**Graphiques matplotlib** :
- Courbe température (réelle vs cible)
- Courbe puissance de chauffage
- Mise à jour dynamique via FigureCanvasAgg

### 7. config.py
**Rôle** : Configuration centralisée

**Paramètres physiques** :
- Masse thermique, puissance, pertes
- Constantes PID
- Dimensions écran

**Paramètres UI** :
- Tailles fenêtre
- Couleurs
- Vitesse simulation

## Flux de données

### Boucle principale (60 FPS)

```
1. Événements utilisateur
   └→ ui.handle_events()
      └→ actions dict

2. Traitement Arduino
   └→ arduino.loop(dt)
      ├→ temp_controller.update()
      │  └→ (relay_state, power%)
      └→ kiln.update(dt, power, relay)
         └→ nouvelle température

3. Rendu
   └→ display_renderer.draw_xxx()
      └→ oled_buffer
         └→ ui.draw()
            ├→ OLED agrandi
            ├→ Graphiques
            └→ Panneau debug
```

### Cycle de température

```
1. arduino calcule target_temp
   └→ Basé sur phase, vitesse, temps écoulé

2. temp_controller reçoit (current, target)
   └→ Calcule PID
      └→ Détermine power% et relay ON/OFF

3. kiln reçoit (power%, relay)
   └→ Simule physique
      └→ Nouvelle temperature

4. Boucle...
```

## Correspondance Arduino ↔ Python

| Arduino | Python | Notes |
|---------|--------|-------|
| `setup()` | `arduino.setup()` | Initialisation |
| `loop()` | `arduino.loop(dt)` | Boucle principale |
| `millis()` | `time.time()` | Temps en secondes |
| `delay()` | ❌ Jamais utilisé | Non-bloquant |
| `EEPROM.get/put` | `json.load/dump` | Fichier JSON |
| `digitalRead()` | Actions dict | Événements UI |
| `digitalWrite()` | `kiln.relay_state` | Simulation |
| `max31856.readTemp()` | `kiln.get_temperature()` | Physique |

## Timing et performance

### Fréquence de mise à jour

| Composant | Fréquence | Notes |
|-----------|-----------|-------|
| Boucle principale | 60 Hz | Limite Pygame |
| Physique four | Variable | Selon dt réel |
| PID | Variable | Selon dt réel |
| Affichage OLED | Chaque frame | Pas de délai |
| Graphiques | Chaque frame | Coûteux en CPU |

### Simulation accélérée

Le facteur de vitesse (`simulation_speed`) multiplie uniquement `dt` :
- `dt_sim = dt_real * speed`
- La physique et le PID utilisent `dt_sim`
- Le rendu reste à 60 FPS réel

**Exemple à 100x** :
- 1 seconde réelle = 100 secondes simulées
- 1 minute réelle = 1h40 simulée
- 6 minutes réelles = 10h simulées

## Différences avec le matériel réel

### Avantages du simulateur
✅ **Visualisation** : Graphiques temps réel  
✅ **Debug** : Toutes les variables visibles  
✅ **Vitesse** : Test rapide des cycles longs  
✅ **Sécurité** : Aucun risque matériel  
✅ **Reproductibilité** : Conditions identiques

### Limitations
⚠️ **Physique simplifiée** : Modèle 0D (pas de gradient spatial)  
⚠️ **Pas de bruit** : Mesures parfaites  
⚠️ **Pas de défaillances** : Relais parfait, alimentation stable  
⚠️ **Pas de CEM** : Pas d'interférences électromagnétiques

## Extension du simulateur

### Ajouter un nouveau graphique

```python
# Dans ui.py
def draw_custom_graph(self, state):
    fig, ax = plt.subplots()
    ax.plot(state['custom_data'])
    # ... rendu
```

### Modifier la physique

```python
# Dans kiln_physics.py
def update(self, dt, power, relay):
    # Ajouter de nouveaux effets physiques
    heat_from_exothermic = calculate_clay_reaction()
    net_heat += heat_from_exothermic
```

### Ajouter un paramètre

1. Ajouter dans `DEFAULT_FIRING_PARAMS` (config.py)
2. Ajouter dans `edit_parameter()` (arduino_core.py)
3. Ajouter dans le rendu OLED (display.py)

## Tests et validation

### Tests unitaires suggérés

1. **PID** : Réponse indicielle, stabilité
2. **Phases** : Transitions correctes
3. **EEPROM** : Sauvegarde/chargement
4. **Physique** : Conservation de l'énergie

### Validation du comportement

Comparer avec le comportement attendu :
- Temps de montée
- Dépassement (overshoot)
- Temps de stabilisation
- Consommation d'énergie

## Performance

### Optimisations possibles

1. **Graphiques** : Ne redessiner que si nécessaire
2. **OLED** : Détection de changements uniquement
3. **Physique** : Pas de temps adaptatif
4. **Historique** : Limiter la taille des buffers

### Profiling

```bash
python3 -m cProfile -o profile.stats main.py
python3 -m pstats profile.stats
```

## Maintenance

### Points d'attention

1. **Synchronisation avec Arduino** : Maintenir la correspondance
2. **Versions des libs** : Tester avec différentes versions
3. **Portabilité** : macOS, Linux, Windows
4. **Documentation** : Garder à jour avec le code

---

**Architecture conçue pour être simple, extensible et fidèle à l'Arduino original.**

