# CalciferAi - Ceramic Kiln Controller

An Arduino-based intelligent temperature controller for ceramic kilns with 3-phase firing programs and active cooling control.

## 📁 Project Files

### Documentation Files
- **`Prompt_CalciferAi_EN.md`** - Complete specification with all requirements (UPDATED with your answers)
- **`BUILD_GUIDE.md`** - Step-by-step hardware build guide for non-coders
- **`CODE_GENERATION_GUIDE.md`** - Guide for generating code with AI assistance (includes your answered questions)
- **`READY_TO_USE_PROMPT.txt`** - Ready-to-paste prompt for AI code generation ⭐ **USE THIS**
- **`README.md`** - This file

## 🚀 Quick Start - Generate Your Code

### Option 1: Copy & Paste (Recommended)

1. Open `READY_TO_USE_PROMPT.txt`
2. Copy **EVERYTHING** from that file
3. Go to [ChatGPT](https://chat.openai.com) or [Claude](https://claude.ai)
4. Paste the entire prompt
5. Wait for code generation
6. Download all generated files to a folder named `lucia`

### Option 2: Use the Prompt File Directly

1. Copy the content from `Prompt_CalciferAi_EN.md`
2. Use the templates in `CODE_GENERATION_GUIDE.md`
3. Combine them and submit to your AI assistant

## 📋 What's Included in the Specification

### Hardware Components
- Arduino Uno
- SH1106 OLED Display (128x64, I2C)
- MAX31856 Thermocouple Amplifier
- Type S Thermocouple (high-temp)
- Rotary Encoder (clickable)
- Push Button
- LED indicator
- Solid State Relay (SSR)

### Features
✅ 3-phase programmable heating with custom rates  
✅ Active cooling control with rate management  
✅ PI temperature control (Kp=2.0, Ki=0.5)  
✅ EEPROM settings persistence  
✅ Real-time temperature graph display  
✅ Error handling with 2-minute grace period  
✅ Non-blocking code using millis()  
✅ Phase elapsed time and total time remaining  
✅ Color-coded status display  

### Control Logic
- **prog_OFF:** Settings screen with encoder navigation
- **prog_ON:** Active firing with automatic phase transitions
- **Graph View:** Temperature profile visualization

## 🔧 Build Instructions

Follow the **`BUILD_GUIDE.md`** for complete hardware assembly instructions including:
- Component shopping list with prices
- Step-by-step wiring diagrams
- Arduino IDE setup
- Library installation
- Testing procedures
- Safety warnings

**Estimated Cost:** $100-200 USD  
**Build Time:** 1-2 weeks (including parts delivery)

## 💻 After Code Generation

1. **Save Files:**
   - Create a folder named `lucia`
   - Save all generated files in this folder
   - The main file MUST be named `lucia.ino`

2. **Install Libraries:**
   - Open Arduino IDE
   - Install: U8g2lib, Adafruit_MAX31856, Encoder

3. **Upload to Arduino:**
   - Connect Arduino Uno via USB
   - Select correct board and port
   - Click Upload

4. **Test:**
   - Follow testing procedures in BUILD_GUIDE.md
   - Test display, encoder, temperature reading
   - Test WITHOUT kiln connected first!

## ⚙️ Your Custom Settings

Your specification includes these customized behaviors:

- **PID Parameters:** Kp=2.0, Ki=0.5, Kd=0
- **Navigation:** Sequential through all 11 parameters
- **Selection:** Inverted colors
- **Editing:** Thin outline
- **Display:** 500ms refresh, all info on one page
- **Colors:** White/blue/red on black background
- **Error Handling:** 2-minute grace period before shutdown
- **Storage:** Auto-save to EEPROM on edit exit
- **Graph Access:** Long press encoder button (>1s)
- **Phase Target:** Within 5°C above setpoint

## 📊 Firing Program Structure

```
Phase 0: prog_OFF (settings screen)
   ↓ [Push Button]
Phase 1: Heat to Step1Temp at Step1Speed, hold for Step1Wait
   ↓ [Auto transition]
Phase 2: Heat to Step2Temp at Step2Speed, hold for Step2Wait
   ↓ [Auto transition]
Phase 3: Heat to Step3Temp at Step3Speed, hold for Step3Stage
   ↓ [Auto transition]
Phase 4: Cool to Step4Target at Step4Speed (active cooling)
   ↓ [Auto return]
Phase 0: prog_OFF (complete)
```

## 🛡️ Safety Features

- Emergency stop via push button (any time)
- Automatic relay shutoff on error
- Temperature reading failure detection
- 2-minute grace period for sensor glitches
- Relay forced OFF in prog_OFF state
- Maximum 10% power change per cycle

## 📝 Default Firing Program

| Phase | Temperature | Rate | Duration |
|-------|-------------|------|----------|
| 1 | 100°C | 50°C/h | 5 min |
| 2 | 570°C | 250°C/h | 15 min |
| 3 | 1100°C | 200°C/h | 20 min |
| 4 (Cool) | 200°C | 150°C/h | - |

## 🎨 Display Layout

### prog_OFF Screen
```
Phase 1: 100°C, 50°C/h, 5min
Phase 2: 570°C, 250°C/h, 15min
Phase 3: 1100°C, 200°C/h, 20min
Cool: 150°C/h → 200°C
Temp: 25°C
OFF, 0%
```

### prog_ON Screen
```
Phase 1: 100°C >150°C, 60min [dimmed]
Phase 2: 150°C >600°C, 10min [dimmed]
Phase 3: 600°C >980°C, 10min [bright]
Cool: 150°C/h <500°C [dimmed]
HEATING: 155°C → 167°C [green]
ON, 60% [red]
```

## 🔌 Pin Assignments

| Component | Pin | Function |
|-----------|-----|----------|
| OLED SDA | A4 | I2C Data |
| OLED SCL | A5 | I2C Clock |
| MAX31856 CS | D10 | SPI Chip Select |
| MAX31856 SDI | D11 | SPI MOSI |
| MAX31856 SDO | D12 | SPI MISO |
| MAX31856 SCK | D13 | SPI Clock |
| MAX31856 DRDY | D9 | Data Ready |
| MAX31856 FLT | D8 | Fault Signal |
| Encoder CLK | D2 | Interrupt |
| Encoder DT | D3 | Direction |
| Encoder SW | D4 | Click Button |
| Push Button | D5 | Start/Stop |
| Relay | D6 | PWM Control |
| LED | A1 | Status Indicator |

## 📚 Resources

- [Arduino Official Site](https://www.arduino.cc/)
- [U8g2 Library Documentation](https://github.com/olikraus/u8g2)
- [Adafruit MAX31856 Guide](https://learn.adafruit.com/adafruit-max31856-thermocouple-amplifier)
- [Encoder Library](https://www.pjrc.com/teensy/td_libs_Encoder.html)

## ⚠️ Important Notes

1. **High Voltage:** Kiln connection involves dangerous voltages - hire an electrician if unsure
2. **Testing:** Always test thoroughly before leaving unattended
3. **Code Size:** Target is <31,232 bytes for Arduino Uno
4. **Non-blocking:** All code uses millis() - no delay() functions
5. **EEPROM Life:** Settings only save on edit exit to preserve EEPROM

## 🤝 Getting Help

If you encounter issues:

1. **Compilation Errors:** Ask AI to fix specific error messages
2. **Hardware Issues:** Check BUILD_GUIDE.md troubleshooting section
3. **Behavior Problems:** Describe what happens vs. what should happen
4. **Arduino Forums:** [forum.arduino.cc](https://forum.arduino.cc)
5. **Reddit:** r/arduino, r/electronics

## 📄 License

This project specification is for personal use. Generated code should include appropriate licensing based on library requirements.

---

**Ready to generate your code?** Open `READY_TO_USE_PROMPT.txt` and follow the Quick Start instructions above! 🚀
