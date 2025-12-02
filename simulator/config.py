"""
Configuration et constantes pour le simulateur CalciferAi
"""

# ===== PARAMÈTRES PID =====
KP = 2.0
KI = 0.5
KD = 0.0
MAX_POWER_CHANGE = 10.0

# ===== PARAMÈTRES PWM =====
CYCLE_LENGTH = 1000  # millisecondes

# ===== PINS DEFINITION (pour référence) =====
PIN_ENCODER_CLK = 2
PIN_ENCODER_DT = 3
PIN_ENCODER_SW = 4
PIN_PUSH_BUTTON = 5
PIN_RELAY = 6
PIN_MAX_CS = 10
PIN_LED = "A1"

# ===== ÉTATS DU PROGRAMME =====
class ProgramState:
    PROG_OFF = 0
    PROG_ON = 1

class Phase:
    PHASE_0 = 0
    PHASE_1 = 1
    PHASE_2 = 2
    PHASE_3 = 3
    PHASE_4_COOLDOWN = 4

class EditMode:
    NAV_MODE = 0
    EDIT_MODE = 1

# ===== PARAMÈTRES DE CUISSON PAR DÉFAUT =====
DEFAULT_FIRING_PARAMS = {
    'step1Temp': 100,
    'step1Speed': 50,
    'step1Wait': 5,
    'step2Temp': 570,
    'step2Speed': 250,
    'step2Wait': 15,
    'step3Temp': 1100,
    'step3Speed': 200,
    'step3Wait': 20,
    'step4Speed': 150,
    'step4Target': 200
}

# ===== PARAMÈTRES DE SIMULATION PHYSIQUE =====
# Capacité thermique du four (J/°C)
KILN_THERMAL_MASS = 50000.0  # Four de taille moyenne

# Puissance de chauffage maximale (Watts)
MAX_HEATING_POWER = 3000.0

# Coefficient de perte thermique (W/°C)
HEAT_LOSS_COEFFICIENT = 15.0

# Température ambiante (°C)
AMBIENT_TEMP = 20.0

# Délai de réponse du thermocouple (secondes)
THERMOCOUPLE_DELAY = 2.0

# ===== PARAMÈTRES D'AFFICHAGE =====
# Dimensions de l'écran OLED
OLED_WIDTH = 128
OLED_HEIGHT = 64

# Facteur de zoom pour l'affichage
DISPLAY_SCALE = 6

# Dimensions de la fenêtre
WINDOW_WIDTH = 1200
WINDOW_HEIGHT = 800

# Rafraîchissement de l'écran (ms)
DISPLAY_UPDATE_INTERVAL = 100

# ===== COULEURS =====
class Colors:
    BLACK = (0, 0, 0)
    WHITE = (255, 255, 255)
    GREEN = (0, 255, 0)
    RED = (255, 0, 0)
    BLUE = (0, 150, 255)
    GRAY = (128, 128, 128)
    DARK_GRAY = (50, 50, 50)
    YELLOW = (255, 255, 0)
    ORANGE = (255, 165, 0)
    
    # Couleurs de l'interface
    BG_COLOR = (30, 30, 30)
    PANEL_COLOR = (45, 45, 45)
    TEXT_COLOR = WHITE
    BUTTON_COLOR = (70, 70, 70)
    BUTTON_HOVER = (90, 90, 90)
    BUTTON_ACTIVE = (110, 110, 110)

# ===== SIMULATION =====
SIMULATION_SPEED = 1.0  # Vitesse de simulation (1.0 = temps réel, 10.0 = 10x plus rapide)
MAX_SIMULATION_SPEED = 100.0
MIN_SIMULATION_SPEED = 0.1

