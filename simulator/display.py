"""
Rendu de l'écran OLED virtuel
Transposition de display.cpp avec rendu bitmap
"""

import pygame
from config import OLED_WIDTH, OLED_HEIGHT, Colors


class OLEDDisplay:
    """Émule un écran OLED SH1106 128x64"""
    
    def __init__(self):
        # Buffer de pixels (True = allumé, False = éteint)
        self.buffer = [[False for _ in range(OLED_WIDTH)] for _ in range(OLED_HEIGHT)]
        self.draw_color = True  # True = dessiner en blanc, False = effacer
        
    def clear_buffer(self):
        """Efface le buffer"""
        self.buffer = [[False for _ in range(OLED_WIDTH)] for _ in range(OLED_HEIGHT)]
    
    def set_draw_color(self, color):
        """Définit la couleur de dessin (True = blanc, False = noir)"""
        self.draw_color = color
    
    def draw_pixel(self, x, y):
        """Dessine un pixel"""
        if 0 <= x < OLED_WIDTH and 0 <= y < OLED_HEIGHT:
            self.buffer[y][x] = self.draw_color
    
    def draw_str(self, x, y, text):
        """Dessine une chaîne de caractères (police 5x7)"""
        for i, char in enumerate(text):
            self.draw_char(x + i * 6, y, char)
    
    def draw_char(self, x, y, char):
        """Dessine un caractère avec une police simple 5x7"""
        # Police bitmap simplifiée
        font = self._get_char_bitmap(char)
        for row in range(7):
            for col in range(5):
                if font[row] & (1 << (4 - col)):
                    self.draw_pixel(x + col, y - 7 + row)
    
    def draw_box(self, x, y, w, h):
        """Dessine un rectangle plein"""
        for dy in range(h):
            for dx in range(w):
                self.draw_pixel(x + dx, y + dy)
    
    def draw_frame(self, x, y, w, h):
        """Dessine un rectangle vide (contour)"""
        # Haut et bas
        for dx in range(w):
            self.draw_pixel(x + dx, y)
            self.draw_pixel(x + dx, y + h - 1)
        # Gauche et droite
        for dy in range(h):
            self.draw_pixel(x, y + dy)
            self.draw_pixel(x + w - 1, y + dy)
    
    def draw_line(self, x0, y0, x1, y1):
        """Dessine une ligne (algorithme de Bresenham)"""
        dx = abs(x1 - x0)
        dy = abs(y1 - y0)
        sx = 1 if x0 < x1 else -1
        sy = 1 if y0 < y1 else -1
        err = dx - dy
        
        while True:
            self.draw_pixel(x0, y0)
            
            if x0 == x1 and y0 == y1:
                break
            
            e2 = 2 * err
            if e2 > -dy:
                err -= dy
                x0 += sx
            if e2 < dx:
                err += dx
                y0 += sy
    
    def get_surface(self, scale=1):
        """
        Convertit le buffer en surface Pygame
        
        Args:
            scale: Facteur d'agrandissement
            
        Returns:
            pygame.Surface
        """
        surface = pygame.Surface((OLED_WIDTH * scale, OLED_HEIGHT * scale))
        surface.fill(Colors.BLACK)
        
        for y in range(OLED_HEIGHT):
            for x in range(OLED_WIDTH):
                if self.buffer[y][x]:
                    pygame.draw.rect(
                        surface,
                        Colors.WHITE,
                        (x * scale, y * scale, scale, scale)
                    )
        
        return surface
    
    def _get_char_bitmap(self, char):
        """
        Retourne le bitmap d'un caractère (police 5x7)
        Simplifié pour les caractères les plus courants
        """
        # Font 5x7 bitmap (chaque ligne est un byte)
        font_data = {
            ' ': [0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00],
            '0': [0x0E, 0x11, 0x13, 0x15, 0x19, 0x11, 0x0E],
            '1': [0x04, 0x0C, 0x04, 0x04, 0x04, 0x04, 0x0E],
            '2': [0x0E, 0x11, 0x01, 0x02, 0x04, 0x08, 0x1F],
            '3': [0x1F, 0x02, 0x04, 0x02, 0x01, 0x11, 0x0E],
            '4': [0x02, 0x06, 0x0A, 0x12, 0x1F, 0x02, 0x02],
            '5': [0x1F, 0x10, 0x1E, 0x01, 0x01, 0x11, 0x0E],
            '6': [0x06, 0x08, 0x10, 0x1E, 0x11, 0x11, 0x0E],
            '7': [0x1F, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08],
            '8': [0x0E, 0x11, 0x11, 0x0E, 0x11, 0x11, 0x0E],
            '9': [0x0E, 0x11, 0x11, 0x0F, 0x01, 0x02, 0x0C],
            'A': [0x0E, 0x11, 0x11, 0x11, 0x1F, 0x11, 0x11],
            'C': [0x0E, 0x11, 0x10, 0x10, 0x10, 0x11, 0x0E],
            'E': [0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x1F],
            'F': [0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x10],
            'H': [0x11, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11],
            'N': [0x11, 0x19, 0x19, 0x15, 0x13, 0x13, 0x11],
            'O': [0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E],
            'P': [0x1E, 0x11, 0x11, 0x1E, 0x10, 0x10, 0x10],
            'R': [0x1E, 0x11, 0x11, 0x1E, 0x14, 0x12, 0x11],
            'T': [0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04],
            'W': [0x11, 0x11, 0x11, 0x15, 0x15, 0x1B, 0x11],
            'a': [0x00, 0x00, 0x0E, 0x01, 0x0F, 0x11, 0x0F],
            'c': [0x00, 0x00, 0x0E, 0x10, 0x10, 0x11, 0x0E],
            'd': [0x01, 0x01, 0x0F, 0x11, 0x11, 0x11, 0x0F],
            'e': [0x00, 0x00, 0x0E, 0x11, 0x1F, 0x10, 0x0E],
            'f': [0x06, 0x09, 0x08, 0x1C, 0x08, 0x08, 0x08],
            'g': [0x00, 0x0F, 0x11, 0x11, 0x0F, 0x01, 0x0E],
            'h': [0x10, 0x10, 0x16, 0x19, 0x11, 0x11, 0x11],
            'i': [0x04, 0x00, 0x0C, 0x04, 0x04, 0x04, 0x0E],
            'l': [0x0C, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0E],
            'm': [0x00, 0x00, 0x1A, 0x15, 0x15, 0x15, 0x11],
            'n': [0x00, 0x00, 0x16, 0x19, 0x11, 0x11, 0x11],
            'o': [0x00, 0x00, 0x0E, 0x11, 0x11, 0x11, 0x0E],
            'p': [0x00, 0x00, 0x1E, 0x11, 0x1E, 0x10, 0x10],
            'r': [0x00, 0x00, 0x16, 0x19, 0x10, 0x10, 0x10],
            's': [0x00, 0x00, 0x0F, 0x10, 0x0E, 0x01, 0x1E],
            't': [0x08, 0x08, 0x1C, 0x08, 0x08, 0x09, 0x06],
            'u': [0x00, 0x00, 0x11, 0x11, 0x11, 0x13, 0x0D],
            'w': [0x00, 0x00, 0x11, 0x11, 0x15, 0x15, 0x0A],
            ':': [0x00, 0x0C, 0x0C, 0x00, 0x0C, 0x0C, 0x00],
            '>': [0x08, 0x04, 0x02, 0x01, 0x02, 0x04, 0x08],
            '<': [0x02, 0x04, 0x08, 0x10, 0x08, 0x04, 0x02],
            '-': [0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00],
            '/': [0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x00],
            '%': [0x18, 0x19, 0x02, 0x04, 0x08, 0x13, 0x03],
            '°': [0x06, 0x09, 0x09, 0x06, 0x00, 0x00, 0x00],
            ',': [0x00, 0x00, 0x00, 0x00, 0x0C, 0x04, 0x08],
            '.': [0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C],
            '!': [0x04, 0x04, 0x04, 0x04, 0x00, 0x00, 0x04],
        }
        
        return font_data.get(char, [0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00])


