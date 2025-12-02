"""
Contrôle de température et PID
Transposition de temperature.cpp
"""

import time
from config import KP, KI, KD, MAX_POWER_CHANGE, CYCLE_LENGTH


class TemperatureController:
    """Contrôleur PID pour la température du four"""
    
    def __init__(self):
        self.power_hold = 0.0  # 0-100%
        self.power_on = False
        self.pwm_cycle_start = 0
        
        # Variables PID
        self.integral_error = 0
        self.last_error = 0
        self.last_power_hold_scaled = 0
        self.last_pid_update = time.time()
        
    def init(self):
        """Initialisation du contrôleur"""
        self.power_hold = 0.0
        self.power_on = False
        self.integral_error = 0
        self.last_error = 0
        self.last_power_hold_scaled = 0
        self.last_pid_update = time.time()
        self.pwm_cycle_start = time.time()
        
    def reset_pid(self):
        """Réinitialise les variables PID"""
        self.integral_error = 0
        self.last_error = 0
        self.last_power_hold_scaled = 0
        self.last_pid_update = time.time()
    
    def update(self, current_temp, target_temp, enabled):
        """
        Met à jour le contrôle de température
        
        Args:
            current_temp: Température actuelle en °C
            target_temp: Température cible en °C
            enabled: True si le contrôle est activé
            
        Returns:
            tuple: (relay_state, power_percentage)
        """
        current_time = time.time()
        
        if not enabled:
            self.power_hold = 0
            self.power_on = False
            self.integral_error = 0
            self.last_error = 0
            return False, 0
        
        # Calcul de l'erreur (scaled by 100)
        error = int((target_temp - current_temp) * 100)
        
        # Calcul du temps écoulé
        dt = current_time - self.last_pid_update
        if dt <= 0:
            dt = 0.001
        self.last_pid_update = current_time
        
        # Calcul PID
        proportional = KP * (error / 100.0)
        self.integral_error += int(error * dt)
        
        # Anti-windup: limiter le terme intégral
        max_integral = int(10000.0 / KI) if KI > 0 else 100000
        if self.integral_error > max_integral:
            self.integral_error = max_integral
        if self.integral_error < -max_integral:
            self.integral_error = -max_integral
        
        integral = KI * (self.integral_error / 100.0)
        
        # Calcul de la nouvelle puissance (scaled 0-10000)
        new_power_hold_scaled = int((proportional + integral) * 100)
        
        # Limitation du taux de changement de puissance
        power_change = new_power_hold_scaled - self.last_power_hold_scaled
        if power_change > MAX_POWER_CHANGE * 100:
            new_power_hold_scaled = self.last_power_hold_scaled + int(MAX_POWER_CHANGE * 100)
        elif power_change < -MAX_POWER_CHANGE * 100:
            new_power_hold_scaled = self.last_power_hold_scaled - int(MAX_POWER_CHANGE * 100)
        
        # Contraindre la sortie (0-10000)
        if new_power_hold_scaled > 10000:
            new_power_hold_scaled = 10000
        if new_power_hold_scaled < 0:
            new_power_hold_scaled = 0
        
        self.power_hold = new_power_hold_scaled / 100.0
        self.last_power_hold_scaled = new_power_hold_scaled
        self.last_error = error
        
        # Contrôle PWM logiciel
        cycle_elapsed_ms = (current_time - self.pwm_cycle_start) * 1000
        
        if cycle_elapsed_ms >= CYCLE_LENGTH:
            # Nouveau cycle
            self.pwm_cycle_start = current_time
            cycle_elapsed_ms = 0
        
        # Calcul du temps ON pour ce cycle
        on_time_ms = (new_power_hold_scaled * CYCLE_LENGTH) / 10000
        
        # Déterminer l'état du relais
        if cycle_elapsed_ms < on_time_ms and new_power_hold_scaled > 0:
            self.power_on = True
        else:
            self.power_on = False
        
        return self.power_on, int(self.power_hold)
    
    def get_power_hold(self):
        """Retourne le pourcentage de puissance actuel"""
        return int(self.power_hold)
    
    def is_relay_on(self):
        """Retourne l'état du relais"""
        return self.power_on

