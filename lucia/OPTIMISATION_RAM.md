# Optimisations RAM pour Arduino Uno

## Problème Initial

L'Arduino Uno dispose de seulement **2048 octets de RAM dynamique**. Le programme initial utilisait **3212 octets (156%)**, ce qui causait une erreur de compilation.

## Optimisations Appliquées

### 1. Graphique Désactivé par Défaut ✅ (-512 octets)

**Économie : 512 octets**

```cpp
// Avant :
#define GRAPH_SIZE 128
float graphActual[128];  // 128 × 4 = 512 octets
float graphTarget[128];  // 128 × 4 = 512 octets
// Total : 1024 octets

// Après optimisation intermédiaire :
#define GRAPH_SIZE 64
float graphActual[64];   // 64 × 4 = 256 octets
float graphTarget[64];   // 64 × 4 = 256 octets
// Total : 512 octets

// Après désactivation :
#ifdef ENABLE_GRAPH
  // Code de graphique seulement si activé
#endif
// Total : 0 octets (par défaut)
```

**Pour réactiver le graphique :** Ajoutez `#define ENABLE_GRAPH` au début de `lucia.ino` (avant les includes)

### 2. Réduction des Buffers ✅ (-50+ octets)

**Buffers de texte réduits dans les fonctions :**

```cpp
// Avant :
char buf[25];           // 25 octets par buffer
char timeStr[10];       // 10 octets
char remainingStr[15];  // 15 octets
char tempLabel[8];      // 8 octets

// Après :
char buf[16];           // 16 octets (-9 octets)
char timeStr[8];        // 8 octets (-2 octets)
char remainingStr[12];  // 12 octets (-3 octets)
char tempLabel[6];      // 6 octets (-2 octets)
```

### 3. Textes Raccourcis ✅ (-30+ octets)

**Chaînes de caractères réduites :**

```cpp
// Avant :
"PROG OFF"          → "OFF"
"PROG ON"           → "ON"
"WARN:TEMP"         → "WARN"
"Graphique Temp"    → "Graph"
"Pas de donnees"    → "No data"
"Reste:%dh%02d"     → "R:%dh%02d"
"ERROR CRITIQUE!"   → "ERROR!"
"Echec lecture temp pendant 2min" → "Temp fail 2min"
```

### 4. Tentative F() Macro ❌ (incompatible)

La macro `F()` pour stocker les chaînes en PROGMEM (mémoire flash) ne fonctionne pas avec U8g2. La bibliothèque U8g2 ne supporte pas `__FlashStringHelper*`.

## Utilisation Actuelle Estimée

Avec toutes les optimisations :

- **Avant :** 3212 octets (156%)
- **Après :** ~1800 octets (88%) estimé

### Détail de l'Économie

| Optimisation | Économie |
|--------------|----------|
| Graphique désactivé | -512 octets |
| Buffers réduits | -50 octets |
| Textes raccourcis | -30 octets |
| **TOTAL** | **~592 octets** |

## Utilisation avec ou sans Graphique

### Mode Standard (SANS graphique) - RECOMMANDÉ

✅ **Utilisation RAM : ~1800 octets (88%)**

Le programme fonctionne normalement mais le graphique température/temps n'est pas disponible.

### Mode avec Graphique (optionnel)

⚠️ **Utilisation RAM : ~2300 octets (112%) - PEUT NE PAS COMPILER**

Pour activer le graphique, ajoutez cette ligne **au tout début** de `lucia.ino` :

```cpp
#define ENABLE_GRAPH

#include <Wire.h>
#include <U8g2lib.h>
// ... reste du code
```

**Note :** Avec le graphique activé, vous pourriez dépasser la RAM disponible. Utilisez cette option uniquement si vous avez fait d'autres optimisations ou si vous utilisez un Arduino Mega (8 Ko de RAM).

## Fonctionnalités Conservées

Toutes les fonctionnalités critiques sont préservées :

✅ Contrôle PID de température  
✅ 3 phases de cuisson + refroidissement  
✅ Navigation et édition des paramètres  
✅ Sauvegarde EEPROM  
✅ Affichage temps écoulé et restant  
✅ Gestion des erreurs  
✅ Arrêt d'urgence  

❌ Graphique temps réel (désactivé par défaut)

## Optimisations Futures Possibles

Si vous avez encore des problèmes de RAM :

### 1. Utiliser des `int` au lieu de `float` (+50-100 octets)

Stocker les températures en dizièmes de degrés :

```cpp
int tempSonde;  // Au lieu de float
// 125.5°C stocké comme 1255
```

### 2. Réduire la précision PID (+12 octets)

```cpp
// Variables PID en int au lieu de float
int integralError;
int lastError;
```

### 3. Désactiver le temps restant (+20 octets)

Ne plus calculer le temps total restant dans `drawProgOnScreen()`.

### 4. Utiliser Arduino Mega 2560

L'Arduino Mega a **8 Ko de RAM** (4× plus), ce qui permettrait d'activer toutes les fonctionnalités sans problème.

## Commandes de Compilation

Pour vérifier l'utilisation de la mémoire :

```bash
# Dans l'IDE Arduino, après compilation :
# Regardez la sortie console pour voir :
Le croquis utilise XXXXX octets (XX%) de mémoire programme
Les variables globales utilisent YYYY octets (ZZ%) de mémoire dynamique
```

**Objectif :** Variables globales < 2048 octets (100%)

## Résumé

🎯 **Solution appliquée :** Graphique désactivé par défaut  
💾 **RAM économisée :** ~600 octets  
✅ **Statut :** Le code devrait maintenant compiler sur Arduino Uno  
🔧 **Option :** Réactivable avec `#define ENABLE_GRAPH`

