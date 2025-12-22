# CalciferAi Build Guide

## 1. Shopping List (~106€)

### Main Components
| Component | Price |
|-----------|-------|
| Arduino Uno | 5€ |
| OLED Display SH1106 128x64 (I2C) | 3€ |
| Adafruit MAX31856 | 7€ |
| Rotary Encoder KY-040 | 3€ |
| Push Button | 0.50€ |
| Control LED 5mm | 2€ |
| SSR Relay SSR-25DA | 7€ |
| Breadboard 170 points | 1€ |
| Transistor 2N2222A | 0.30€ |
| 1kΩ Resistors (x10) | 0.50€ |
| 10kΩ Resistors (x10) | 0.50€ |
| Dupont Wires (kit 40 pcs) | 3€ |
| 9V Power Supply for Arduino | 2€ |
| Aluminium Enclosure | 18€ |


### Thermocouple
| Component | Price |
|-----------|-------|
| Type S Thermocouple | 20€ |
| Thermocouple Wire | 5€ |


---

## 2. Wiring

### Power Supply (Breadboard)
- Arduino **5V** → Breadboard + rail
- Arduino **GND** → Breadboard - rail

### OLED Display
| Display | Arduino |
|---------|---------|
| VCC | 5V (+ rail) |
| GND | GND (- rail) |
| SCL | A5 |
| SDA | A4 |

### MAX31856 (Thermocouple)
| MAX31856 | Arduino |
|----------|---------|
| VIN | 5V (+ rail) |
| GND | GND (- rail) |
| SCK | D13 |
| SDO | D12 |
| SDI | D11 |
| CS | D10 |
| DRDY | D9 |
| FLT | D8 |
| T+ | Thermocouple + |
| T- | Thermocouple - |

### Rotary Encoder
| Encoder | Arduino |
|---------|---------|
| VCC | 5V (+ rail) |
| GND | GND (- rail) |
| CLK | D2 |
| DT | D3 |
| SW | D4 |

### Push Button
- One leg → **D5**
- Other leg → **GND** (- rail)

### Control LED
- Anode (+) → **A1**
- Cathode (-) → **GND** (- rail)

### SSR Relay (with 2N2222A transistor protection)
```
+5V Arduino
     │
     ▼
SSR (+) input
     │
SSR (-) input
     │
     ▼
Collector (C)
     │
     │
   Base (B)
   NPN Transistor  ─── [R1 1kΩ]── D6 Arduino
     │              │
     │       [R_pulldown 10kΩ]
     │              │
Emitter (E)        │
     │              │
     └──────────────┘
     │
     ▼
   GND Arduino
```

**⚠️ 2N2222A Pinout**: E-B-C (Emitter-Base-Collector) - Check the datasheet!

---

## 3. Upload the Program

### Step 0: Choose Features (optional)
In `lucia/definitions.h`, enable/disable by adding or removing `//`:
```
#define ENABLE_LOGGING  // Serial logging (~250 bytes)
#define ENABLE_GRAPH    // Temperature graph (~800 bytes)
```
⚠️ **Arduino memory is limited**: enable only one option at a time if the program doesn't compile.

### Step 1: Install Arduino IDE
1. Download from [arduino.cc/software](https://www.arduino.cc/software)
2. Install and launch

### Step 2: Install Libraries
In Arduino IDE: **Tools → Manage Libraries**

Search and install:
- `U8g2` (for the OLED display)
- `Adafruit MAX31856` (for the thermocouple)
- `Encoder` (for the rotary encoder)

### Step 3: Configure the Board
- **Tools → Board** → Arduino Uno
- **Tools → Port** → Select Arduino port (e.g.: COM3 or /dev/ttyUSB0)

### Step 4: Open and Upload
1. **File → Open** → Navigate to `lucia/lucia.ino`
2. Click **→** (Upload) or Ctrl+U
3. Wait for "Done uploading"

---

**You're ready!** The display should light up and show the main menu.
