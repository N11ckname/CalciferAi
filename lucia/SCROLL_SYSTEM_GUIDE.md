# Système de Défilement pour OLED 128x64

## 🎯 Objectif

Afficher une liste d'éléments plus longue que l'écran en permettant le défilement avec l'encodeur rotatif.

---

## 📐 Contraintes de l'Écran

### Dimensions OLED SH1106
- **Largeur** : 128 pixels
- **Hauteur** : 64 pixels

### Capacité d'Affichage par Fonte

| Fonte | Hauteur ligne | Lignes visibles | Usage |
|-------|---------------|-----------------|-------|
| `u8g2_font_9x15_tf` | ~15px | **4 lignes** | Grande (température) |
| `u8g2_font_6x10_tf` | ~10px | **6 lignes** | Moyenne (standard) |
| `u8g2_font_5x7_tf` | ~8px | **8 lignes** | Petite (détails) |

**→ Si vous avez 11 paramètres et 6 lignes visibles, il faut un défilement !**

---

## 💡 Principe du Défilement

### Concept de "Fenêtre Glissante"

```
Liste complète (11 éléments) :     Écran (6 lignes visibles) :
┌─────────────────────┐
│ [0] P1 Speed        │ ←─┐       ┌─────────────────────┐
│ [1] P1 Temp         │   │       │ [0] P1 Speed        │
│ [2] P1 Wait         │   │       │ [1] P1 Temp         │
│ [3] P2 Speed        │   │       │ [2] P1 Wait         │
│ [4] P2 Temp         │   ├──────→│ [3] P2 Speed        │
│ [5] P2 Wait         │   │       │ [4] P2 Temp         │
│ [6] P3 Speed        │   │       │ [5] P2 Wait         │
│ [7] P3 Temp         │ ←─┘       └─────────────────────┘
│ [8] P3 Wait         │            scrollOffset = 0
│ [9] Cool Speed      │
│ [10] Cool Target    │
│ [11] Settings       │
└─────────────────────┘

Après scroll vers le bas (offset = 3) :

                            ┌─────────────────────┐
                            │ [3] P2 Speed        │
                            │ [4] P2 Temp         │
                            │ [5] P2 Wait         │
                            │ [6] P3 Speed        │
                            │ [7] P3 Temp         │
                            │ [8] P3 Wait         │
                            └─────────────────────┘
                             scrollOffset = 3
```

---

## 🔧 Implémentation Simple

### 1. Variables Nécessaires

```cpp
// Dans lucia.ino (section variables globales)
int scrollOffset = 0;        // Premier élément visible à l'écran
const int ITEMS_PER_PAGE = 6; // Nombre de lignes visibles
const int TOTAL_ITEMS = 12;   // Nombre total d'éléments (11 params + Settings)
```

### 2. Fonction de Calcul du Défilement

```cpp
// Calculer automatiquement l'offset pour garder l'élément sélectionné visible
void updateScrollOffset() {
  // Si l'élément sélectionné est au-dessus de la fenêtre visible
  if (selectedParam < scrollOffset) {
    scrollOffset = selectedParam;
  }
  
  // Si l'élément sélectionné est en dessous de la fenêtre visible
  if (selectedParam >= scrollOffset + ITEMS_PER_PAGE) {
    scrollOffset = selectedParam - ITEMS_PER_PAGE + 1;
  }
  
  // Contraindre l'offset dans les limites
  if (scrollOffset < 0) {
    scrollOffset = 0;
  }
  
  int maxOffset = TOTAL_ITEMS - ITEMS_PER_PAGE;
  if (scrollOffset > maxOffset && maxOffset >= 0) {
    scrollOffset = maxOffset;
  }
}
```

### 3. Fonction d'Affichage avec Défilement

