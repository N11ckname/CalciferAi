# CalciferAi - User Guide

## 🎯 Overview

CalciferAi is an automatic control system for ceramic kilns. It allows you to program firing cycles with 3 heating phases followed by controlled cooling.

## 🎮 Controls

### Buttons and Encoder
- **Rotary encoder**: Navigate and modify values
- **Encoder click**: Confirm/edit a parameter
- **Push button**: Start/Stop the program

### Indicator LED
- **LED on**: The kiln is heating

## 📋 Main Screen (Stop Mode)

The screen displays your firing program parameters:

**Screen simulation:**
```
┌────────────────────────────────┐
│ 25C              Settings      │
│ P1: 50C/h >100C 5m             │
│ P2: 250C/h >570C 15m           │
│ P3: 200C/h >1100C 20m          │
│ Cool: 150C/h <200C             │
└────────────────────────────────┘
```
*Note: The selected parameter appears with a frame*

### Phase 1, 2 and 3 (Temperature Rise)
- **Rate**: Heating rate in °C/h (10 to 1000)
- **Temperature**: Target temperature in °C (0 to 1500)
- **Duration**: Hold time at plateau in minutes (0 to 999)

### Phase 4 (Cooling)
- **Rate**: Cooling rate in °C/h (1 to 1000)
- **Temperature**: End-of-cycle temperature in °C (0 to 1000)

## ⚙️ Program Configuration

### 1. Navigating between parameters
1. Turn the encoder to select a parameter (it displays in reverse video)
2. Parameters scroll in order: Settings → rate → temperature → duration for each phase


## 🚀 Starting a Program

1. Verify that all your parameters are correct
2. **Press the push button** to start
3. The screen switches to "RUNNING" mode
4. The program starts automatically

**Screen simulation during firing:**
```
┌────────────────────────────────┐
│ Phase 2                        │
│ 250C/h->570C, 15m              │
│────────────────────────────────│
│ Temp Read            365C      │
│ Temp Target          420C      │
│ Heat Power            78%      │
│ Phase                 45%      │
└────────────────────────────────┘
```

### Hot Start
If you start a program while the kiln is already hot, the system automatically detects the appropriate phase and resumes from the current temperature.

## 🔥 During Firing (Running Mode)

### Display
- **Current phase**: Clearly displayed (example: "Phase 2: 250°C/h→570C, 15m")
- **Current vs target temperature**: Displayed in real-time
- **Heating status**: "Temp Read" and "Temp Target" shown
- **Power**: "Heat Power" with percentage (0-100%)

### Emergency Stop
**Press the push button** at any time to immediately stop the program and cut the heating.

## 🔬 Optional Features (Advanced)

CalciferAi offers two optional features that can be enabled according to your needs:

### 📊 Temperature Graph (ENABLE_GRAPH)

**Description**: Displays a real-time graph during firing showing:
- The expected temperature curve (programmed profile)
- The actual measured temperature curve
- Allows you to visualize if the kiln is following the program correctly

**Access**: During firing, click the encoder to switch between the main screen and the graph.

**Graph screen simulation:**
```
┌────────────────────────────────┐
│P:12.5  ┌──────────────────┐    │
│I:8.3   │         ╱────────│1100C│
│        │       ╱          │    │
│        │     ╱            │    │
│        │   ╱··            │    │
│        │ ╱·               │    │
│        └──────────────────┘    │
│                       4h30      │
└────────────────────────────────┘
```
*Solid line = target temperature*  
*Dots = measured temperature*

**Memory usage**: ~800 bytes of RAM

### 📡 Serial Logging (ENABLE_LOGGING)

**Description**: Sends firing data via USB serial port to a computer:
- Current and target temperature in real-time
- PID values (Proportional, Integral)
- Heating power (%)
- Temperature error
- One data line every 5 seconds

**Usage**: Connect the Arduino to a computer, open the serial monitor (9600 baud) to view and record data.

**Install dependencies** (once only):
```bash
cd /path/to/CalciferAi/Logger
pip3 install -r requirements_logger.txt
```

**Launch the Logger with graph**:
```bash
cd /path/to/CalciferAi
python3 Logger/arduino_logger.py
```

**Serial output example:**
```
=== LUCIA START ===
PID: Kp=2.50 Ki=0.03
Time(ms), Temp(C), Target(C), P, I, Power(%), Error(C)
---
>>> PROGRAM STARTED <<<
Initial temperature: 25.5C
Detected phase: 1
---
5000, 28.3, 30.5, 5.5, 0.2, 15, 2.2
10000, 33.1, 35.2, 5.2, 0.8, 20, 2.1
15000, 38.7, 40.8, 5.2, 1.5, 28, 2.1
...
```

**Memory usage**: ~250 bytes of RAM

### ⚠️ Important Limitation

**You CANNOT enable both at the same time!**

**Reason**: The Arduino Uno only has **2048 bytes of RAM** in total. Both features together would consume ~1050 bytes, leaving too little memory for normal system operation and causing crashes or unpredictable behavior.

### 🔧 How to Enable These Features

**File to modify**: `lucia/definitions.h`

**Lines 21-23**:
```cpp
// ===== OPTIONAL FEATURES =====
// Uncomment to enable (see ACTIVATION_FONCTIONNALITES.md for details)
#define ENABLE_LOGGING  // Serial Logging (~250 bytes) - Monitoring/Debug
//#define ENABLE_GRAPH    // Temperature graph (~800 bytes) - Visualization
```

