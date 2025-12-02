#!/usr/bin/env python3
"""
Script de vérification de l'installation du simulateur CalciferAi
Vérifie que toutes les dépendances sont installées
"""

import sys

def check_module(module_name, package_name=None):
    """Vérifie qu'un module est installé"""
    if package_name is None:
        package_name = module_name
    
    try:
        __import__(module_name)
        print(f"✅ {package_name}")
        return True
    except ImportError:
        print(f"❌ {package_name} - MANQUANT")
        return False

def main():
    print("="*60)
    print("🔥 Vérification de l'installation CalciferAi Simulator")
    print("="*60)
    print()
    
    # Vérifier Python
    print(f"Version Python: {sys.version}")
    if sys.version_info < (3, 8):
        print("⚠️  Python 3.8 ou supérieur est recommandé")
    print()
    
    # Vérifier les modules
    print("Vérification des dépendances:")
    print("-" * 60)
    
    all_ok = True
    all_ok &= check_module("pygame", "pygame")
    all_ok &= check_module("matplotlib", "matplotlib")
    all_ok &= check_module("numpy", "numpy")
    
    print()
    
    # Vérifier les fichiers du simulateur
    print("Vérification des fichiers:")
    print("-" * 60)
    
    import os
    files = [
        "main.py",
        "arduino_core.py",
        "temperature.py",
        "display.py",
        "kiln_physics.py",
        "ui.py",
        "config.py",
        "requirements.txt",
        "README.md"
    ]
    
    for filename in files:
        if os.path.exists(filename):
            print(f"✅ {filename}")
        else:
            print(f"❌ {filename} - MANQUANT")
            all_ok = False
    
    print()
    print("="*60)
    
    if all_ok:
        print("✅ Installation complète ! Vous pouvez lancer le simulateur.")
        print()
        print("Pour démarrer:")
        print("  python3 main.py")
        print()
        return 0
    else:
        print("❌ Installation incomplète.")
        print()
        print("Pour installer les dépendances manquantes:")
        print("  pip3 install -r requirements.txt")
        print()
        return 1

if __name__ == "__main__":
    sys.exit(main())

