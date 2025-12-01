# Step-by-Step Build Guide: Ceramic Kiln Controller

This guide will walk you through building an Arduino-based temperature controller for your ceramic kiln. Even if you're not a coder, you can follow these steps to complete the project.

## ⚠️ IMPORTANT SAFETY WARNINGS

**Before you begin:**
- This project involves HIGH VOLTAGE and HIGH TEMPERATURES
- Always disconnect power before making wiring changes
- The solid-state relay will control your kiln's heating elements
- Ensure all connections are secure before powering on
- Test the system thoroughly before leaving it unattended
- Consider having an electrician verify high-voltage connections

---

## Phase 1: Gather Your Components

### Required Components

Purchase these items (you can find them on Amazon, Adafruit, SparkFun, or local electronics stores):

1. **Arduino Uno** (or compatible board)
2. **SH1106 OLED Display** (128x64 pixels, I2C interface)
3. **Adafruit MAX31856 Thermocouple Amplifier** (specifically for Type S thermocouples)
4. **Type S Thermocouple Probe** (rated for high temperatures - up to 1600°C)
5. **Rotary Encoder with Push Button** (KY-040 style or similar)
6. **Push Button** (momentary switch)
7. **LED** (any color, for status indication)
8. **Solid-State Relay (SSR)** - **IMPORTANT:** Must be rated for your kiln's voltage and current
   - For 240V kilns: Use a 240V SSR (typically 25-40A)
   - Check your kiln's specifications for exact requirements
9. **Breadboard** (half-size or full-size)
10. **Jumper Wires** (male-to-male, various lengths)
11. **USB Cable** (for Arduino Uno - Type B)
12. **Resistors** (optional, for LED if needed - usually 220Ω)

### Tools Needed

- Computer with USB port
- Wire strippers
- Small screwdriver set
- Multimeter (optional but recommended)
- Breadboard-friendly wire cutters

---

## Phase 2: Set Up Your Computer

### Step 2.1: Install Arduino IDE

