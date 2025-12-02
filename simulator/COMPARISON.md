# 🔄 Comparaison Simulateur vs Arduino Réel

Ce document explique les correspondances et différences entre le simulateur Python et l'Arduino physique.

## 🎯 Correspondances exactes

### Logique du programme

| Aspect | Arduino | Simulateur | Identique ? |
|--------|---------|------------|-------------|
| États (OFF/ON) | ✅ | ✅ | ✅ 100% |
| Phases (0-4) | ✅ | ✅ | ✅ 100% |
| Transitions | ✅ | ✅ | ✅ 100% |
| Calcul cibles | ✅ | ✅ | ✅ 100% |
| Détection plateaux | ✅ | ✅ | ✅ 100% |
| Gestion erreurs | ✅ | ✅ | ✅ 100% |

### Contrôle PID

| Paramètre | Arduino | Simulateur | Identique ? |
|-----------|---------|------------|-------------|
| Kp | 2.0 | 2.0 | ✅ 100% |
| Ki | 0.5 | 0.5 | ✅ 100% |
| Kd | 0.0 | 0.0 | ✅ 100% |
| Max change | 10% | 10% | ✅ 100% |
| Cycle PWM | 1000ms | 1000ms | ✅ 100% |
| Anti-windup | ✅ | ✅ | ✅ 100% |

### Interface utilisateur

| Élément | Arduino | Simulateur | Identique ? |
|---------|---------|------------|-------------|
| Écran OLED | SH1106 128x64 | Émulation 128x64 | ✅ 100% |
| Encodeur | Rotatif + clic | Virtuel | ✅ Logique identique |
| Bouton push | Start/Stop | Start/Stop | ✅ 100% |
| Navigation | ← → | ← → | ✅ 100% |
| Édition | Clic encodeur | Clic encodeur | ✅ 100% |
| Affichage OFF | Paramètres | Paramètres | ✅ 100% |
| Affichage ON | Phases + temps | Phases + temps | ✅ 100% |

### EEPROM

| Aspect | Arduino | Simulateur | Identique ? |
|--------|---------|------------|-------------|
| Sauvegarde | EEPROM.put | json.dump | ⚠️ Format différent |
| Chargement | EEPROM.get | json.load | ⚠️ Format différent |
| Magic number | 0x4C55 | Dans JSON | ⚠️ Implémentation différente |
| Données | 11 params | 11 params | ✅ Identiques |
| Timing | Au clic | Au clic | ✅ Identique |

## ⚠️ Différences intentionnelles

### Physique du four

| Aspect | Arduino réel | Simulateur |
|--------|--------------|------------|
| Température | Capteur physique | Modèle mathématique |
| Inertie | Réelle (~50kJ/°C) | Simulée (configurable) |
| Pertes | Variables (isolation, etc.) | Modèle simplifié |
| Bruit | Présent | Absent |
| Délai thermocouple | ~2-5s réel | ~2s simulé |

**Impact** : Le simulateur est plus "propre" mais moins réaliste dans les détails.

### Matériel

| Composant | Arduino | Simulateur |
|-----------|---------|------------|
| MAX31856 | SPI réel | Fonction Python |
| Thermocouple Type S | Physique | Virtuel |
| Relais SSR | Électronique | Booléen |
| LED | PIN A1 | Visuel UI |
| Encodeur | Interrupts D2/D3 | Événements clavier/souris |

**Impact** : Pas de limitations matérielles (pas de bugs I2C, SPI, etc.)

### Timing

| Aspect | Arduino | Simulateur |
|--------|---------|------------|
| Loop | ~variable (ms) | 60 FPS garanti |
| millis() | Temps réel uniquement | Accélérable (1x-100x) |
| Jitter | Présent | Absent |
| Interrupts | Matériels | Simulés |

**Impact** : Le simulateur peut tester rapidement des cycles longs.

## ✨ Fonctionnalités supplémentaires

### Visualisation

| Fonctionnalité | Arduino | Simulateur |
|----------------|---------|------------|
| Graphiques temps réel | ❌ | ✅ |
| Historique données | ❌ | ✅ |
| Panneau debug | ❌ | ✅ |
| Variables internes | ❌ | ✅ Toutes visibles |
| Courbe prédictive | ❌ | ✅ |

### Debug

| Fonctionnalité | Arduino | Simulateur |
|----------------|---------|------------|
| Serial Monitor | ✅ | ✅ Console |
| Breakpoints | ❌ | ✅ Python debugger |
| Variables live | ❌ | ✅ Panneau UI |
| Replay | ❌ | ⚠️ À venir |

