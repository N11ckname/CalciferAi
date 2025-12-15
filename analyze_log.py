#!/usr/bin/env python3
"""
Analyse et visualisation du log LUCIA
Génère des graphiques détaillés du comportement du four et du PID
"""

import matplotlib
matplotlib.use('Agg')  # Mode non-interactif pour éviter les problèmes d'affichage
import matplotlib.pyplot as plt
import numpy as np
from datetime import datetime
import sys

def parse_log_file(filepath):
    """Parse le fichier de log et extrait les données."""
    time_data = []
    temp_data = []
    target_data = []
    p_data = []
    i_data = []
    d_data = []
    power_data = []
    
    with open(filepath, 'r', encoding='utf-8') as f:
        for line in f:
            line = line.strip()
            # Ignorer les commentaires et lignes vides
            if not line or line.startswith('#') or line.startswith('===') or line.startswith('>>>') or line.startswith('<<<') or line.startswith('PID:') or line.startswith('Time(ms)') or line == '---' or line.startswith('Phase') or line.startswith('Temperature') or line.startswith('Refroidissement'):
                continue
            
            # Parser les lignes de données
            if ',' in line and line[0].isdigit():
                parts = [p.strip() for p in line.split(',')]
                try:
                    if len(parts) >= 7:
                        timestamp = int(parts[0])
                        temp = float(parts[1])
                        target = float(parts[2])
                        p = float(parts[3])
                        i = float(parts[4])
                        d = float(parts[5])
                        power = int(float(parts[6]))
                        
                        # Convertir le temps en minutes
                        time_min = timestamp / 60000.0
                        
                        time_data.append(time_min)
                        temp_data.append(temp)
                        target_data.append(target)
                        p_data.append(p)
                        i_data.append(i)
                        d_data.append(d)
                        power_data.append(power)
                except (ValueError, IndexError):
                    continue
    
    return {
        'time': np.array(time_data),
        'temp': np.array(temp_data),
        'target': np.array(target_data),
        'p': np.array(p_data),
        'i': np.array(i_data),
        'd': np.array(d_data),
        'power': np.array(power_data)
    }

def create_comprehensive_graph(data, filepath):
    """Crée un graphique complet avec 4 subplots."""
    
    # Calculer l'erreur (Target - Temp)
    error = data['target'] - data['temp']
    
    # Configuration du style
    plt.style.use('dark_background')
    
    # Créer la figure avec 4 subplots
    fig, axes = plt.subplots(4, 1, figsize=(16, 12))
    fig.suptitle('LUCIA - Analyse Complète du Cycle de Cuisson', fontsize=16, fontweight='bold')
    
    # Subplot 1 : Températures
    ax1 = axes[0]
    ax1.plot(data['time'], data['temp'], 'r-', label='Température Mesurée', linewidth=2)
    ax1.plot(data['time'], data['target'], 'g--', label='Consigne', linewidth=1.5, alpha=0.8)
    ax1.fill_between(data['time'], data['temp'], data['target'], alpha=0.2, color='yellow')
    ax1.set_ylabel('Température (°C)', fontsize=12)
    ax1.set_title('Température Mesurée vs Consigne', fontsize=14)
    ax1.legend(loc='upper left')
    ax1.grid(True, alpha=0.3)
    ax1.set_xlim(data['time'][0], data['time'][-1])
    
    # Subplot 2 : Erreur
    ax2 = axes[1]
    ax2.plot(data['time'], error, 'cyan', linewidth=1.5, label='Erreur (Target - Temp)')
    ax2.axhline(y=0, color='white', linestyle='--', alpha=0.5)
    ax2.fill_between(data['time'], 0, error, where=(error > 0), color='orange', alpha=0.3, label='Four en retard')
    ax2.fill_between(data['time'], 0, error, where=(error < 0), color='blue', alpha=0.3, label='Dépassement')
    ax2.set_ylabel('Erreur (°C)', fontsize=12)
    ax2.set_title('Erreur de Température (Target - Temp)', fontsize=14)
    ax2.legend(loc='upper right')
    ax2.grid(True, alpha=0.3)
    ax2.set_xlim(data['time'][0], data['time'][-1])
    
    # Subplot 3 : Composantes PID
    ax3 = axes[2]
    ax3.plot(data['time'], data['p'], 'c-', label='P (Proportionnel)', linewidth=1.5)
    ax3.plot(data['time'], data['i'], 'm-', label='I (Intégral)', linewidth=1.5)
    ax3.plot(data['time'], data['d'], 'y-', label='D (Dérivé)', linewidth=1.5)
    ax3.set_ylabel('Valeur PID', fontsize=12)
    ax3.set_title('Composantes du Contrôleur PID', fontsize=14)
    ax3.legend(loc='upper left')
    ax3.grid(True, alpha=0.3)
    ax3.set_xlim(data['time'][0], data['time'][-1])
    
    # Subplot 4 : Puissance
    ax4 = axes[3]
    ax4.plot(data['time'], data['power'], 'red', linewidth=2, label='Puissance Relais')
    ax4.fill_between(data['time'], 0, data['power'], alpha=0.3, color='red')
    ax4.set_ylabel('Puissance (%)', fontsize=12)
    ax4.set_xlabel('Temps (minutes)', fontsize=12)
    ax4.set_title('Puissance du Relais', fontsize=14)
    ax4.legend(loc='upper right')
    ax4.grid(True, alpha=0.3)
    ax4.set_ylim(0, 105)
    ax4.set_xlim(data['time'][0], data['time'][-1])
    
    # Ajuster l'espacement
    plt.tight_layout()
    
    # Sauvegarder le graphique
    output_file = filepath.replace('.csv', '_analysis.png')
    plt.savefig(output_file, dpi=300, bbox_inches='tight')
    print(f"✅ Graphique sauvegardé : {output_file}")
    plt.close(fig)