```cpp
void drawScrollableList() {
  u8g2.setFont(u8g2_font_6x10_tf);
  
  // Calculer le défilement automatiquement
  updateScrollOffset();
  
  // Afficher uniquement les éléments visibles
  int y = 10; // Position Y de départ
  int lineHeight = 10; // Hauteur de chaque ligne
  
  for (int i = 0; i < ITEMS_PER_PAGE; i++) {
    int itemIndex = scrollOffset + i;
    
    // Vérifier si l'élément existe
    if (itemIndex >= TOTAL_ITEMS) break;
    
    // Afficher l'élément avec effet de sélection
    bool isSelected = (itemIndex == selectedParam);
    
    // Position Y pour cet élément
    int itemY = y + (i * lineHeight);
    
    // Dessiner avec effet de sélection
    if (isSelected && editMode == NAV_MODE) {
      u8g2.drawFrame(0, itemY - 9, 128, 11);
    } else if (isSelected && editMode == EDIT_MODE) {
      u8g2.setDrawColor(1);
      u8g2.drawBox(0, itemY - 9, 128, 11);
      u8g2.setDrawColor(0);
    }
    
    // Dessiner le texte de l'élément
    drawListItem(itemIndex, 2, itemY);
    
    if (isSelected && editMode == EDIT_MODE) {
      u8g2.setDrawColor(1);
    }
  }
  
  // Dessiner l'indicateur de défilement (optionnel)
  drawScrollIndicator();
}
```

### 4. Indicateur de Défilement Visuel (Optionnel)

```cpp
void drawScrollIndicator() {
  if (TOTAL_ITEMS <= ITEMS_PER_PAGE) {
    return; // Pas besoin d'indicateur si tout tient sur l'écran
  }
  
  // Barre de défilement à droite (2 pixels de large)
  int barX = 126; // Position X (à droite)
  int barY = 0;   // Position Y de départ
  int barHeight = 64; // Hauteur totale
  
  // Calculer la taille et position de la "poignée"
  int handleHeight = (ITEMS_PER_PAGE * barHeight) / TOTAL_ITEMS;
  int handleY = (scrollOffset * barHeight) / TOTAL_ITEMS;
  
  // Dessiner le fond de la barre (optionnel)
  // u8g2.drawFrame(barX, barY, 2, barHeight);
  
  // Dessiner la poignée
  u8g2.drawBox(barX, barY + handleY, 2, handleHeight);
}
```

### 5. Dessiner un Élément de Liste

```cpp
void drawListItem(int itemIndex, int x, int y) {
  char buf[20];
  
  switch (itemIndex) {
    case 0:
      snprintf(buf, 20, "P1 Spd:%dC/h", params.step1Speed);
      break;
    case 1:
      snprintf(buf, 20, "P1 Tmp:%dC", params.step1Temp);
      break;
    case 2:
      snprintf(buf, 20, "P1 Wait:%dm", params.step1Wait);
      break;
    case 3:
      snprintf(buf, 20, "P2 Spd:%dC/h", params.step2Speed);
      break;
    case 4:
      snprintf(buf, 20, "P2 Tmp:%dC", params.step2Temp);
      break;
    case 5:
      snprintf(buf, 20, "P2 Wait:%dm", params.step2Wait);
      break;
    case 6:
      snprintf(buf, 20, "P3 Spd:%dC/h", params.step3Speed);
      break;
    case 7:
      snprintf(buf, 20, "P3 Tmp:%dC", params.step3Temp);
      break;
    case 8:
      snprintf(buf, 20, "P3 Wait:%dm", params.step3Wait);
      break;
    case 9:
      snprintf(buf, 20, "Cool:%dC/h", params.step4Speed);
      break;
    case 10:
      snprintf(buf, 20, "Tgt:%dC", params.step4Target);
      break;
    case 11:
      snprintf(buf, 20, "Settings");
      break;
  }
  
  u8g2.drawStr(x, y, buf);
}
```

---

## 🎮 Navigation avec l'Encodeur

### Modification de `handleEncoder()`

```cpp
void handleEncoder() {
  long newPosition = encoder.read() / 4;
  
  if (newPosition != encoderPosition) {
    int delta = encoderPosition - newPosition;
    encoderPosition = newPosition;
    
    if (editMode == NAV_MODE) {
      // Naviguer dans la liste
      selectedParam += delta;
      
      // Limites circulaires
      if (selectedParam < 0) selectedParam = TOTAL_ITEMS - 1;
      if (selectedParam >= TOTAL_ITEMS) selectedParam = 0;
      
      // Le défilement sera calculé automatiquement dans updateScrollOffset()
    } else {
      // Éditer le paramètre sélectionné
      editParameter(delta);
    }
  }
}
```

---

## 📊 Exemple Visuel du Résultat

### Affichage Normal (tout visible)

