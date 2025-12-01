# 🚀 Guide de Démarrage Rapide - TEST_MODE

## Étape 1 : Upload
```
1. Ouvrir TEST_MODE.ino
2. Sélectionner Arduino Uno
3. Upload
```

## Étape 2 : Tester
```
Encodeur Click → Test suivant (1→2→3→4→5→6→1)
Bouton Poussoir → ON/OFF Relais+LED
```

## Étape 3 : Vérifier

### ✅ Test 1 - ÉCRAN
- [ ] Texte visible et net

### ✅ Test 2 - ENCODEUR
- [ ] Position change quand on tourne
- [ ] Clic fonctionne (change de test)

### ✅ Test 3 - BOUTONS
- [ ] Encodeur SW détecté (ON quand pressé)
- [ ] Push Button détecté (ON quand pressé)

### ✅ Test 4 - THERMOCOUPLE
- [ ] Température affichée (~20-30°C)
- [ ] Pas de code erreur
- [ ] CJ température affichée

### ✅ Test 5 - RELAIS & LED
- [ ] LED s'allume/s'éteint
- [ ] Relais clique (si audible)
- [ ] État change sur écran

### ✅ Test 6 - RÉSUMÉ
- [ ] Toutes les valeurs cohérentes
- [ ] Tous les composants répondent

## ✅ Si Tous les Tests Passent

**FÉLICITATIONS !** Votre système est prêt.

Vous pouvez maintenant uploader le programme principal :
```
LUCIA/lucia.ino
```

## ❌ Si Un Test Échoue

Consulter `README.md` section **Dépannage** pour résoudre le problème.

## 📊 Moniteur Série

Ouvrir à **9600 bauds** pour voir les messages de debug.

---

**Durée totale du test :** ~2-3 minutes  
**Niveau de difficulté :** Facile ⭐

