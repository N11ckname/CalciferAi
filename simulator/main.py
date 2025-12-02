"""
Simulateur CalciferAi - Four céramique
Point d'entrée principal
"""

import sys
import time
from kiln_physics import KilnPhysics
from temperature import TemperatureController
from arduino_core import ArduinoCore
from display import DisplayRenderer
from ui import SimulatorUI
from config import SIMULATION_SPEED, MAX_SIMULATION_SPEED, MIN_SIMULATION_SPEED, ProgramState


class CalciferSimulator:
    """Simulateur principal"""
    
    def __init__(self):
        print("🔥 Initialisation du simulateur CalciferAi...")
        
        # Créer les composants
        self.kiln = KilnPhysics()
        self.temp_controller = TemperatureController()
        self.arduino = ArduinoCore(self.temp_controller, self.kiln)
        self.display_renderer = DisplayRenderer()
        self.ui = SimulatorUI()
        
        # Vitesse de simulation
        self.simulation_speed = SIMULATION_SPEED
        
        # Timing
        self.last_time = time.time()
        
        print("✅ Simulateur prêt!")
        print("\n" + "="*60)
        print("CONTRÔLES:")
        print("  Flèches ← → : Rotation encodeur")
        print("  Entrée/Espace : Clic encodeur")
        print("  S : Bouton Start/Stop")
        print("  R : Reset du four")
        print("  +/- : Vitesse de simulation")
        print("  0 : Vitesse normale (1x)")
        print("="*60 + "\n")
    
    def run(self):
        """Boucle principale du simulateur"""
        self.arduino.setup()
        
        running = True
        while running:
            # Calculer le delta time
            current_time = time.time()
            dt_real = current_time - self.last_time
            self.last_time = current_time
            
            # Appliquer la vitesse de simulation
            dt_sim = dt_real * self.simulation_speed
            
            # Gérer les événements UI
            actions = self.ui.handle_events()
            
            if actions['quit']:
                running = False
                continue
            
            # Gérer les contrôles
            self.handle_actions(actions)
            
            # Mettre à jour la simulation Arduino
            status = self.arduino.loop(dt_sim)
            
            if status == 'critical_error':
                self.render_critical_error()
            else:
                self.render_normal()
            
            # Limiter la vitesse (60 FPS max)
            time.sleep(0.001)
        
        self.cleanup()
    
    def handle_actions(self, actions):
        """Gère les actions de l'utilisateur"""
        # Encodeur
        if actions['encoder_left']:
            self.arduino.handle_encoder_rotation(-1)
        if actions['encoder_right']:
            self.arduino.handle_encoder_rotation(1)
        if actions['encoder_click']:
            self.arduino.handle_encoder_click()
        if actions['encoder_long_press']:
            self.arduino.handle_encoder_long_press()
        
        # Bouton push
        if actions['push_button']:
            self.arduino.handle_push_button()
        
        # Contrôles de vitesse
        if actions['speed_slower']:
            self.simulation_speed = max(MIN_SIMULATION_SPEED, self.simulation_speed / 1.5)
            print(f"⏱️  Vitesse: {self.simulation_speed:.1f}x")
        if actions['speed_faster']:
            self.simulation_speed = min(MAX_SIMULATION_SPEED, self.simulation_speed * 1.5)
            print(f"⏱️  Vitesse: {self.simulation_speed:.1f}x")
        if actions['speed_reset']:
            self.simulation_speed = 1.0
            print("⏱️  Vitesse: 1.0x (temps réel)")
        
        # Reset
        if actions['reset']:
            self.reset_simulation()
    
    def reset_simulation(self):
        """Réinitialise la simulation"""
        print("\n🔄 Reset de la simulation...")
        self.kiln.reset()
        self.temp_controller.init()
        self.arduino.prog_state = ProgramState.PROG_OFF
        self.arduino.current_phase = 0
        self.arduino.graph_actual = []
        self.arduino.graph_target = []
        self.arduino.graph_time = []
        print("✅ Simulation réinitialisée\n")
    
    def render_normal(self):
        """Rendu normal de l'interface"""
        # Obtenir l'état actuel
        state = self.arduino.get_display_state()
        
        # Générer l'affichage OLED
        if state['show_graph'] and state['prog_state'] == ProgramState.PROG_ON:
            # Afficher le graphe si activé en mode PROG_ON
            oled = self.display_renderer.draw_graph(
                state['graph_actual'],
                state['graph_target'],
                state['current_temp'],
                state['target_temp'],
                state['current_phase'],
                state['program_start_time']
            )
        elif state['prog_state'] == ProgramState.PROG_OFF:
            oled = self.display_renderer.draw_prog_off_screen(
                state['params'],
                state['current_temp'],
                state['power_hold'],
                state['selected_param'],
                state['edit_mode'],
                state['temp_fail_active']
            )
        else:
            oled = self.display_renderer.draw_prog_on_screen(
                state['params'],
                state['current_temp'],
                state['target_temp'],
                state['power_hold'],
                state['current_phase'],
                state['phase_start_time'],
                state['plateau_reached'],
                state['plateau_start_time'],
                state['temp_fail_active']
            )
        
        # Afficher l'interface
        self.ui.draw(oled, state, self.kiln, self.simulation_speed)
    
    def render_critical_error(self):
        """Rendu de l'écran d'erreur critique"""
        oled = self.display_renderer.draw_critical_error()
        state = self.arduino.get_display_state()
        self.ui.draw(oled, state, self.kiln, self.simulation_speed)
        
        # Attendre une action utilisateur
        print("\n❌ ERREUR CRITIQUE: Défaillance température depuis 2 minutes!")
        print("   Appuyez sur Start/Stop pour continuer...\n")
    
    def cleanup(self):
        """Nettoie les ressources"""
        print("\n🔥 Arrêt du simulateur...")
        self.ui.cleanup()
        print("👋 Au revoir!\n")


def main():
    """Point d'entrée principal"""
    try:
        simulator = CalciferSimulator()
        simulator.run()
    except KeyboardInterrupt:
        print("\n\n⚠️  Interruption par l'utilisateur")
        sys.exit(0)
    except Exception as e:
        print(f"\n❌ ERREUR FATALE: {e}")
        import traceback
        traceback.print_exc()
        sys.exit(1)


if __name__ == "__main__":
    main()