def print_statistics(data):
    """Affiche des statistiques sur le cycle."""
    print("\n" + "="*60)
    print("📊 STATISTIQUES DU CYCLE DE CUISSON")
    print("="*60)
    
    duration_hours = data['time'][-1] / 60.0
    
    print(f"\n⏱️  Durée totale : {duration_hours:.1f} heures ({data['time'][-1]:.0f} minutes)")
    print(f"\n🌡️  Températures :")
    print(f"   • Température initiale : {data['temp'][0]:.1f}°C")
    print(f"   • Température finale : {data['temp'][-1]:.1f}°C")
    print(f"   • Température maximale : {np.max(data['temp']):.1f}°C")
    print(f"   • Consigne maximale : {np.max(data['target']):.1f}°C")
    
    # Calculer l'erreur
    error = data['target'] - data['temp']
    print(f"\n📏 Erreur :")
    print(f"   • Erreur moyenne : {np.mean(error):.1f}°C")
    print(f"   • Erreur max (retard) : {np.max(error):.1f}°C")
    print(f"   • Erreur min (dépassement) : {np.min(error):.1f}°C")
    print(f"   • Écart-type : {np.std(error):.1f}°C")
    
    print(f"\n🔧 PID :")
    print(f"   • P moyen : {np.mean(data['p']):.1f}")
    print(f"   • I moyen : {np.mean(data['i']):.1f}")
    print(f"   • I maximum : {np.max(data['i']):.1f}")
    print(f"   • D moyen : {np.mean(data['d']):.1f}")
    
    print(f"\n⚡ Puissance :")
    print(f"   • Puissance moyenne : {np.mean(data['power']):.0f}%")
    print(f"   • Puissance maximale : {np.max(data['power']):.0f}%")
    print(f"   • Temps à 100% : {np.sum(data['power'] == 100) * 5 / 60:.1f} minutes")
    
    # Détecter les anomalies de température (variations > 10°C)
    temp_diff = np.diff(data['temp'])
    anomalies = np.where(np.abs(temp_diff) > 10)[0]
    if len(anomalies) > 0:
        print(f"\n⚠️  Anomalies détectées :")
        print(f"   • {len(anomalies)} variation(s) > 10°C détectée(s)")
        for idx in anomalies[:5]:  # Montrer les 5 premières
            print(f"   • À {data['time'][idx]:.1f} min : {data['temp'][idx]:.1f}°C → {data['temp'][idx+1]:.1f}°C ({temp_diff[idx]:.1f}°C)")
    
    print("\n" + "="*60 + "\n")

def main():
    if len(sys.argv) > 1:
        filepath = sys.argv[1]
    else:
        # Utiliser le fichier par défaut
        filepath = "logs/lucia_log_20251215_185044.csv"
    
    print(f"📂 Lecture du fichier : {filepath}")
    
    try:
        data = parse_log_file(filepath)
        
        if len(data['time']) == 0:
            print("❌ Aucune donnée trouvée dans le fichier")
            return
        
        print(f"✅ {len(data['time'])} points de données chargés")
        
        # Afficher les statistiques
        print_statistics(data)
        
        # Créer le graphique
        print("📈 Génération du graphique...")
        create_comprehensive_graph(data, filepath)
        
    except FileNotFoundError:
        print(f"❌ Fichier non trouvé : {filepath}")
    except Exception as e:
        print(f"❌ Erreur : {e}")
        import traceback
        traceback.print_exc()

if __name__ == "__main__":
    main()

