#!/usr/bin/env python3
"""
Arduino Logger - Capture et enregistrement des données du four LUCIA
Auteur: CalciferAi Project
Description: Script de logging qui écoute le port série Arduino et enregistre
             les données dans des fichiers CSV horodatés.
"""

import serial
import serial.tools.list_ports
import time
import os
import subprocess
import platform
from datetime import datetime
import sys
import threading
# collections.deque supprimé - on garde tout l'historique maintenant

# Import matplotlib pour le graphique
try:
    import matplotlib.pyplot as plt
    import matplotlib.animation as animation
    from matplotlib.gridspec import GridSpec
    MATPLOTLIB_AVAILABLE = True
except ImportError:
    MATPLOTLIB_AVAILABLE = False
    print("⚠️  matplotlib non installé - graphique désactivé")
    print("   Installez avec: pip install matplotlib")

# Configuration
BAUD_RATE = 9600  # 9600 bauds pour robustesse maximale (environnement industriel)
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))  # Répertoire du script
LOGS_DIR = os.path.join(SCRIPT_DIR, "logs")  # Dossier logs toujours à côté du script
TIMEOUT = 2  # Timeout de lecture série (secondes)

# Données pour le graphique (partagées entre threads)
# Listes sans limite pour conserver tout l'historique de la cuisson
graph_data = {
    'time': [],
    'temp': [],
    'target': [],
    'p': [],
    'i': [],
    'power': [],
    'error': [],
    'lock': threading.Lock()
}

def find_arduino_port():
    """
    Détecte automatiquement le port série de l'Arduino.
    Retourne le nom du port ou None si non trouvé.
    """
    print("🔍 Recherche de l'Arduino...")
    ports = serial.tools.list_ports.comports()
    
    print(f"   Ports détectés: {len(ports)}")
    
    # Liste des ports USB potentiels (exclure Bluetooth)
    usb_ports = []
    for port in ports:
        device = port.device
        device_lower = device.lower()
        desc_lower = port.description.lower()
        
        # Exclure les ports Bluetooth
        if 'bluetooth' in device_lower or 'bluetooth' in desc_lower:
            continue
        
        # Collecter tous les ports USB
        if (device.startswith('/dev/cu.usb') or 
            device.startswith('/dev/tty.usb') or
            'ttyusb' in device_lower or
            'ttyacm' in device_lower):
            usb_ports.append(port)
    
    # PRIORITÉ 1 : Ports avec "Arduino" dans la description
    for port in usb_ports:
        if 'arduino' in port.description.lower():
            print(f"✅ Arduino trouvé: {port.device}")
            print(f"   Description: {port.description}")
            return port.device
    
    # PRIORITÉ 2 : Ports avec CH340/FTDI (clones Arduino)
    for port in usb_ports:
        desc_lower = port.description.lower()
        if 'ch340' in desc_lower or 'ftdi' in desc_lower:
            print(f"✅ Arduino (clone) trouvé: {port.device}")
            print(f"   Description: {port.description}")
            return port.device
    
    # PRIORITÉ 3 : S'il n'y a qu'un seul port USB, le prendre
    if len(usb_ports) == 1:
        port = usb_ports[0]
        print(f"✅ Port USB unique trouvé: {port.device}")
        print(f"   Description: {port.description}")
        return port.device
    
    # PRIORITÉ 4 : Plusieurs ports USB, demander à l'utilisateur
    if len(usb_ports) > 1:
        print(f"\n📋 {len(usb_ports)} ports USB détectés:")
        for i, port in enumerate(usb_ports, 1):
            print(f"   {i}. {port.device} - {port.description}")
        
        try:
            choice = input("\nChoisissez le port Arduino (numéro): ")
            if choice.strip():
                index = int(choice) - 1
                if 0 <= index < len(usb_ports):
                    selected = usb_ports[index]
                    print(f"✅ Port sélectionné: {selected.device}")
                    return selected.device
        except (ValueError, IndexError):
            pass
    
    # Si pas trouvé automatiquement, lister tous les ports
    if ports:
        print("\n📋 Ports série disponibles:")
        for i, port in enumerate(ports, 1):
            print(f"   {i}. {port.device}")
            print(f"      Description: {port.description}")
            print(f"      Manufacturer: {port.manufacturer if port.manufacturer else 'N/A'}")
            print()
        
        try:
            choice = input("\nChoisissez un port (numéro) ou appuyez sur Entrée pour quitter: ")
            if choice.strip():
                index = int(choice) - 1
                if 0 <= index < len(ports):
                    selected_port = ports[index].device
                    print(f"✅ Port sélectionné: {selected_port}")
                    return selected_port
        except (ValueError, IndexError):
            print("❌ Choix invalide")
    else:
        print("❌ Aucun port série détecté!")
        print("\nVérifiez que:")
        print("   1. L'Arduino est connecté en USB")
        print("   2. Les drivers USB sont installés (CH340/FTDI)")
        print("   3. Le câble USB fonctionne (testez avec Arduino IDE)")
    
    return None

