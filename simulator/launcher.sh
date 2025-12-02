#!/bin/bash

# Lanceur interactif pour le simulateur CalciferAi

echo "════════════════════════════════════════════════════════════"
echo "🔥  Simulateur CalciferAi - Four Céramique"
echo "════════════════════════════════════════════════════════════"
echo ""

# Fonction pour vérifier si une commande existe
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Vérifier Python
if command_exists python3; then
    PYTHON=python3
elif command_exists python; then
    PYTHON=python
else
    echo "❌ Python n'est pas installé"
    echo "   Installez Python 3.8 ou supérieur depuis python.org"
    exit 1
fi

echo "✅ Python trouvé: $PYTHON"
$PYTHON --version
echo ""

# Vérifier les dépendances
echo "Vérification des dépendances..."
$PYTHON check_installation.py

if [ $? -ne 0 ]; then
    echo ""
    echo "❓ Voulez-vous installer les dépendances maintenant? (o/n)"
    read -r response
    if [[ "$response" =~ ^([oO][uU][iI]|[oO])$ ]]; then
        echo ""
        echo "Installation des dépendances..."
        pip3 install -r requirements.txt
        
        if [ $? -eq 0 ]; then
            echo "✅ Installation réussie!"
        else
            echo "❌ Erreur lors de l'installation"
            exit 1
        fi
    else
        echo "Installation annulée."
        exit 1
    fi
fi

echo ""
echo "════════════════════════════════════════════════════════════"
echo "🚀 Lancement du simulateur..."
echo "════════════════════════════════════════════════════════════"
echo ""
echo "Contrôles rapides:"
echo "  ← → : Encodeur"
echo "  Entrée : Clic"
echo "  S : Start/Stop"
echo "  + - : Vitesse"
echo ""
echo "Appuyez sur Entrée pour continuer..."
read -r

# Lancer le simulateur
$PYTHON main.py

echo ""
echo "👋 Simulateur fermé."

