"""
Interface utilisateur Pygame pour le simulateur
Affiche l'écran OLED, les contrôles et les graphiques
"""

import pygame
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from matplotlib.backends.backend_agg import FigureCanvasAgg
from config import (
    WINDOW_WIDTH, WINDOW_HEIGHT, OLED_WIDTH, OLED_HEIGHT, 
    DISPLAY_SCALE, Colors, ProgramState
)


class SimulatorUI:
    """Interface utilisateur du simulateur"""
    
    def __init__(self, width=WINDOW_WIDTH, height=WINDOW_HEIGHT):
        pygame.init()
        self.screen = pygame.display.set_mode((width, height))
        pygame.display.set_caption("CalciferAi - Simulateur de Four Céramique")
        
        self.font_large = pygame.font.Font(None, 36)
        self.font_medium = pygame.font.Font(None, 24)
        self.font_small = pygame.font.Font(None, 18)
        
        self.clock = pygame.time.Clock()
        
        # Positions des éléments UI
        self.oled_pos = (20, 20)
        self.controls_pos = (20, 20 + OLED_HEIGHT * DISPLAY_SCALE + 30)
        self.graph_pos = (OLED_WIDTH * DISPLAY_SCALE + 50, 20)
        self.debug_pos = (OLED_WIDTH * DISPLAY_SCALE + 50, 450)
        
        # État des boutons de l'UI
        self.buttons = {}
        self.create_buttons()
        
        # Graphique matplotlib
        self.fig, (self.ax_temp, self.ax_power) = plt.subplots(2, 1, figsize=(7, 5))
        self.fig.patch.set_facecolor('#2e2e2e')
        
    def create_buttons(self):
        """Crée les boutons de contrôle"""
        button_y = self.controls_pos[1]
        
        # Boutons de contrôle
        self.buttons['encoder_left'] = pygame.Rect(20, button_y, 80, 40)
        self.buttons['encoder_right'] = pygame.Rect(110, button_y, 80, 40)
        self.buttons['encoder_click'] = pygame.Rect(200, button_y, 80, 40)
        self.buttons['push_button'] = pygame.Rect(290, button_y, 100, 40)
        
        # Contrôle de vitesse de simulation
        self.buttons['speed_slower'] = pygame.Rect(20, button_y + 60, 50, 30)
        self.buttons['speed_faster'] = pygame.Rect(80, button_y + 60, 50, 30)
        self.buttons['speed_reset'] = pygame.Rect(140, button_y + 60, 50, 30)
        
        # Bouton de reset
        self.buttons['reset'] = pygame.Rect(20, button_y + 110, 100, 40)
    
    def draw(self, oled_display, state, kiln, simulation_speed):
        """
        Dessine l'interface complète
        
        Args:
            oled_display: L'objet OLEDDisplay
            state: État du programme depuis arduino_core
            kiln: Objet KilnPhysics
            simulation_speed: Vitesse de simulation
        """
        self.screen.fill(Colors.BG_COLOR)
        
        # Dessiner l'écran OLED
        self.draw_oled(oled_display)
        
        # Dessiner les contrôles
        self.draw_controls(state, simulation_speed)
        
        # Dessiner les graphiques
        self.draw_graphs(state, kiln)
        
        # Dessiner le panneau de debug
        self.draw_debug_panel(state, kiln)
        
        pygame.display.flip()
        self.clock.tick(60)  # 60 FPS
    
    def draw_oled(self, oled_display):
        """Dessine l'écran OLED agrandi"""
        # Titre
        title = self.font_large.render("Écran OLED SH1106", True, Colors.WHITE)
        self.screen.blit(title, (self.oled_pos[0], self.oled_pos[1] - 30))
        
        # Cadre autour de l'OLED
        oled_rect = pygame.Rect(
            self.oled_pos[0] - 5,
            self.oled_pos[1] - 5,
            OLED_WIDTH * DISPLAY_SCALE + 10,
            OLED_HEIGHT * DISPLAY_SCALE + 10
        )
        pygame.draw.rect(self.screen, Colors.GRAY, oled_rect, 2)
        
        # Afficher l'écran OLED
        oled_surface = oled_display.get_surface(DISPLAY_SCALE)
        self.screen.blit(oled_surface, self.oled_pos)
    
    def draw_controls(self, state, simulation_speed):
        """Dessine les boutons de contrôle"""
        y = self.controls_pos[1]
        
        # Titre
        title = self.font_large.render("Contrôles", True, Colors.WHITE)
        self.screen.blit(title, (20, y - 30))
        
        # Boutons encodeur
        self.draw_button(self.buttons['encoder_left'], "◄", Colors.BLUE)
        self.draw_button(self.buttons['encoder_right'], "►", Colors.BLUE)
        self.draw_button(self.buttons['encoder_click'], "CLICK", Colors.BLUE)
        
        # Bouton push
        color = Colors.GREEN if state['prog_state'] == ProgramState.PROG_OFF else Colors.RED
        self.draw_button(self.buttons['push_button'], "START/STOP", color)
        
        # Légendes
        y_legend = y + 50
        legend = self.font_small.render("Encodeur rotatif", True, Colors.WHITE)
        self.screen.blit(legend, (20, y_legend))
        
        # Contrôles de vitesse
        y_speed = y + 80
        speed_title = self.font_medium.render("Vitesse de simulation", True, Colors.WHITE)
        self.screen.blit(speed_title, (20, y_speed - 20))
        
        self.draw_button(self.buttons['speed_slower'], "◄", Colors.ORANGE)
        self.draw_button(self.buttons['speed_faster'], "►", Colors.ORANGE)
        self.draw_button(self.buttons['speed_reset'], "1x", Colors.ORANGE)
        
        speed_text = self.font_medium.render(f"{simulation_speed:.1f}x", True, Colors.YELLOW)
        self.screen.blit(speed_text, (200, y_speed + 5))
        
        # Bouton reset
        self.draw_button(self.buttons['reset'], "RESET", Colors.RED)
    
    def draw_button(self, rect, text, color):
        """Dessine un bouton"""
        # Fond du bouton
        pygame.draw.rect(self.screen, color, rect)
        pygame.draw.rect(self.screen, Colors.WHITE, rect, 2)
        
        # Texte
        text_surf = self.font_small.render(text, True, Colors.WHITE)
        text_rect = text_surf.get_rect(center=rect.center)
        self.screen.blit(text_surf, text_rect)
    
    def draw_graphs(self, state, kiln):
        """Dessine les graphiques de température et puissance"""
        x, y = self.graph_pos
        
        # Titre
        title = self.font_large.render("Graphiques en temps réel", True, Colors.WHITE)
        self.screen.blit(title, (x, y - 30))
        
        # Effacer les graphiques précédents
        self.ax_temp.clear()
        self.ax_power.clear()
        
        # Graphique de température
        if state['graph_time']:
            # Convertir les temps en minutes relatifs
            start_time = state['graph_time'][0]
            times_minutes = [(t - start_time) / 60 for t in state['graph_time']]
            
            self.ax_temp.plot(times_minutes, state['graph_actual'], 
                            label='Température réelle', color='#ff4444', linewidth=2)
            self.ax_temp.plot(times_minutes, state['graph_target'], 
                            label='Température cible', color='#44ff44', 
                            linestyle='--', linewidth=2)
            self.ax_temp.set_xlabel('Temps (minutes)', color='white')
            self.ax_temp.set_ylabel('Température (°C)', color='white')
            self.ax_temp.set_title('Courbe de température', color='white')
            self.ax_temp.legend(facecolor='#2e2e2e', edgecolor='white', 
                              labelcolor='white')
            self.ax_temp.grid(True, alpha=0.3)
            self.ax_temp.set_facecolor('#1e1e1e')
            self.ax_temp.tick_params(colors='white')
            for spine in self.ax_temp.spines.values():
                spine.set_color('white')
            
            # Graphique de puissance
            power_history = [state['power_hold']] * len(times_minutes)  # Simplifi pour l'instant
            self.ax_power.fill_between(times_minutes, 0, power_history, 
                                      color='#ff8800', alpha=0.6)
            self.ax_power.plot(times_minutes, power_history, 
                             color='#ffaa00', linewidth=2)
            self.ax_power.set_xlabel('Temps (minutes)', color='white')
            self.ax_power.set_ylabel('Puissance (%)', color='white')
            self.ax_power.set_title('Puissance de chauffage', color='white')
            self.ax_power.set_ylim(0, 100)
            self.ax_power.grid(True, alpha=0.3)
            self.ax_power.set_facecolor('#1e1e1e')
            self.ax_power.tick_params(colors='white')
            for spine in self.ax_power.spines.values():
                spine.set_color('white')
        
        # Convertir matplotlib en surface Pygame
        self.fig.tight_layout()
        canvas = FigureCanvasAgg(self.fig)
        canvas.draw()
        renderer = canvas.get_renderer()
        raw_data = renderer.tostring_rgb()
        size = canvas.get_width_height()
        
        surf = pygame.image.fromstring(raw_data, size, "RGB")
        self.screen.blit(surf, (x, y))
    
    def draw_debug_panel(self, state, kiln):
        """Dessine le panneau d'informations de debug"""
        x, y = self.debug_pos
        
        # Titre
        title = self.font_large.render("Informations", True, Colors.WHITE)
        self.screen.blit(title, (x, y - 30))
        
        # Cadre
        panel_rect = pygame.Rect(x - 10, y - 5, 500, 320)
        pygame.draw.rect(self.screen, Colors.PANEL_COLOR, panel_rect)
        pygame.draw.rect(self.screen, Colors.GRAY, panel_rect, 2)
        
        # Informations
        info_lines = [
            f"État: {'EN MARCHE' if state['prog_state'] == ProgramState.PROG_ON else 'ARRÊTÉ'}",
            f"Phase: {self._phase_name(state['current_phase'])}",
            f"",
            f"Température actuelle: {state['current_temp']:.1f}°C",
            f"Température cible: {state['target_temp']:.1f}°C",
            f"Température réelle four: {kiln.get_actual_temperature():.1f}°C",
            f"",
            f"Puissance chauffage: {state['power_hold']}%",
            f"Relais: {'ON' if kiln.relay_state else 'OFF'}",
            f"Taux de chauffage: {kiln.get_heating_rate():.1f}°C/h",
            f"",
            f"Mode édition: {'ÉDITION' if state['edit_mode'] == 1 else 'NAVIGATION'}",
            f"Paramètre sélectionné: {state['selected_param']}",
        ]
        
        for i, line in enumerate(info_lines):
            color = Colors.GREEN if i in [0, 3, 7] else Colors.WHITE
            text = self.font_small.render(line, True, color)
            self.screen.blit(text, (x, y + i * 22))
    
    def _phase_name(self, phase):
        """Retourne le nom de la phase"""
        names = {
            0: "Arrêt (Phase 0)",
            1: "Phase 1 - Chauffage",
            2: "Phase 2 - Chauffage",
            3: "Phase 3 - Chauffage",
            4: "Phase 4 - Refroidissement"
        }
        return names.get(phase, "Inconnue")
    
    def handle_events(self):
        """
        Gère les événements Pygame
        
        Returns:
            dict: Actions à effectuer
        """
        actions = {
            'quit': False,
            'encoder_left': False,
            'encoder_right': False,
            'encoder_click': False,
            'encoder_long_press': False,
            'push_button': False,
            'speed_slower': False,
            'speed_faster': False,
            'speed_reset': False,
            'reset': False,
        }
        
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                actions['quit'] = True
            
            elif event.type == pygame.MOUSEBUTTONDOWN:
                mouse_pos = pygame.mouse.get_pos()
                
                # Vérifier tous les boutons
                for button_name, button_rect in self.buttons.items():
                    if button_rect.collidepoint(mouse_pos):
                        actions[button_name] = True
            
            elif event.type == pygame.KEYDOWN:
                # Raccourcis clavier
                if event.key == pygame.K_LEFT:
                    actions['encoder_left'] = True
                elif event.key == pygame.K_RIGHT:
                    actions['encoder_right'] = True
                elif event.key == pygame.K_RETURN or event.key == pygame.K_SPACE:
                    actions['encoder_click'] = True
                elif event.key == pygame.K_s:
                    actions['push_button'] = True
                elif event.key == pygame.K_r:
                    actions['reset'] = True
                elif event.key == pygame.K_MINUS or event.key == pygame.K_KP_MINUS:
                    actions['speed_slower'] = True
                elif event.key == pygame.K_PLUS or event.key == pygame.K_KP_PLUS or event.key == pygame.K_EQUALS:
                    actions['speed_faster'] = True
                elif event.key == pygame.K_0:
                    actions['speed_reset'] = True
        
        return actions
    
    def cleanup(self):
        """Nettoie les ressources"""
        plt.close(self.fig)
        pygame.quit()