class DisplayRenderer:
    """Gère le rendu de l'interface utilisateur du programme"""
    
    def __init__(self):
        self.oled = OLEDDisplay()
        
    def draw_prog_off_screen(self, params, current_temp, power_hold, selected_param, edit_mode, temp_fail_active):
        """Dessine l'écran en mode PROG_OFF"""
        self.oled.clear_buffer()
        self.oled.set_draw_color(True)
        
        # Température actuelle - Grande en haut (police 9x15 simulée avec double hauteur)
        temp_str = f"{int(current_temp)}C"
        self._draw_large_text(0, 12, temp_str)
        
        # Avertissement température ou pourcentage de puissance
        if temp_fail_active:
            self.oled.draw_str(90, 10, "WARN")
        else:
            self.oled.draw_str(90, 10, f"{power_hold}%")
        
        # Phase 1 - Ordre: gauche à droite
        self.oled.draw_str(0, 24, "P1:")
        self._draw_param(18, 24, f"{params['step1Speed']}C/h", 0, selected_param, edit_mode)    # Gauche
        self._draw_param(60, 24, f">{params['step1Temp']}C", 1, selected_param, edit_mode)      # Centre
        self._draw_param(105, 24, f"{params['step1Wait']}m", 2, selected_param, edit_mode)      # Droite
        
        # Phase 2 - Ordre: gauche à droite
        self.oled.draw_str(0, 36, "P2:")
        self._draw_param(18, 36, f"{params['step2Speed']}C/h", 3, selected_param, edit_mode)    # Gauche
        self._draw_param(60, 36, f">{params['step2Temp']}C", 4, selected_param, edit_mode)      # Centre
        self._draw_param(105, 36, f"{params['step2Wait']}m", 5, selected_param, edit_mode)      # Droite
        
        # Phase 3 - Ordre: gauche à droite
        self.oled.draw_str(0, 48, "P3:")
        self._draw_param(18, 48, f"{params['step3Speed']}C/h", 6, selected_param, edit_mode)    # Gauche
        self._draw_param(60, 48, f">{params['step3Temp']}C", 7, selected_param, edit_mode)      # Centre
        self._draw_param(105, 48, f"{params['step3Wait']}m", 8, selected_param, edit_mode)      # Droite
        
        # Cooldown - Ordre: gauche à droite
        self.oled.draw_str(0, 60, "Cool:")
        self._draw_param(30, 60, f"{params['step4Speed']}C/h", 9, selected_param, edit_mode)    # Gauche
        self._draw_param(80, 60, f"<{params['step4Target']}C", 10, selected_param, edit_mode)   # Droite
        
        return self.oled
    
    def draw_prog_on_screen(self, params, current_temp, target_temp, power_hold, 
                           current_phase, phase_start_time, plateau_reached, plateau_start_time,
                           temp_fail_active):
        """Dessine l'écran en mode PROG_ON"""
        self.oled.clear_buffer()
        self.oled.set_draw_color(True)
        
        # Titre
        self.oled.draw_str(0, 7, "ON")
        
        # Avertissement température
        if temp_fail_active:
            self.oled.draw_str(50, 7, "WARN")
        
        # Temps écoulé
        import time
        elapsed = int((time.time() - phase_start_time) * 1000)
        minutes = elapsed // 60000
        seconds = (elapsed % 60000) // 1000
        
        if minutes < 60:
            time_str = f"{minutes:02d}:{seconds:02d}"
        else:
            hours = minutes // 60
            minutes = minutes % 60
            time_str = f"{hours:02d}:{minutes:02d}"
        
        self.oled.draw_str(75, 7, time_str)
        
        # Phase 1
        if current_phase > 1:
            self.oled.set_draw_color(False)  # Dim
        text = f"P1:{params['step1Speed']}>{params['step1Temp']},{params['step1Wait']}m"
        self.oled.draw_str(0, 17, text[:20])  # Limiter la longueur
        self.oled.set_draw_color(True)
        
        # Phase 2
        if current_phase == 1 or current_phase > 2:
            self.oled.set_draw_color(False)
        text = f"P2:{params['step2Speed']}>{params['step2Temp']},{params['step2Wait']}m"
        self.oled.draw_str(0, 27, text[:20])
        self.oled.set_draw_color(True)
        
        # Phase 3
        if current_phase < 3 or current_phase > 3:
            self.oled.set_draw_color(False)
        text = f"P3:{params['step3Speed']}>{params['step3Temp']},{params['step3Wait']}m"
        self.oled.draw_str(0, 37, text[:20])
        self.oled.set_draw_color(True)
        
        # Cooldown
        if current_phase != 4:
            self.oled.set_draw_color(False)
        text = f"Cool:{params['step4Speed']}<{params['step4Target']}C"
        self.oled.draw_str(0, 47, text[:20])
        self.oled.set_draw_color(True)
        
        # Température actuelle et cible
        self.oled.draw_str(0, 57, f"T:{int(current_temp)}->{int(target_temp)}C"[:18])
        
        # État du relais
        self.oled.draw_str(90, 57, f"ON,{power_hold}%")
        
        return self.oled
    
    def draw_critical_error(self):
        """Dessine l'écran d'erreur critique"""
        self.oled.clear_buffer()
        self.oled.set_draw_color(True)
        
        self.oled.draw_str(0, 10, "ERROR!")
        self.oled.draw_str(0, 25, "Temp fail 2min")
        self.oled.draw_str(0, 40, "Heat stopped")
        self.oled.draw_str(0, 55, "Press button")
        
        return self.oled
    
    def _draw_param(self, x, y, text, param_index, selected_param, edit_mode):
        """Dessine un paramètre avec sélection/édition"""
        from config import EditMode
        
        if selected_param == param_index and edit_mode == EditMode.NAV_MODE:
            # Cadre d'un pixel pour la sélection (navigation)
            self.oled.draw_str(x, y, text)
            width = len(text) * 6
            self.oled.draw_frame(x - 1, y - 9, width + 2, 11)
        elif selected_param == param_index and edit_mode == EditMode.EDIT_MODE:
            # Couleurs inversées pour l'édition (négatif)
            width = len(text) * 6
            self.oled.set_draw_color(True)
            self.oled.draw_box(x - 1, y - 9, width + 2, 11)
            self.oled.set_draw_color(False)
            self.oled.draw_str(x, y, text)
            self.oled.set_draw_color(True)
        else:
            self.oled.draw_str(x, y, text)
    
    def _draw_large_text(self, x, y, text):
        """Dessine du texte en grande taille (approximation de 9x15)"""
        # Utilise simplement la police normale pour l'instant
        # Dans une vraie implémentation, on doublerait les pixels
        self.oled.draw_str(x, y, text)
    
    def draw_graph(self, graph_actual, graph_target, current_temp, target_temp, current_phase, program_start_time):
        """Dessine le graphique des températures"""
        self.oled.clear_buffer()
        self.oled.set_draw_color(True)
        
        if len(graph_actual) < 2:
            self.oled.draw_str(20, 32, "Pas de donnees")
            return self.oled
        
        # Calculer le temps écoulé
        import time
        total_elapsed = int((time.time() - program_start_time) * 1000)
        hours = total_elapsed // 3600000
        minutes = (total_elapsed % 3600000) // 60000
        
        # En-tête: Phase + Temps écoulé
        phase_names = ["", "P1", "P2", "P3", "Cool"]
        header = f"{phase_names[current_phase]} T+{hours}h{minutes:02d}"
        self.oled.draw_str(0, 8, header)
        
        # Températures actuelles
        temp_str = f"{int(current_temp)}->{int(target_temp)}C"
        self.oled.draw_str(70, 8, temp_str)
        
        # Trouver min et max pour l'échelle
        all_temps = graph_actual + graph_target
        min_temp = min(all_temps)
        max_temp = max(all_temps)
        
        # Ajouter 10% de marge
        temp_range = max_temp - min_temp
        min_temp -= temp_range * 0.1
        max_temp += temp_range * 0.1
        if temp_range < 10:
            min_temp -= 5
            max_temp += 5
            temp_range = max_temp - min_temp
        
        # Zone du graphe
        graph_height = 48
        graph_top = 14
        graph_left = 20
        graph_width = 107  # 127 - 20
        
        # Dessiner les axes
        self.oled.draw_line(graph_left, graph_top, graph_left, graph_top + graph_height)
        self.oled.draw_line(graph_left, graph_top + graph_height, 127, graph_top + graph_height)
        
        # Labels de température
        self.oled.draw_str(0, graph_top + 5, f"{int(max_temp)}")
        self.oled.draw_str(0, graph_top + graph_height - 2, f"{int(min_temp)}")
        
        # Dessiner les données
        points_to_show = min(len(graph_actual), graph_width)
        start_index = max(0, len(graph_actual) - points_to_show)
        
        for i in range(points_to_show - 1):
            index = start_index + i
            x1 = graph_left + 1 + i
            x2 = x1 + 1
            
            # Température cible (ligne pointillée)
            y1_target = graph_top + graph_height - int((graph_target[index] - min_temp) / temp_range * graph_height)
            y2_target = graph_top + graph_height - int((graph_target[index + 1] - min_temp) / temp_range * graph_height)
            
            if i % 3 == 0:  # Pointillé
                self.oled.draw_line(x1, y1_target, x2, y2_target)
            
            # Température réelle (ligne pleine)
            y1_actual = graph_top + graph_height - int((graph_actual[index] - min_temp) / temp_range * graph_height)
            y2_actual = graph_top + graph_height - int((graph_actual[index + 1] - min_temp) / temp_range * graph_height)
            
            self.oled.draw_line(x1, y1_actual, x2, y2_actual)
        
        return self.oled

