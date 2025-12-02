"""
Logique principale du programme Arduino
Transposition de lucia.ino
"""

import time
import json
import os
from config import ProgramState, Phase, EditMode, DEFAULT_FIRING_PARAMS


class ArduinoCore:
    """Émule la logique principale de l'Arduino"""
    
    def __init__(self, temp_controller, kiln_physics):
        self.temp_controller = temp_controller
        self.kiln = kiln_physics
        
        # État du programme
        self.prog_state = ProgramState.PROG_OFF
        self.current_phase = Phase.PHASE_0
        
        # Paramètres de cuisson
        self.params = DEFAULT_FIRING_PARAMS.copy()
        
        # Interface utilisateur
        self.edit_mode = EditMode.NAV_MODE
        self.selected_param = 0
        self.num_params = 11
        self.encoder_position = 0
        
        # Variables de timing
        self.last_display_update = time.time()
        self.program_start_time = time.time()
        self.phase_start_time = time.time()
        self.plateau_start_time = time.time()
        self.temp_fail_start_time = time.time()
        self.temp_fail_active = False
        self.last_graph_update = time.time()
        
        # États des boutons
        self.last_encoder_button = False
        self.last_push_button = False
        self.encoder_button_press_time = time.time()
        self.long_press_handled = False
        self.show_graph = False
        
        # Contrôle de température
        self.target_temp = 20.0
        self.plateau_reached = False
        
        # Graphique
        self.graph_actual = []
        self.graph_target = []
        self.graph_time = []
        
        # EEPROM virtuel
        self.eeprom_file = "simulator_eeprom.json"
        self.load_from_eeprom()
        
    def setup(self):
        """Initialisation (équivalent de setup())"""
        self.temp_controller.init()
        self.load_from_eeprom()
        
    def loop(self, dt):
        """
        Boucle principale (équivalent de loop())
        
        Args:
            dt: Delta time en secondes
        """
        current_time = time.time()
        
        # Lire la température
        temp = self.kiln.get_temperature()
        
        # Vérifier les défaillances de lecture de température
        if temp < -100 or temp > 1500:
            if not self.temp_fail_active:
                self.temp_fail_active = True
                self.temp_fail_start_time = current_time
            elif current_time - self.temp_fail_start_time > 120:  # 2 minutes
                # Erreur critique - arrêter le chauffage
                if self.prog_state == ProgramState.PROG_ON:
                    self.prog_state = ProgramState.PROG_OFF
                    self.current_phase = Phase.PHASE_0
                    self.temp_controller.update(temp, self.target_temp, False)
                return 'critical_error'
        else:
            self.temp_fail_active = False
        
        # Mettre à jour le programme
        if self.prog_state == ProgramState.PROG_ON:
            self.update_program(current_time, temp)
            self.update_graph_data(temp)
        
        # Mettre à jour le contrôle de température
        enabled = (self.prog_state == ProgramState.PROG_ON)
        relay_on, power = self.temp_controller.update(temp, self.target_temp, enabled)
        
        # Mettre à jour la physique du four
        self.kiln.update(dt, power, relay_on)
        
        return 'normal'
    
    def handle_encoder_rotation(self, delta):
        """Gère la rotation de l'encodeur"""
        if self.prog_state == ProgramState.PROG_OFF and not self.show_graph:
            if self.edit_mode == EditMode.NAV_MODE:
                # Navigation entre les paramètres
                self.selected_param += delta
                if self.selected_param < 0:
                    self.selected_param = self.num_params - 1
                if self.selected_param >= self.num_params:
                    self.selected_param = 0
            else:
                # Édition du paramètre sélectionné
                self.edit_parameter(delta)
    
    def handle_encoder_click(self):
        """Gère le clic sur l'encodeur (appui court)"""
        if self.prog_state == ProgramState.PROG_ON:
            # En mode PROG_ON, switcher entre l'écran normal et le graphe
            self.show_graph = not self.show_graph
        elif self.prog_state == ProgramState.PROG_OFF:
            # En mode PROG_OFF, basculer le mode d'édition
            self.toggle_edit_mode()
    
    def handle_encoder_long_press(self):
        """Gère l'appui long sur l'encodeur"""
        # Plus utilisé, mais gardé pour compatibilité
        pass
    
    def handle_push_button(self):
        """Gère l'appui sur le bouton push (start/stop)"""
        self.toggle_prog_state()
    
    def toggle_edit_mode(self):
        """Bascule entre mode navigation et mode édition"""
        if self.edit_mode == EditMode.NAV_MODE:
            self.edit_mode = EditMode.EDIT_MODE
        else:
            self.edit_mode = EditMode.NAV_MODE
            self.save_to_eeprom()
    
    def toggle_prog_state(self):
        """Bascule entre PROG_OFF et PROG_ON"""
        if self.prog_state == ProgramState.PROG_OFF:
            self.prog_state = ProgramState.PROG_ON
            self.current_phase = Phase.PHASE_1
            self.program_start_time = time.time()
            self.phase_start_time = time.time()
            self.plateau_reached = False
            self.target_temp = self.kiln.get_temperature()
            self.graph_actual = []
            self.graph_target = []
            self.graph_time = []
            self.last_graph_update = time.time()
            self.temp_controller.reset_pid()
        else:
            self.prog_state = ProgramState.PROG_OFF
            self.current_phase = Phase.PHASE_0
            self.temp_controller.update(0, 0, False)
            self.show_graph = False
    
    def edit_parameter(self, delta):
        """Édite le paramètre sélectionné - Ordre: gauche à droite, haut en bas"""
        param_config = [
            # Phase 1
            ('step1Speed', 1, 1000, 10),   # 0: P1 gauche
            ('step1Temp', 0, 1500, 10),    # 1: P1 centre
            ('step1Wait', 0, 999, 5),      # 2: P1 droite
            # Phase 2
            ('step2Speed', 1, 1000, 10),   # 3: P2 gauche
            ('step2Temp', 0, 1500, 10),    # 4: P2 centre
            ('step2Wait', 0, 999, 5),      # 5: P2 droite
            # Phase 3
            ('step3Speed', 1, 1000, 10),   # 6: P3 gauche
            ('step3Temp', 0, 1500, 10),    # 7: P3 centre
            ('step3Wait', 0, 999, 5),      # 8: P3 droite
            # Cooldown
            ('step4Speed', 1, 1000, 10),   # 9: Cool gauche
            ('step4Target', 0, 1000, 10),  # 10: Cool droite
        ]
        
        if self.selected_param < len(param_config):
            param_name, min_val, max_val, step = param_config[self.selected_param]
            self.params[param_name] += delta * step
            self.params[param_name] = max(min_val, min(max_val, self.params[param_name]))
    
    def update_program(self, current_time, current_temp):
        """Met à jour le programme de cuisson"""
        phase_elapsed_ms = (current_time - self.phase_start_time) * 1000
        
        if self.current_phase == Phase.PHASE_1:
            self.target_temp = self.calculate_target_temp(
                self.target_temp, self.params['step1Temp'], 
                self.params['step1Speed'], phase_elapsed_ms
            )
            if self.check_phase_complete(
                current_temp, self.params['step1Temp'], 
                self.params['step1Wait'], current_time
            ):
                self.current_phase = Phase.PHASE_2
                self.phase_start_time = current_time
                self.plateau_reached = False
                
        elif self.current_phase == Phase.PHASE_2:
            self.target_temp = self.calculate_target_temp(
                self.target_temp, self.params['step2Temp'],
                self.params['step2Speed'], phase_elapsed_ms
            )
            if self.check_phase_complete(
                current_temp, self.params['step2Temp'],
                self.params['step2Wait'], current_time
            ):
                self.current_phase = Phase.PHASE_3
                self.phase_start_time = current_time
                self.plateau_reached = False
                
        elif self.current_phase == Phase.PHASE_3:
            self.target_temp = self.calculate_target_temp(
                self.target_temp, self.params['step3Temp'],
                self.params['step3Speed'], phase_elapsed_ms
            )
            if self.check_phase_complete(
                current_temp, self.params['step3Temp'],
                self.params['step3Wait'], current_time
            ):
                self.current_phase = Phase.PHASE_4_COOLDOWN
                self.phase_start_time = current_time
                self.plateau_reached = False
                
        elif self.current_phase == Phase.PHASE_4_COOLDOWN:
            self.target_temp = self.calculate_cooling_target(
                self.target_temp, self.params['step4Target'],
                self.params['step4Speed'], phase_elapsed_ms
            )
            if current_temp <= self.params['step4Target']:
                self.prog_state = ProgramState.PROG_OFF
                self.current_phase = Phase.PHASE_0
                self.temp_controller.update(0, 0, False)
    
    def calculate_target_temp(self, start_temp, target_temp, speed, elapsed_ms):
        """Calcule la température cible pour le chauffage"""
        calculated_target = start_temp + (speed * (elapsed_ms / 3600000.0))
        return min(calculated_target, target_temp)
    
    def calculate_cooling_target(self, start_temp, target_temp, speed, elapsed_ms):
        """Calcule la température cible pour le refroidissement"""
        calculated_target = start_temp - (speed * (elapsed_ms / 3600000.0))
        return max(calculated_target, target_temp)
    
    def check_phase_complete(self, current_temp, phase_temp, wait_minutes, current_time):
        """Vérifie si la phase est terminée"""
        if not self.plateau_reached and current_temp >= phase_temp - 5 and current_temp >= phase_temp:
            self.plateau_reached = True
            self.plateau_start_time = current_time
        
        if self.plateau_reached:
            plateau_elapsed = current_time - self.plateau_start_time
            if plateau_elapsed >= wait_minutes * 60:
                return True
        
        return False
    
    def update_graph_data(self, temp):
        """Met à jour les données du graphique (toutes les 30 secondes)"""
        current_time = time.time()
        
        # Enregistrer un point toutes les 30 secondes
        if current_time - self.last_graph_update >= 30.0:  # 30 secondes
            self.last_graph_update = current_time
            
            self.graph_actual.append(temp)
            self.graph_target.append(self.target_temp)
            self.graph_time.append(current_time)
            
            # Limiter la taille du graphique (64 points = 32 minutes)
            max_points = 64
            if len(self.graph_actual) > max_points:
                self.graph_actual.pop(0)
                self.graph_target.pop(0)
                self.graph_time.pop(0)
    
    def save_to_eeprom(self):
        """Sauvegarde les paramètres dans un fichier JSON"""
        try:
            with open(self.eeprom_file, 'w') as f:
                json.dump(self.params, f, indent=2)
        except Exception as e:
            print(f"Erreur lors de la sauvegarde EEPROM: {e}")
    
    def load_from_eeprom(self):
        """Charge les paramètres depuis le fichier JSON"""
        try:
            if os.path.exists(self.eeprom_file):
                with open(self.eeprom_file, 'r') as f:
                    loaded_params = json.load(f)
                    self.params.update(loaded_params)
        except Exception as e:
            print(f"Erreur lors du chargement EEPROM: {e}")
            # Utiliser les valeurs par défaut
            self.save_to_eeprom()
    
    def get_display_state(self):
        """Retourne l'état pour l'affichage"""
        return {
            'prog_state': self.prog_state,
            'current_phase': self.current_phase,
            'params': self.params,
            'current_temp': self.kiln.get_temperature(),
            'target_temp': self.target_temp,
            'power_hold': self.temp_controller.get_power_hold(),
            'selected_param': self.selected_param,
            'edit_mode': self.edit_mode,
            'temp_fail_active': self.temp_fail_active,
            'program_start_time': self.program_start_time,
            'phase_start_time': self.phase_start_time,
            'plateau_reached': self.plateau_reached,
            'plateau_start_time': self.plateau_start_time,
            'show_graph': self.show_graph,
            'graph_actual': self.graph_actual,
            'graph_target': self.graph_target,
            'graph_time': self.graph_time,
        }