def create_log_file():
    """
    Crée un nouveau fichier de log avec timestamp.
    Retourne le chemin du fichier et l'objet fichier ouvert.
    """
    # Créer le dossier logs s'il n'existe pas
    if not os.path.exists(LOGS_DIR):
        os.makedirs(LOGS_DIR)
        print(f"📁 Dossier '{LOGS_DIR}' créé")
    
    # Générer un nom de fichier avec timestamp
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    filename = f"lucia_log_{timestamp}.csv"
    filepath = os.path.join(LOGS_DIR, filename)
    
    # Ouvrir le fichier en mode écriture
    log_file = open(filepath, 'w', encoding='utf-8')
    
    # Écrire l'en-tête
    log_file.write("# LUCIA Four Log File\n")
    log_file.write(f"# Date: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}\n")
    log_file.write("# Format: START ligne = config démarrage, DATA lignes = données en temps réel\n")
    log_file.write("#\n")
    log_file.flush()
    
    print(f"📄 Fichier de log créé: {filepath}")
    return filepath, log_file

def parse_and_display_startup(line):
    """Parse et affiche les informations de démarrage de façon lisible."""
    # Nouveau format texte lisible depuis Arduino
    if "LUCIA START" in line:
        print("\n" + "="*60)
        print("🚀 DÉMARRAGE DU FOUR LUCIA")
        print("="*60)
        return True
    return False

def parse_and_display_data(line, count):
    """Parse et affiche les données en temps réel."""
    # Format Arduino: time, temp, target, P, I, power, error
    parts = [p.strip() for p in line.split(',')]
    
    try:
        if len(parts) >= 7:
            timestamp = int(parts[0])
            temp_actual = float(parts[1])
            temp_target = float(parts[2])
            pid_p = float(parts[3])
            pid_i = float(parts[4])
            power = int(float(parts[5]))
            pid_error = float(parts[6])
            
            # Ajouter les données au graphique
            time_minutes = timestamp / 60000.0  # Convertir en minutes
            with graph_data['lock']:
                graph_data['time'].append(time_minutes)
                graph_data['temp'].append(temp_actual)
                graph_data['target'].append(temp_target)
                graph_data['p'].append(pid_p)
                graph_data['i'].append(pid_i)
                graph_data['power'].append(power)
                graph_data['error'].append(pid_error)
            
            # Calculer le temps écoulé pour affichage
            hours = timestamp // 3600000
            minutes = (timestamp % 3600000) // 60000
            seconds = (timestamp % 60000) // 1000
            
            # Afficher une ligne compacte
            if count % 12 == 1:  # En-tête toutes les 12 lignes (1 minute)
                print("\n  Temps  |  Sonde  | Consigne | P     | I     | Power | Error")
                print("-" * 68)
            
            print(f"{hours:02d}:{minutes:02d}:{seconds:02d} | {temp_actual:6.1f}° | {temp_target:7.1f}° | "
                  f"{pid_p:5.1f} | {pid_i:5.1f} | {power:3d}% | {pid_error:5.1f}")
            return True
    except (ValueError, IndexError):
        pass
    
    return False