1. Go to [arduino.cc/en/software](https://www.arduino.cc/en/software)
2. Download Arduino IDE (choose the version for your operating system)
3. Install the software following the on-screen instructions
4. Open Arduino IDE

### Step 2.2: Install Required Libraries

You'll need to install three libraries:

**Library 1: U8g2lib (for the OLED screen)**
1. In Arduino IDE, go to **Sketch → Include Library → Manage Libraries**
2. Search for "U8g2"
3. Find "U8g2" by olikraus
4. Click **Install**

**Library 2: Adafruit MAX31856**
1. Still in Library Manager, search for "MAX31856"
2. Find "Adafruit MAX31856" by Adafruit
3. Click **Install**
4. It may ask to install dependencies - click **Install All**

**Library 3: Encoder**
1. Search for "Encoder"
2. Find "Encoder" by Paul Stoffregen
3. Click **Install**

### Step 2.3: Connect Your Arduino

1. Connect your Arduino Uno to your computer using the USB cable
2. In Arduino IDE, go to **Tools → Board → Arduino Uno**
3. Go to **Tools → Port** and select the port that appears (it will show something like "COM3" on Windows or "/dev/tty.usbmodem..." on Mac)

---

## Phase 3: Build the Circuit (Wiring)

**⚠️ Work on a clean, well-lit surface. Double-check each connection before moving to the next.**

### Step 3.1: Set Up the Breadboard

1. Place your breadboard on your work surface
2. Connect Arduino 5V pin to the breadboard's positive rail (red line)
3. Connect Arduino GND pin to the breadboard's negative rail (blue/black line)

### Step 3.2: Wire the OLED Display (SH1106)

Connect the 4 wires from your OLED display:
- **VCC** → Breadboard 5V rail (red)
- **GND** → Breadboard GND rail (black)
- **SCL** → Arduino pin **A5**
- **SDA** → Arduino pin **A4**

### Step 3.3: Wire the MAX31856 Thermocouple Amplifier

This has 8 connections:
- **VIN** → Breadboard 5V rail
- **GND** → Breadboard GND rail
- **SCK** → Arduino pin **D13**
- **SDO** → Arduino pin **D12**
- **SDI** → Arduino pin **D11**
- **CS** → Arduino pin **D10**
- **DRDY** → Arduino pin **D9**
- **FLT** → Arduino pin **D8**

### Step 3.4: Connect the Thermocouple Probe

- **Positive wire (+)** → MAX31856 board **T+** terminal
- **Negative wire (-)** → MAX31856 board **T-** terminal

**Note:** Type S thermocouples have specific wire colors - usually red for positive. Check your probe's documentation.

### Step 3.5: Wire the Rotary Encoder

- **GND** → Breadboard GND rail
- **VCC** → Breadboard 5V rail
- **CLK (or A)** → Arduino pin **D2**
- **DT (or B)** → Arduino pin **D3**
- **SW (button)** → Arduino pin **D4**

### Step 3.6: Wire the Push Button

- One pin → Arduino pin **D5**
- Other pin → Breadboard GND rail

### Step 3.7: Wire the Control LED

- **Anode (+)** → Arduino pin **A1**
- **Cathode (-)** → Breadboard GND rail

**Note:** If your LED doesn't have a built-in resistor, add a 220Ω resistor between the anode and pin A1.

### Step 3.8: Wire the Solid-State Relay (SSR)

**⚠️ CRITICAL:** The SSR controls high voltage. Be very careful here.

- **Control + (positive terminal)** → Arduino pin **D6**
- **Control - (negative terminal)** → Breadboard GND rail

**DO NOT connect the high-voltage side yet!** We'll test the low-voltage control first.

---

## Phase 4: Get the Code

### Option A: If Code is Already Created

1. If you have the code files (`.ino` files), open the main file (usually `lucia.ino`) in Arduino IDE
2. Make sure all related files are in the same folder

### Option B: If You Need to Generate Code

1. Use the specification document (`Prompt_CalciferAi_EN.md`) with an AI coding assistant
2. Request: "Generate Arduino code based on this specification"
3. Save all code files in a folder named `lucia` (this is important - Arduino requires the folder name to match the main file)

### Step 4.1: Verify the Code

1. In Arduino IDE, click the **Verify** button (checkmark icon)
2. Wait for compilation
3. If there are errors, fix them (common issues: missing libraries, typos)
4. You should see "Done compiling" when successful

---

## Phase 5: Upload and Test (Low Voltage)

### Step 5.1: Upload the Code

1. Make sure your Arduino is connected via USB
2. Click the **Upload** button (arrow icon) in Arduino IDE
3. Wait for "Done uploading" message
4. The Arduino will restart automatically

### Step 5.2: Initial Testing (Without Kiln Connected)

1. The OLED screen should turn on and display information
2. Try rotating the encoder - you should see values change on screen
3. Press the encoder button - it should allow editing values
4. Press the push button - it should toggle between prog_OFF and prog_ON states
5. The LED should light up when the relay would be active

### Step 5.3: Test Temperature Reading

1. The thermocouple should read room temperature
2. Try gently warming the thermocouple tip (carefully with your hand)
3. Watch the temperature reading on the screen change

**If something doesn't work:**
- Check all wire connections
- Verify libraries are installed
- Check the Serial Monitor (Tools → Serial Monitor) for error messages
- Review the wiring diagram in the specification document

---

## Phase 6: Connect to Your Kiln (High Voltage)

**⚠️ EXTREME CAUTION: This step involves high voltage. If you're not comfortable, hire an electrician.**

### Step 6.1: Understand Your Kiln's Wiring

1. **Turn off power to your kiln** at the circuit breaker
2. Locate where your kiln's heating elements connect to power
3. Identify the "hot" wire (usually black or red) that goes to the heating elements
4. Your SSR will interrupt this wire

### Step 6.2: Install the SSR in the Kiln Circuit

**This is a simplified explanation - actual installation may vary:**

1. The SSR has two sides:
   - **Low-voltage side:** Already connected to Arduino (D6 and GND)
   - **High-voltage side:** Will control your kiln

2. For the high-voltage side:
   - One terminal connects to the incoming "hot" wire
   - Other terminal connects to the heating element wire
   - The neutral wire bypasses the SSR (stays connected directly)

3. **Important:** 
   - Use proper wire gauges for your kiln's current
   - Ensure all connections are secure
   - Use wire nuts or proper terminal blocks
   - Follow local electrical codes

### Step 6.3: Mount the Thermocouple

1. Install the thermocouple probe inside your kiln
2. Position it where you want to measure temperature (usually near the center)
3. Route the wires carefully to avoid damage
4. Ensure the probe can handle your kiln's maximum temperature

### Step 6.4: Final Safety Check

Before powering on:
- [ ] All low-voltage connections are secure
- [ ] All high-voltage connections are secure and properly insulated
- [ ] Thermocouple is properly installed
- [ ] Arduino code is uploaded and tested
- [ ] You understand how to stop the system (push button or circuit breaker)
- [ ] Someone else knows where the circuit breaker is

---

## Phase 7: First Real Test

### Step 7.1: Power On Sequence

1. Turn on power to your kiln at the circuit breaker
2. The Arduino should already be powered via USB
3. The OLED screen should show the prog_OFF state

### Step 7.2: Configure Your Firing Program

1. Use the rotary encoder to navigate through settings
2. Set your desired temperatures and heating rates for each phase
3. Click the encoder button to edit values
4. Rotate to change values
5. Click again to save

### Step 7.3: Start a Test Firing

1. Start with a low-temperature test (e.g., 100°C)
2. Press the push button to start the program (prog_ON)
3. Watch the temperature rise
4. Monitor the display for any issues
5. The LED should indicate when heating is active

### Step 7.4: Monitor Closely

**For the first few firings:**
- Stay nearby and monitor constantly
- Watch for any unusual behavior
- Check that temperatures match expectations
- Verify the cooling phase works correctly

---

## Phase 8: Troubleshooting Common Issues

### Problem: OLED Screen Doesn't Turn On

**Solutions:**
- Check VCC and GND connections
- Verify SCL and SDA are on A5 and A4
- Try a different I2C address (some screens need code modification)
- Check if screen needs external power

### Problem: Temperature Reading is Wrong

**Solutions:**
- Verify thermocouple wires are connected correctly (T+ and T-)
- Check that thermocouple type is set to Type S in code
- Ensure thermocouple probe is not damaged
- Check MAX31856 connections

### Problem: Encoder Doesn't Work

**Solutions:**
- Verify D2, D3, D4 connections
- Check that Encoder library is installed
- Try rotating slower - some encoders are sensitive
- Test encoder with a simple sketch first

### Problem: Relay Doesn't Turn On

**Solutions:**
- Check D6 connection
- Verify SSR is getting power
- Test SSR with a simple on/off sketch
- Check SSR rating matches your needs
- Ensure high-voltage side is wired correctly

### Problem: Program Crashes or Freezes

**Solutions:**
- Check Serial Monitor for error messages
- Verify all libraries are up to date
- Ensure code doesn't use `delay()` function
- Check that all variables are initialized

---

## Phase 9: Enclosure and Final Assembly

### Step 9.1: Choose an Enclosure

1. Find a suitable electrical box or project enclosure
2. Ensure it's large enough for Arduino, breadboard, and SSR
3. Consider ventilation (electronics generate some heat)
4. Make sure it's rated for your environment

### Step 9.2: Mount Components

1. Secure Arduino inside the enclosure
2. Mount the OLED screen where it's visible
3. Mount the encoder and push button on the front panel
4. Secure the SSR (it may need a heat sink)
5. Route all wires neatly

### Step 9.3: Safety Features

1. Add a physical emergency stop button (optional but recommended)
2. Label all controls clearly
3. Add warning labels about high voltage
4. Ensure proper grounding

---

## Phase 10: Documentation and Maintenance

### Step 10.1: Document Your Setup

1. Take photos of your wiring
2. Write down your typical firing programs
3. Note any customizations you made
4. Keep the specification document handy

### Step 10.2: Regular Maintenance

1. Periodically check all connections
2. Clean the thermocouple probe
3. Verify temperature accuracy
4. Update code if libraries are updated
5. Test emergency stop functionality regularly

---

## Getting Help

If you get stuck:

1. **Check the specification document** (`Prompt_CalciferAi_EN.md`) for technical details
2. **Arduino Forums** - [forum.arduino.cc](https://forum.arduino.cc)
3. **Reddit** - r/arduino, r/electronics
4. **Local Maker Spaces** - Often have people who can help
5. **YouTube Tutorials** - Search for "Arduino kiln controller" or specific component tutorials

---

## Estimated Timeline

- **Phase 1 (Gathering components):** 1-2 weeks (ordering and shipping)
- **Phase 2 (Computer setup):** 1-2 hours
- **Phase 3 (Wiring):** 2-4 hours (take your time!)
- **Phase 4 (Getting code):** Depends on if code exists or needs generation
- **Phase 5 (Testing):** 1-2 hours
- **Phase 6 (Kiln connection):** 2-4 hours (or hire electrician)
- **Phase 7 (First test):** 1-2 hours
- **Phase 8 (Troubleshooting):** Variable
- **Phase 9 (Enclosure):** 2-4 hours
- **Phase 10 (Documentation):** 1 hour

**Total:** Plan for 1-2 weeks from start to first successful firing

---

## Cost Estimate

- Arduino Uno: $20-30
- OLED Display: $5-10
- MAX31856 Board: $15-25
- Type S Thermocouple: $20-50
- Rotary Encoder: $2-5
- Push Button: $1-2
- LED: $1
- Solid-State Relay: $15-40 (depends on rating)
- Breadboard and wires: $5-10
- Enclosure: $10-30

**Total:** Approximately $100-200 USD (prices vary by location and source)

---

## Final Notes

- **Take your time** - Rushing leads to mistakes
- **Double-check everything** - Especially high-voltage connections
- **Test incrementally** - Don't connect everything at once
- **Ask for help** - There's no shame in getting assistance
- **Safety first** - If something doesn't feel right, stop and reassess

Good luck with your project! 🎨🔥