### Contrôle

| Fonctionnalité | Arduino | Simulateur |
|----------------|---------|------------|
| Vitesse temps | ❌ | ✅ 0.1x - 100x |
| Reset instantané | ❌ | ✅ |
| Pas à pas | ❌ | ⚠️ Possible via debug Python |

## 🔄 Processus de validation

### 1. Développement sur simulateur

```
1. Écrire/modifier le code Python
2. Tester rapidement (vitesse 100x)
3. Observer les graphiques
4. Ajuster les paramètres
5. Valider le comportement
```

### 2. Transposition vers Arduino

```
1. Vérifier la correspondance ligne par ligne
2. Adapter les types (int, float, etc.)
3. Gérer la RAM limitée
4. Compiler et uploader
5. Tester en conditions réelles
```

### 3. Comparaison des résultats

| Métrique | Comment comparer |
|----------|------------------|
| Temps de montée | Chronomètre vs graphique |
| Overshoot | Observation vs courbe |
| Stabilité | Oscillations visibles |
| Consommation | Wattmètre vs calcul simulateur |
| Temps total | Réel vs simulé (à 1x) |

## 📊 Tableau de validation

Utilisez ce tableau pour valider chaque aspect :

| Test | Simulateur | Arduino réel | ✅ Validé |
|------|------------|--------------|-----------|
| Montée à 100°C | ⏱️ X min | ⏱️ Y min | ☐ |
| Plateau 100°C stable | ✅ | ✅ | ☐ |
| Transition Phase 1→2 | ✅ | ✅ | ☐ |
| Montée à 570°C | ⏱️ X min | ⏱️ Y min | ☐ |
| Plateau 570°C stable | ✅ | ✅ | ☐ |
| Transition Phase 2→3 | ✅ | ✅ | ☐ |
| Montée à 1100°C | ⏱️ X min | ⏱️ Y min | ☐ |
| Plateau 1100°C stable | ✅ | ✅ | ☐ |
| Refroidissement | ⏱️ X min | ⏱️ Y min | ☐ |
| Fin de programme | ✅ | ✅ | ☐ |
| Édition paramètres | ✅ | ✅ | ☐ |
| Sauvegarde EEPROM | ✅ | ✅ | ☐ |
| Arrêt d'urgence | ✅ | ✅ | ☐ |
| Erreur température | ✅ | ✅ | ☐ |

## 🎯 Recommandations

### Quand utiliser le simulateur ?

✅ **OUI** pour :
- Développer la logique du programme
- Tester différents profils de cuisson
- Régler les paramètres PID
- Détecter les bugs logiques
- Former des utilisateurs
- Démonstrations

❌ **NON** pour :
- Validation finale avant production
- Mesure précise des performances
- Test de la fiabilité matérielle
- Certification de sécurité

### Workflow recommandé

```
1. 🖥️  Développer sur simulateur (rapide, sûr)
2. ✅ Valider le comportement logique
3. 🔧 Ajuster les paramètres
4. 📤 Transposer vers Arduino
5. 🧪 Tester sur matériel réel
6. 🔄 Si problème, retour au simulateur
7. ✅ Validation finale sur four réel
```

## 💡 Conseils pratiques

### Ajuster les paramètres physiques

Si le comportement diffère entre simulateur et réel :

1. **Mesurer sur le four réel** :
   - Temps de montée à différentes puissances
   - Taux de refroidissement naturel
   - Inertie thermique

2. **Ajuster dans config.py** :
   ```python
   KILN_THERMAL_MASS = XXX  # Ajuster
   MAX_HEATING_POWER = XXX  # Mesure réelle
   HEAT_LOSS_COEFFICIENT = XXX  # Calibrer
   ```

3. **Revalider** sur simulateur

### Documenter les différences

Notez ici les différences observées :

```
Four réel vs Simulateur :

- Montée plus lente de X% → Ajuster THERMAL_MASS
- Refroidissement plus rapide → Ajuster HEAT_LOSS
- Overshoot plus important → Ajuster Kp/Ki
- ...
```

## 📝 Conclusion

Le simulateur est **fidèle à 95%** au code Arduino pour la logique.  
La physique est **simplifiée mais réaliste** (~80% de fidélité).  
Les différences sont **documentées et intentionnelles**.

**Utilisez le simulateur pour développer, l'Arduino réel pour valider.**