def setup_graph():
    """Configure et retourne la figure matplotlib avec les subplots."""
    if not MATPLOTLIB_AVAILABLE:
        return None, None, None
    
    plt.style.use('dark_background')
    fig = plt.figure(figsize=(12, 7))
    fig.suptitle('LUCIA - Monitoring Four Céramique', fontsize=14, fontweight='bold')
    
    gs = GridSpec(2, 1, height_ratios=[2, 1.5], hspace=0.3)
    
    # Subplot 1 : Température
    ax1 = fig.add_subplot(gs[0])
    ax1.set_ylabel('Température (°C)')
    ax1.set_title('Température')
    ax1.grid(True, alpha=0.3)
    line_temp, = ax1.plot([], [], 'r-', label='Mesure', linewidth=2)
    line_target, = ax1.plot([], [], 'g--', label='Consigne', linewidth=1.5)
    ax1.legend(loc='upper left')
    
    # Subplot 2 : PID (P, I) + Puissance
    ax2 = fig.add_subplot(gs[1])
    ax2.set_ylabel('Valeur PID')
    ax2.set_xlabel('Temps (minutes)')
    ax2.set_title('Composantes PID & Puissance')
    ax2.grid(True, alpha=0.3)
    ax2.set_ylim(0, 105)  # Axe Y fixé à 100 (+ marge)
    line_p, = ax2.plot([], [], 'c-', label='P', linewidth=1.5)
    line_i, = ax2.plot([], [], 'y-', label='I', linewidth=1.5)  # I en jaune
    
    # Axe Y secondaire pour la puissance (%)
    ax2_power = ax2.twinx()
    ax2_power.set_ylabel('Puissance (%)', color='red')
    ax2_power.set_ylim(0, 105)
    ax2_power.tick_params(axis='y', labelcolor='red')
    line_power, = ax2_power.plot([], [], 'red', label='Power', linewidth=2, alpha=0.8)
    
    # Légende combinée
    lines_for_legend = [line_p, line_i, line_power]
    labels_for_legend = ['P', 'I', 'Power']
    ax2.legend(lines_for_legend, labels_for_legend, loc='upper left')
    
    lines = {
        'temp': line_temp,
        'target': line_target,
        'p': line_p,
        'i': line_i,
        'power': line_power
    }
    axes = {'ax1': ax1, 'ax2': ax2}
    
    plt.tight_layout()
    return fig, lines, axes

def update_graph(frame, lines, axes):
    """Met à jour le graphique avec les nouvelles données."""
    with graph_data['lock']:
        if len(graph_data['time']) < 2:
            return lines.values()
        
        time_data = list(graph_data['time'])
        temp_data = list(graph_data['temp'])
        target_data = list(graph_data['target'])
        p_data = list(graph_data['p'])
        i_data = list(graph_data['i'])
        power_data = list(graph_data['power'])
    
    # Mettre à jour les lignes
    lines['temp'].set_data(time_data, temp_data)
    lines['target'].set_data(time_data, target_data)
    lines['p'].set_data(time_data, p_data)
    lines['i'].set_data(time_data, i_data)
    lines['power'].set_data(time_data, power_data)
    
    # Ajuster les axes dynamiquement
    if time_data:
        x_max = max(time_data)
        x_margin = max(1, x_max * 0.05)
        
        for ax in axes.values():
            ax.set_xlim(0, x_max + x_margin)
        
        # Axe Y pour température (jamais négatif)
        if temp_data and target_data:
            all_temps = temp_data + target_data
            y_min, y_max = min(all_temps), max(all_temps)
            y_margin = max(10, (y_max - y_min) * 0.1)
            axes['ax1'].set_ylim(0, y_max + y_margin)
        
        # Axe Y pour PID fixé à 100 (configuré dans setup_graph)
    
    return lines.values()

def serial_reader_thread(ser, log_file, stop_event):
    """Thread de lecture série (tourne en arrière-plan)."""
    data_count = 0
    in_data_mode = False
    
    while not stop_event.is_set():
        try:
            if ser.in_waiting > 0:
                line = ser.readline().decode('utf-8', errors='ignore').strip()
                
                if line:
                    # TOUJOURS écrire dans le fichier de log
                    log_file.write(line + '\n')
                    log_file.flush()
                    
                    # Affichage selon le type de ligne
                    if "LUCIA START" in line:
                        parse_and_display_startup(line)
                        in_data_mode = False
                    elif "PROGRAMME DEMARRE" in line:
                        print("\n" + ">"*60)
                        print("▶️  PROGRAMME DÉMARRÉ")
                        print(">"*60)
                        in_data_mode = False
                    elif "PROGRAMME ARRETE" in line:
                        print("\n" + "<"*60)
                        print("⏹️  PROGRAMME ARRÊTÉ")
                        print("<"*60)
                        in_data_mode = False
                    elif line.startswith("PID:"):
                        print(f"   {line}")
                    elif line.startswith("Time(ms)"):
                        print(f"\n📊 Format: {line}")
                        in_data_mode = True
                    elif line == "---":
                        in_data_mode = True
                    elif line and line[0].isdigit() and ',' in line:
                        data_count += 1
                        if not parse_and_display_data(line, data_count):
                            print(f"[Data] {line}")
                    elif line:
                        print(f"[Arduino] {line}")
            
            time.sleep(0.01)
            
        except Exception as e:
            if not stop_event.is_set():
                print(f"\n❌ Erreur lecture: {e}")
            break

