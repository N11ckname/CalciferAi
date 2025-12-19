# CalciferAi - Notice d'Utilisation

## 🎯 Présentation

CalciferAi est un système de contrôle automatique pour four céramique. Il permet de programmer des cycles de cuisson en 3 phases de chauffe suivies d'un refroidissement contrôlé.

## 🎮 Les Commandes

### Boutons et Encodeur
- **Encodeur rotatif** : Naviguer et modifier les valeurs
- **Clic sur l'encodeur** : Valider/éditer un paramètre
- **Bouton poussoir** : Démarrer/Arrêter le programme

### LED Témoin
- **LED allumée** : Le four chauffe
- **LED éteinte** : Le four ne chauffe pas

## 📋 Écran Principal (Mode Arrêt)

L'écran affiche les paramètres de votre programme de cuisson :

### Phase 1, 2 et 3 (Montée en température)
- **Vitesse** : Vitesse de chauffe en °C/h (10 à 1000)
- **Température** : Température cible en °C (0 à 1500)
- **Durée** : Temps de maintien au palier en minutes (0 à 999)

### Phase 4 (Refroidissement)
- **Vitesse** : Vitesse de refroidissement en °C/h (1 à 1000)
- **Température** : Température de fin de cycle en °C (0 à 1000)

## ⚙️ Configuration d'un Programme

### 1. Navigation entre les paramètres
1. Tournez l'encodeur pour sélectionner un paramètre (il s'affiche en vidéo inversé)
2. Les paramètres défilent dans l'ordre : vitesse → température → durée pour chaque phase

### 2. Modification d'un paramètre
1. **Cliquez** sur l'encodeur pour entrer en mode édition (un cadre fin apparaît)
2. **Tournez** l'encodeur pour modifier la valeur
3. **Cliquez** à nouveau pour valider (la valeur est automatiquement sauvegardée)

### 3. Incréments de modification
- **Vitesses** : Pas de 10°C/h
- **Températures** : Pas de 5°C
- **Durées** : Pas de 1 minute

## 🚀 Démarrage d'un Programme

1. Vérifiez que tous vos paramètres sont corrects
2. **Appuyez sur le bouton poussoir** pour démarrer
3. L'écran passe en mode "EN MARCHE"
4. Le programme démarre automatiquement

### Reprise à chaud
Si vous démarrez un programme alors que le four est déjà chaud, le système détecte automatiquement la phase appropriée et reprend à partir de la température actuelle.

## 🔥 Pendant la Cuisson (Mode En Marche)

### Affichage
- **Phase en cours** : Affichée en blanc
- **Phases terminées** : Grisées
- **Phases à venir** : Grisées
- **Température actuelle** vs **Température cible** : Affichées en temps réel
- **État relais** : ON/OFF avec pourcentage de puissance (en rouge)

### Arrêt d'urgence
**Appuyez sur le bouton poussoir** à tout moment pour arrêter immédiatement le programme et couper le chauffage.

## 🔧 Réglages Avancés (Menu Settings)

### Accès au menu Settings
1. En mode Arrêt, sélectionnez l'icône "S" en haut à droite
2. Cliquez sur l'encodeur pour entrer dans les réglages

### Paramètres disponibles
- **Heat Cycle** : Durée du cycle PWM (100 à 10000 ms) - *Avancé*
- **Kp** : Gain proportionnel PID (0.0 à 10.0) - *Avancé*
- **Ki** : Gain intégral PID (0.0 à 1.0) - *Avancé*
- **Max delta** : Tolérance de fin de phase (1 à 50°C) - *Recommandé : 10°C*
- **Exit** : Sortir du menu Settings

⚠️ **Note** : Ne modifiez les paramètres PID (Kp, Ki) que si vous comprenez leur fonctionnement. Les valeurs par défaut sont optimisées.

## ⚠️ Messages d'Erreur

### "Temp fail 2min" / "Heat stopped"
**Cause** : Le capteur de température ne fonctionne pas correctement depuis plus de 2 minutes.

**Action** :
1. Vérifiez les connexions du thermocouple
2. Vérifiez que le thermocouple n'est pas endommagé
3. Appuyez sur le bouton pour réinitialiser

### "MAX31856 Error!" / "Check wiring"
**Cause** : Le module de lecture de température n'est pas détecté au démarrage.

**Action** :
1. Vérifiez toutes les connexions du module MAX31856
2. Redémarrez le système
3. Appuyez sur le bouton pour tenter une reconnexion

## 🛡️ Consignes de Sécurité

### ⚠️ IMPORTANT
1. **Ne laissez JAMAIS un four en chauffe sans surveillance**
2. Le relais se coupe automatiquement en mode Arrêt
3. En cas d'erreur température > 2 minutes, le chauffage s'arrête automatiquement
4. Le bouton d'arrêt fonctionne à tout moment (arrêt immédiat)

### Recommandations
- Testez toujours votre programme à vide avant une vraie cuisson
- Notez vos programmes de cuisson réussis pour les réutiliser
- Surveillez les premières minutes après le démarrage pour vérifier le bon fonctionnement
- N'ouvrez pas le four pendant la cuisson (sauf urgence)

## 📊 Exemple de Programme Typique

### Cuisson Raku (Exemple)
- **Phase 1** : 100°C/h → 150°C, maintien 60 min (séchage)
- **Phase 2** : 150°C/h → 600°C, maintien 10 min (préchauffage)
- **Phase 3** : 600°C/h → 980°C, maintien 10 min (cuisson)
- **Refroidissement** : 150°C/h → 500°C (refroidissement contrôlé)

### Cuisson Grès (Exemple)
- **Phase 1** : 50°C/h → 100°C, maintien 5 min
- **Phase 2** : 250°C/h → 570°C, maintien 15 min
- **Phase 3** : 200°C/h → 1100°C, maintien 20 min
- **Refroidissement** : 150°C/h → 200°C

## 💾 Sauvegarde des Paramètres

Les paramètres de votre programme sont **automatiquement sauvegardés** à chaque modification. Ils sont conservés même après une coupure de courant.

## 📞 En Cas de Problème

1. **Le four ne chauffe pas** : Vérifiez que le programme est démarré (bouton poussoir)
2. **La température ne monte pas** : Vérifiez le relais SSR et le câblage du four
3. **L'écran est noir** : Vérifiez l'alimentation et les connexions de l'écran OLED
4. **Température aberrante** : Vérifiez le thermocouple (Type S requis)

---

**Version** : 1.0  
**Système** : CalciferAi - Contrôleur intelligent de four céramique

