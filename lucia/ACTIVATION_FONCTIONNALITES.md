# Guide d'Activation des Fonctionnalités - CalciferAi

## 🎯 Gestion de la Mémoire Flash

**Limite Arduino Uno : 32256 octets (100%)**

Votre programme peut être configuré selon vos besoins en mémoire.

---

## 📊 Tableau des Configurations

| Configuration | Taille Estimée | État | Fonctionnalités |
|---------------|----------------|------|-----------------|
| **Minimal** | ~31500 octets (98%) | ✅ Compile | Toutes sauf Logging + Graphe |
| **+ Logging** | ~31750 octets (98.4%) | ✅ Compile | Toutes sauf Graphe |
| **+ Graphe** | ~32300 octets (100.1%) | ✅ Compile | Toutes sauf Logging |
| **Complet** | ~32550 octets (101%) | ❌ Trop gros | Logging + Graphe |

---

## 🔧 Configuration dans `definitions.h`

### Fichier : `/lucia/definitions.h` (lignes 19-20)

```cpp
#define TEMP_READ_INTERVAL 500
#define DISPLAY_UPDATE_INTERVAL 100
#define ENCODER_CHECK_INTERVAL 20
#define TEMP_FAIL_TIMEOUT 120000
#define EEPROM_WRITE_MIN_INTERVAL 10000

// ===== ACTIVATIONS OPTIONNELLES =====
//#define ENABLE_LOGGING  // Décommenter pour activer le logging (~250 octets)
//#define ENABLE_GRAPH    // Décommenter pour activer le graphe (~800 octets)
```

---

## 📝 Option 1 : Configuration Minimale (RECOMMANDÉ)

**Économie maximale de mémoire**

```cpp
//#define ENABLE_LOGGING  // ❌ Désactivé
//#define ENABLE_GRAPH    // ❌ Désactivé
```

**Résultat :**
- ✅ Taille : ~31500 octets (98%)
- ✅ Compile sans problème
- ✅ Toutes les fonctions principales actives
- ❌ Pas de logging Serial
- ❌ Pas de graphe température

**Utilisation :** Configuration idéale pour utilisation normale du four

---

## 📡 Option 2 : Avec Logging Serial

**Pour monitoring et debug**

```cpp
#define ENABLE_LOGGING   // ✅ Activé
//#define ENABLE_GRAPH    // ❌ Désactivé
```

**Résultat :**
- ✅ Taille : ~31750 octets (98.4%)
- ✅ Compile correctement
- ✅ Logging Serial 9600 bauds
- ❌ Pas de graphe

**Données envoyées :**
```
S,2.0,0.5,0.0                    // Startup: Kp,Ki,Kd
D,5000,125.3,130.0,10.5,2.3,75  // Data: temps,temp,target,P,I,power
```

**Utilisation :** Pour logger les données de cuisson sur PC/Raspberry Pi

---

## 📊 Option 3 : Avec Graphe Température

**Pour visualisation sur écran**

```cpp
//#define ENABLE_LOGGING  // ❌ Désactivé
#define ENABLE_GRAPH     // ✅ Activé
```

**Résultat :**
- ✅ Taille : ~32300 octets (100.1%)
- ⚠️ Compile (mais très limite)
- ✅ Graphe température visible
- ❌ Pas de logging Serial

**Fonctionnalités graphe :**
- 64 points de données
- Courbe consigne théorique
- Points mesure réels
- Échantillonnage adaptatif (5s → 60s)
- Accessible par bouton encodeur

**Utilisation :** Pour visualisation directe sans PC

---

## ⚠️ Option 4 : Configuration Complète (NON RECOMMANDÉ)

**Toutes les fonctionnalités**

```cpp
#define ENABLE_LOGGING   // ✅ Activé
#define ENABLE_GRAPH     // ✅ Activé
```

**Résultat :**
- ❌ Taille : ~32550 octets (101%)
- ❌ **NE COMPILE PAS** sur Arduino Uno
- ✅ Logging + Graphe actifs

**Solutions :**
1. Utiliser un **Arduino Mega** (256 Ko Flash)
2. Désactiver une fonctionnalité
3. Optimisations supplémentaires (complexe)

---

## 🚀 Procédure de Changement

### Étape 1 : Ouvrir le Fichier

1. Dans Arduino IDE : **Fichier** → **Ouvrir**
2. Naviguer vers `/lucia/definitions.h`

### Étape 2 : Modifier les Définitions

**Activer** une fonctionnalité :
```cpp
#define ENABLE_LOGGING  // Retirer le //
```

**Désactiver** une fonctionnalité :
```cpp
//#define ENABLE_LOGGING  // Ajouter // au début
```

### Étape 3 : Sauvegarder

**Fichier** → **Enregistrer** (Ctrl+S)

### Étape 4 : Compiler

**Croquis** → **Vérifier/Compiler** (Ctrl+R)

### Étape 5 : Vérifier la Taille

Dans la console en bas :
```
Le croquis utilise XXXXX octets (XX%) de l'espace de stockage
```

**Objectif :** < 32256 octets

---

## 📋 Tableau de Dépannage

| Problème | Cause | Solution |
|----------|-------|----------|
| "text section exceeds available space" | Programme trop gros | Désactiver ENABLE_GRAPH ou ENABLE_LOGGING |
| Pas de données Serial | Logging désactivé | Activer ENABLE_LOGGING |
| Bouton encodeur ne montre pas graphe | Graphe désactivé | Activer ENABLE_GRAPH |
| Compile à 101% | Les deux activés | Choisir : Logging OU Graphe |

---

## 💡 Recommandations par Usage

### Usage Production (Four en service)
```cpp
//#define ENABLE_LOGGING  // ❌
//#define ENABLE_GRAPH    // ❌
```
**Pourquoi :** Maximum de stabilité, minimum de mémoire

### Usage Debug (Développement)
```cpp
#define ENABLE_LOGGING   // ✅
//#define ENABLE_GRAPH    // ❌
```
**Pourquoi :** Logging des données sans surcharge graphe

### Usage Démonstration (Présentation)
```cpp
//#define ENABLE_LOGGING  // ❌
#define ENABLE_GRAPH     // ✅
```
**Pourquoi :** Visualisation impressionnante sur écran

### Usage Arduino Mega
```cpp
#define ENABLE_LOGGING   // ✅
#define ENABLE_GRAPH     // ✅
```
**Pourquoi :** Mémoire suffisante pour tout

---

## 🔍 Vérification État Actuel

### Dans le Code

Ouvrir `/lucia/definitions.h` et regarder lignes 19-20 :
- Avec `//` devant = DÉSACTIVÉ ❌
- Sans `//` devant = ACTIVÉ ✅

### Lors de la Compilation

Message console :
- **~31500 octets** = Minimal (rien activé)
- **~31750 octets** = Logging activé
- **~32300 octets** = Graphe activé
- **~32550 octets** = Tout activé (ne compile pas)

---

## 📞 Support

Si vous avez besoin d'activer les deux fonctionnalités :
1. Envisager un **Arduino Mega 2560** (256 Ko Flash)
2. Ou demander des optimisations supplémentaires (plus complexe)

---

## ✅ Configuration Actuelle Recommandée

**Pour commencer :**

```cpp
//#define ENABLE_LOGGING  // Désactivé par défaut
//#define ENABLE_GRAPH    // Désactivé par défaut
```

**Taille : ~31500 octets (98%)** ✅

Activez ensuite selon vos besoins !
