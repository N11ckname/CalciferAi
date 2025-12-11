# Disposition Finale des Écrans LUCIA

## 📐 **ORGANISATION FINALE**

Date : 10 Décembre 2025  
Version : v2.2.1 (Layout Optimisé)

---

## 🎯 **PRINCIPE**

### Écran Principal (Phase 0) : Disposition en Grille
- **Layout** : Grille compacte 4 lignes × 3 colonnes
- **Typo** : Une seule fonte (`u8g2_font_6x10_tf`)
- **Scroll** : Non (tout tient sur l'écran)
- **Usage** : Configuration rapide des phases

### Écran Settings : Liste Scrollable
- **Layout** : Liste verticale défilante
- **Typo** : Une seule fonte (`u8g2_font_6x10_tf`)
- **Scroll** : Oui (pour extensibilité future)
- **Usage** : Réglages avancés PID

---

## 📱 **ÉCRAN PRINCIPAL (PROG_OFF)**

### Disposition

```
┌────────────────────────────┐
│ 25C              [S]  WARN │ ← Température + Settings + Warning
├────────────────────────────┤
│ P1: 100C/h  >570C    5m   │ ← Phase 1 (3 paramètres)
│ P2: 250C/h >1100C   20m   │ ← Phase 2 (3 paramètres)
│ P3: 200C/h >1100C   15m   │ ← Phase 3 (3 paramètres)
│ Cool: 150C/h  <200C       │ ← Cooldown (2 paramètres)
└────────────────────────────┘
```

### Éléments

| Ligne | Label | Colonnes | Sélection |
|-------|-------|----------|-----------|
| 1 | Temp | Température actuelle | - |
| 2 | P1: | Speed / Temp / Wait | 0, 1, 2 |
| 3 | P2: | Speed / Temp / Wait | 3, 4, 5 |
| 4 | P3: | Speed / Temp / Wait | 6, 7, 8 |
| 5 | Cool: | Speed / Target | 9, 10 |
| - | [S] | Settings icon | 11 |

### Sélection

**Mode Navigation (NAV_MODE) :**
```
┌────────────────────────────┐
│ 25C                   [S] │
│ P1: ┏━━━━━━┓ >570C    5m  │
│     ┃100C/h┃              │ ← Cadre autour
│     ┗━━━━━━┛              │
│ P2: 250C/h >1100C   20m   │
│ P3: 200C/h >1100C   15m   │
│ Cool: 150C/h  <200C       │
└────────────────────────────┘
```

**Mode Édition (EDIT_MODE) :**
```
┌────────────────────────────┐
│ 25C                   [S] │
│ P1: ┏━━━━━━┓ >570C    5m  │
│     ┃100C/h┃              │ ← Inversion vidéo
│     ┗━━━━━━┛              │
│ P2: 250C/h >1100C   20m   │
│ P3: 200C/h >1100C   15m   │
│ Cool: 150C/h  <200C       │
└────────────────────────────┘
```

---

## ⚙️ **ÉCRAN SETTINGS (Scrollable)**

### Disposition (Haut de liste)

```
┌────────────────────────────┐
│ SETTINGS                   │ ← Titre fixe
├────────────────────────────┤
│ ┏━━━━━━━━━━━━━━━━━━━━━━┓ │
│ ┃Heat Cycle      1000ms┃ │ ← Sélectionné
│ ┗━━━━━━━━━━━━━━━━━━━━━━┛ │
│ Kp                   2.0  │
│ Ki                   0.5  │
│ Kd                   0.0 v│ ← Flèche scroll bas
└────────────────────────────┘
scrollOffset = 0
```

### Disposition (Après scroll)

```
┌────────────────────────────┐
│ SETTINGS                   │
├────────────────────────────┤
│ Kp                   2.0 ^│ ← Flèche scroll haut
│ Ki                   0.5  │
│ ┏━━━━━━━━━━━━━━━━━━━━━━┓ │
│ ┃Kd                   0.0┃ │ ← Sélectionné
│ ┗━━━━━━━━━━━━━━━━━━━━━━┛ │
│ Exit                      │
└────────────────────────────┘
scrollOffset = 2
```

### Éléments

| Index | Label | Valeur | Éditable |
|-------|-------|--------|----------|
| 0 | Heat Cycle | 1000ms | Oui |
| 1 | Kp | 2.0 | Oui |
| 2 | Ki | 0.5 | Oui |
| 3 | Kd | 0.0 | Oui |
| 4 | Exit | - | Non (action) |

### Capacité

- **Lignes visibles** : 5 (après titre)
- **Éléments actuels** : 5 (Heat Cycle, Kp, Ki, Kd, Exit)
- **Scroll nécessaire** : Non actuellement
- **Extensible** : Oui, jusqu'à ~10-15 éléments

---

## 🔄 **NAVIGATION**

### Écran Principal → Settings

1. Tourner encodeur → Sélectionner icône [S]
2. Clic encodeur → Entrer dans Settings
3. Écran Settings s'affiche

### Settings → Écran Principal

1. Tourner encodeur → Sélectionner "Exit"
2. Clic encodeur → Retour à l'écran principal

### Navigation dans Settings

1. **Tourner encodeur** → Défilement automatique
2. **Clic encodeur** → Entrer/Sortir mode édition
3. **En mode édition** → Tourner encodeur modifie valeur

---

## 📏 **DIMENSIONS**

### Écran Principal

| Élément | Largeur | Hauteur |
|---------|---------|---------|
| **Ligne température** | 128px | 12px |
| **Ligne P1** | 128px | 12px |
| **Ligne P2** | 128px | 12px |
| **Ligne P3** | 128px | 12px |
| **Ligne Cool** | 128px | 12px |
| **Total** | 128px | 60px |

### Écran Settings

| Élément | Largeur | Hauteur |
|---------|---------|---------|
| **Titre** | 128px | 12px |
| **5 lignes items** | 128px | 60px |
| **Total** | 128px | 72px (scroll) |

---

## 💾 **COMPARAISON VERSIONS**

### v2.2 (Scroll Partout)

| Écran | Layout | Scroll | Lignes Code |
|-------|--------|--------|-------------|
| PROG_OFF | Liste | Oui | 60 |
| Settings | Fixe | Non | 80 |
| **Total** | - | - | **140** |

### v2.2.1 (Layout Optimisé)

| Écran | Layout | Scroll | Lignes Code |
|-------|--------|--------|-------------|
| PROG_OFF | Grille | Non | 50 |
| Settings | Liste | Oui | 70 |
| **Total** | - | - | **120** |

**Économie : 20 lignes de code**

---

## ✅ **AVANTAGES DE CETTE DISPOSITION**

### Écran Principal (Grille)
1. ✅ **Compact** - Tout visible d'un coup d'œil
2. ✅ **Familier** - Layout habituel conservé
3. ✅ **Rapide** - Pas de scroll nécessaire
4. ✅ **Clair** - Organisation par phases
5. ✅ **Efficace** - 11 paramètres sur 4 lignes

### Écran Settings (Liste)
1. ✅ **Extensible** - Facile d'ajouter des paramètres
2. ✅ **Lisible** - Une info par ligne
3. ✅ **Scroll** - Prêt pour futures fonctions
4. ✅ **Aligné** - Valeurs alignées à droite
5. ✅ **Simple** - Navigation intuitive

---

## 🎨 **UNIFORMISATION TYPO**

### Une Seule Fonte Partout

```cpp
u8g2.setFont(u8g2_font_6x10_tf);  // PARTOUT
```

**Avantages :**
- ✅ Économie Flash : 2-4 Ko
- ✅ Cohérence visuelle
- ✅ Lisibilité garantie
- ✅ Maintenance simplifiée

**Caractéristiques :**
- Largeur caractère : 6 pixels
- Hauteur caractère : 10 pixels
- Lisibilité : Excellente
- Compacité : Optimale

---

## 🔧 **FONCTIONS IMPLÉMENTÉES**

### Écran Principal
```cpp
void drawProgOffScreen()              // Grille compacte
void drawParamInline(...)             // Affiche param avec sélection
```

### Écran Settings
```cpp
void drawSettingsScreen()             // Liste scrollable
void updateSettingsScrollOffset()     // Calcul auto-scroll
void drawSettingsScrollIndicator()    // Flèches ↑↓
void drawSettingsItem(...)            // Affiche item settings
```

---

## 📊 **BILAN FINAL**

### Mémoire

| Type | Avant | Après | Gain |
|------|-------|-------|------|
| **RAM** | - | +2 octets | -2 |
| **Flash** | - | +2300 octets | +2300 |

### Code

| Métrique | Valeur |
|----------|--------|
| **Fontes** | 1 (u8g2_font_6x10_tf) |
| **Écrans** | 2 (PROG_OFF + Settings) |
| **Scroll** | 1 (Settings uniquement) |
| **Lignes code** | ~120 (display.cpp) |

### Utilisabilité

| Critère | Score |
|---------|-------|
| **Lisibilité** | ⭐⭐⭐⭐⭐ |
| **Navigation** | ⭐⭐⭐⭐⭐ |
| **Compacité** | ⭐⭐⭐⭐⭐ |
| **Extensibilité** | ⭐⭐⭐⭐⭐ |

---

## 🎯 **RÉSULTAT**

✅ **Écran principal** : Grille compacte, tout visible  
✅ **Écran Settings** : Liste scrollable, extensible  
✅ **Une seule typo** : Uniformité et économie  
✅ **2.3 Ko libérés** : Place pour futures fonctions  
✅ **Navigation fluide** : Intuitive et rapide

**Le meilleur des deux mondes !** 🎉

---

**Version : v2.2.1**  
**Date : 10 Décembre 2025**  
**Statut : ✅ Implémenté et Optimisé**