```
┌────────────────────────┐
│ Temp: 25C         [S] │ ← Ligne du haut (température)
│ ┏━━━━━━━━━━━━━━━━━━┓ │
│ ┃P1 Spd:100C/h     ┃ │ ← Sélectionné
│ ┗━━━━━━━━━━━━━━━━━━┛ │
│ P1 Tmp:570C          │
│ P1 Wait:5m           │
│ P2 Spd:250C/h        │
│ P2 Tmp:1100C         │
└────────────────────────┘
```

### Après Scroll (milieu de liste)

```
┌────────────────────────┐
│ Temp: 125C        [S] │
│ P2 Wait:20m          │
│ ┏━━━━━━━━━━━━━━━━━━┓ │
│ ┃P3 Spd:200C/h     ┃ │ ← Sélectionné (scroll auto)
│ ┗━━━━━━━━━━━━━━━━━━┛ │
│ P3 Tmp:1100C         │
│ P3 Wait:15m          │
│ Cool:150C/h        ▓ │ ← Barre de scroll
└────────────────────────┘
```

---

## 💾 Coût en Mémoire

### Variables Supplémentaires

```cpp
int scrollOffset = 0;           // 2 octets
const int ITEMS_PER_PAGE = 6;   // 0 octet (constante compilée)
const int TOTAL_ITEMS = 12;     // 0 octet (constante compilée)
```

**Total : 2 octets de RAM** ✅

### Fonctions Supplémentaires

- `updateScrollOffset()` : ~50 octets de Flash
- `drawScrollIndicator()` : ~80 octets de Flash

**Total : ~130 octets de Flash** ✅

---

## ⚡ Optimisations Possibles

### 1. Défilement Doux (Smooth Scroll)

```cpp
float scrollOffsetSmooth = 0.0;
const float SCROLL_SPEED = 0.3;

void updateSmoothScroll() {
  scrollOffsetSmooth += (scrollOffset - scrollOffsetSmooth) * SCROLL_SPEED;
}
```

⚠️ **Coût** : 4 octets RAM + calculs float (lent sur Arduino)

### 2. Indicateur Simplifié

Au lieu d'une barre complète, juste des flèches :

```cpp
void drawSimpleScrollIndicator() {
  if (scrollOffset > 0) {
    u8g2.drawStr(120, 10, "↑"); // Flèche haut
  }
  if (scrollOffset < TOTAL_ITEMS - ITEMS_PER_PAGE) {
    u8g2.drawStr(120, 60, "↓"); // Flèche bas
  }
}
```

**Économie** : ~50 octets de Flash

### 3. Scroll Circulaire Infini

```cpp
// Au lieu de limiter, boucler
if (selectedParam < 0) selectedParam = TOTAL_ITEMS - 1;
if (selectedParam >= TOTAL_ITEMS) selectedParam = 0;
```

✅ **Déjà implémenté dans l'exemple**

---

## ✅ Avantages de Cette Approche

1. ✅ **Simple à implémenter** - ~100 lignes de code
2. ✅ **Peu coûteux** - 2 octets RAM, ~130 octets Flash
3. ✅ **Automatique** - Scroll se calcule seul
4. ✅ **Fluide** - Suit l'élément sélectionné
5. ✅ **Compatible** - Fonctionne avec U8g2 mode 2-page
6. ✅ **Extensible** - Facile d'ajouter des éléments

---

## 🎯 Recommandation

### Pour Votre Cas (11 paramètres + Settings)

**Avec `u8g2_font_6x10_tf` (fonte unique) :**
- 6 lignes visibles
- 12 éléments au total
- **Défilement nécessaire : OUI**

### Plan d'Action

1. ✅ Implémenter le système de défilement (ci-dessus)
2. ✅ Unifier les fontes à `u8g2_font_6x10_tf`
3. ✅ Adapter `drawProgOffScreen()` pour utiliser la liste scrollable
4. ✅ Tester la navigation

**Temps estimé : 1-2 heures d'implémentation**

---

## 🚀 Voulez-vous que je l'implémente ?

Je peux :
1. ✅ Créer les fonctions de défilement
2. ✅ Adapter `drawProgOffScreen()` pour utiliser la liste
3. ✅ Ajouter l'indicateur de scroll
4. ✅ Tester avec les 12 éléments
5. ✅ Unifier les fontes en même temps

**Dites-moi si vous voulez que je procède à l'implémentation !**