def main():
    """Fonction principale du logger."""
    print("╔════════════════════════════════════════════════════════════╗")
    print("║         LUCIA Four - Logger de données Arduino            ║")
    print("║                   CalciferAi Project                       ║")
    print("╚════════════════════════════════════════════════════════════╝\n")
    
    # Trouver le port Arduino
    port = find_arduino_port()
    if not port:
        print("❌ Aucun Arduino détecté. Vérifiez la connexion USB.")
        input("\nAppuyez sur Entrée pour quitter...")
        return
    
    # Créer le fichier de log
    log_filepath, log_file = create_log_file()
    
    ser = None
    stop_event = threading.Event()
    reader_thread = None
    
    try:
        # Sur Mac/Linux : utiliser stty pour désactiver le reset (hupcl)
        if platform.system() in ['Darwin', 'Linux']:
            try:
                subprocess.run(['stty', '-f', port, '-hupcl'], 
                              capture_output=True, timeout=2)
                subprocess.run(['stty', '-f', port, str(BAUD_RATE)], 
                              capture_output=True, timeout=2)
            except Exception:
                pass
        
        # Créer l'objet Serial sans ouvrir le port
        ser = serial.Serial()
        ser.port = port
        ser.baudrate = BAUD_RATE
        ser.timeout = TIMEOUT
        ser.write_timeout = TIMEOUT
        ser.bytesize = serial.EIGHTBITS
        ser.parity = serial.PARITY_NONE
        ser.stopbits = serial.STOPBITS_ONE
        ser.xonxoff = False
        ser.rtscts = False
        ser.dsrdtr = False
        ser.dtr = False
        ser.rts = False
        
        ser.open()
        ser.dtr = False
        ser.rts = False
        
        if not ser.is_open:
            print(f"❌ Impossible d'ouvrir {port}")
            return
        
        print(f"✅ Connexion établie sur {port} @ {BAUD_RATE} bauds")
        time.sleep(0.3)
        ser.reset_input_buffer()
        
        print("🎧 Écoute en cours...")
        print("📊 Format: Time(ms), Temp(C), Target(C), P, I, Power(%), Error(C)")
        
        # Lancer le thread de lecture série
        reader_thread = threading.Thread(
            target=serial_reader_thread, 
            args=(ser, log_file, stop_event),
            daemon=True
        )
        reader_thread.start()
        
        # Configurer et lancer le graphique (dans le thread principal)
        if MATPLOTLIB_AVAILABLE:
            print("📈 Ouverture du graphique...\n")
            fig, lines, axes = setup_graph()
            
            if fig:
                ani = animation.FuncAnimation(
                    fig, update_graph, fargs=(lines, axes),
                    interval=1000, blit=False, cache_frame_data=False
                )
                plt.show()  # Bloque jusqu'à fermeture de la fenêtre
        else:
            print("\n⚠️  Graphique non disponible (installez matplotlib)")
            print("   Continuez avec Ctrl+C pour arrêter\n")
            # Boucle d'attente si pas de graphique
            try:
                while True:
                    time.sleep(1)
            except KeyboardInterrupt:
                pass
    
    except serial.SerialException as e:
        print(f"❌ Erreur port série: {e}")
    
    except KeyboardInterrupt:
        print("\n\n⚠️  Interruption (Ctrl+C)")
    
    finally:
        # Arrêt propre
        stop_event.set()
        
        if reader_thread and reader_thread.is_alive():
            reader_thread.join(timeout=1)
        
        if ser and ser.is_open:
            ser.close()
            print("🔌 Port série fermé")
        
        if log_file:
            log_file.close()
            print(f"💾 Log sauvegardé: {log_filepath}")
        
        print("\n✅ Logger arrêté")

if __name__ == "__main__":
    main()