**To enable LOGGING** (default configuration):
```cpp
#define ENABLE_LOGGING  // ← Active line (without //)
//#define ENABLE_GRAPH    // ← Disabled line (with //)
```

**To enable GRAPH**:
```cpp
//#define ENABLE_LOGGING  // ← Disabled line (add //)
#define ENABLE_GRAPH    // ← Active line (remove //)
```



**⚠️ Important**: After modification, you must **recompile and upload** the program to the Arduino.

### 💡 Which Mode to Choose?

- **LOGGING**: To analyze and record firings, create curves on computer, debug
- **GRAPH**: To visually monitor the firing directly on the OLED screen, without computer
- **None**: To save RAM if you encounter stability issues

## 🔧 Advanced Settings (Settings Menu)

### Accessing the Settings menu
1. In Stop mode, select the "Settings" icon in the top right
2. Click the encoder to enter the settings

**Settings screen simulation:**
```
┌────────────────────────────────┐
│ SETTINGS             v01.0     │
│                                │
│ Heat Cycle             1000ms  │
│ Kp                      2.5    │
│ Ki                     0.030   │
│ Max delta                10C   │
│ Max Temp               1200C   │  ← Safety
│ Exit                    <--    │
└────────────────────────────────┘
```
*Note: The selected item is surrounded by a frame. Use the encoder to navigate between parameters.*

### Available parameters
- **Heat Cycle**: PWM cycle duration (100 to 10000 ms) - *Advanced*
- **Kp**: PID proportional gain (0.0 to 10.0) - *Advanced*
- **Ki**: PID integral gain (0.0 to 1.0) - *Advanced*
- **Max delta**: End-of-phase tolerance (1 to 50°C) - *Recommended: 10°C*
- **Max Temp**: Maximum kiln temperature (500 to 1500°C) - *🛡️ SAFETY*
- **Exit**: Exit the Settings menu

⚠️ **Important notes**:
- Only modify the PID parameters (Kp, Ki) if you understand how they work. Default values are optimized.

### 🛡️ Max Temp Protection (IMPORTANT)

**Max Temp** is a crucial safety parameter that limits the maximum programmable temperature:

- **Default value**: 1200°C (standard ceramic kiln)
- **Adjustment range**: 500°C to 1500°C
- **Modification**: In 10°C steps

**Protection role**:
1. Prevents programming temperatures higher than the kiln's capabilities
2. Protects against manipulation errors (e.g., 1800°C instead of 180°C)
3. Allows adapting the system to different kiln types
4. Phase 1, 2 and 3 temperatures are automatically limited to this value

**Setting examples**:
- Raku kiln: 1000-1100°C
- Stoneware/porcelain kiln: 1200-1300°C
- High temperature kiln: 1400-1500°C

⚠️ **Before modifying**: Check the maximum temperature supported by your kiln in its technical documentation!

## ⚠️ Error Messages

### "Temp fail 2min" / "Heat stopped"
**Cause**: The temperature sensor has not been working correctly for more than 2 minutes.

**Error screen simulation:**
```
┌────────────────────────────────┐
│ ERROR!                         │
│                                │
│ Temp fail 2min                 │
│ Heat stopped                   │
│ Check sensor                   │
│                                │
└────────────────────────────────┘
```

**Action**:
1. Check thermocouple connections
2. Check that the thermocouple is not damaged
3. Press the button to reset

### "MAX31856 Error!" / "Check wiring"
**Cause**: The temperature reading module was not detected at startup.

**Error screen simulation:**
```
┌────────────────────────────────┐
│                                │
│ MAX31856 Error!                │
│                                │
│ Check wiring                   │
│ Press to retry                 │
│                                │
└────────────────────────────────┘
```

**Action**:
1. Check all MAX31856 module connections
2. Restart the system
3. Press the button to attempt reconnection

## 🛡️ Safety Instructions

### ⚠️ IMPORTANT
1. **NEVER leave a heating kiln unattended**
2. The relay automatically cuts off in Stop mode
3. If temperature error > 2 minutes, heating stops automatically
4. The stop button works at any time (immediate stop)

### Recommendations
- Always test your program empty before a real firing
- Note your successful firing programs to reuse them
- Monitor the first minutes after starting to verify proper operation
- Do not open the kiln during firing (except emergency)

## 📊 Typical Program Example

### Raku Firing (Example)
- **Phase 1**: 100°C/h → 150°C, hold 60 min (drying)
- **Phase 2**: 150°C/h → 600°C, hold 10 min (preheating)
- **Phase 3**: 600°C/h → 980°C, hold 10 min (firing)
- **Cooling**: 150°C/h → 500°C (controlled cooling)

### Stoneware Firing (Example)
- **Phase 1**: 50°C/h → 100°C, hold 5 min
- **Phase 2**: 250°C/h → 570°C, hold 15 min
- **Phase 3**: 200°C/h → 1100°C, hold 20 min
- **Cooling**: 150°C/h → 200°C

## 💾 Parameter Saving

Your program parameters are **automatically saved** with each modification. They are preserved even after a power outage.

## 📞 Troubleshooting

1. **The kiln doesn't heat**: Check that the program is started (push button)
2. **Temperature doesn't rise**: Check the SSR relay and kiln wiring
3. **Screen is black**: Check power supply and OLED screen connections
4. **Abnormal temperature**: Check thermocouple (Type S required)

---

**Version**: 1.0  
**System**: CalciferAi - Intelligent ceramic kiln controller

