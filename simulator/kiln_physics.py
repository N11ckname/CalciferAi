"""
Simulation physique du four céramique
Modélise l'inertie thermique, le chauffage et le refroidissement
"""

import math
from config import (
    KILN_THERMAL_MASS,
    MAX_HEATING_POWER,
    HEAT_LOSS_COEFFICIENT,
    AMBIENT_TEMP,
    THERMOCOUPLE_DELAY
)


class KilnPhysics:
    """Simule le comportement thermique d'un four céramique"""
    
    def __init__(self):
        self.temperature = AMBIENT_TEMP  # Température actuelle du four (°C)
        self.thermocouple_reading = AMBIENT_TEMP  # Lecture du thermocouple avec délai
        self.heating_power = 0.0  # Puissance de chauffage actuelle (0-100%)
        self.relay_state = False  # État du relais
        
        # Variables pour le délai du thermocouple
        self._thermocouple_buffer = []
        self._buffer_size = int(THERMOCOUPLE_DELAY * 10)  # 10 samples par seconde
        
    def update(self, dt, power_percentage, relay_on):
        """
        Met à jour la simulation physique
        
        Args:
            dt: Delta time en secondes
            power_percentage: Pourcentage de puissance (0-100)
            relay_on: État du relais (True/False)
        """
        self.relay_state = relay_on
        self.heating_power = power_percentage
        
        # Calcul de la puissance de chauffage effective
        if relay_on:
            heating_watts = (power_percentage / 100.0) * MAX_HEATING_POWER
        else:
            heating_watts = 0.0
        
        # Calcul des pertes thermiques (loi de Newton)
        # Plus le four est chaud, plus il perd de chaleur
        temp_diff = self.temperature - AMBIENT_TEMP
        heat_loss_watts = HEAT_LOSS_COEFFICIENT * temp_diff
        
        # Pertes par radiation (Stefan-Boltzmann simplifié)
        # Devient significatif à haute température
        if self.temperature > 500:
            radiation_factor = ((self.temperature + 273.15) / 773.15) ** 4
            heat_loss_watts += HEAT_LOSS_COEFFICIENT * temp_diff * radiation_factor * 0.1
        
        # Bilan énergétique
        net_heat_watts = heating_watts - heat_loss_watts
        
        # Variation de température (Q = m * c * ΔT)
        delta_temp = (net_heat_watts * dt) / KILN_THERMAL_MASS
        
        # Mise à jour de la température
        self.temperature += delta_temp
        
        # La température ne peut pas descendre en dessous de l'ambiant
        if self.temperature < AMBIENT_TEMP:
            self.temperature = AMBIENT_TEMP
        
        # Simulation du délai du thermocouple
        self._thermocouple_buffer.append(self.temperature)
        if len(self._thermocouple_buffer) > self._buffer_size:
            self._thermocouple_buffer.pop(0)
        
        # La lecture du thermocouple est la moyenne du buffer (lissage)
        if self._thermocouple_buffer:
            self.thermocouple_reading = sum(self._thermocouple_buffer) / len(self._thermocouple_buffer)
        else:
            self.thermocouple_reading = self.temperature
    
    def get_temperature(self):
        """Retourne la température lue par le thermocouple"""
        return self.thermocouple_reading
    
    def get_actual_temperature(self):
        """Retourne la température réelle du four (sans délai)"""
        return self.temperature
    
    def get_heating_rate(self):
        """Retourne le taux de chauffage actuel en °C/h"""
        if len(self._thermocouple_buffer) < 2:
            return 0.0
        
        # Calcul sur les 10 dernières secondes
        samples = min(10, len(self._thermocouple_buffer))
        if samples < 2:
            return 0.0
        
        old_temp = self._thermocouple_buffer[-samples]
        new_temp = self._thermocouple_buffer[-1]
        time_diff = samples / 10.0  # secondes
        
        rate_per_second = (new_temp - old_temp) / time_diff
        rate_per_hour = rate_per_second * 3600.0
        
        return rate_per_hour
    
    def is_heating(self):
        """Retourne True si le four chauffe actuellement"""
        return self.relay_state and self.heating_power > 0
    
    def get_power_percentage(self):
        """Retourne le pourcentage de puissance actuel"""
        return self.heating_power
    
    def reset(self):
        """Réinitialise le four à température ambiante"""
        self.temperature = AMBIENT_TEMP
        self.thermocouple_reading = AMBIENT_TEMP
        self.heating_power = 0.0
        self.relay_state = False
        self._thermocouple_buffer = []

