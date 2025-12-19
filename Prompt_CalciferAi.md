# ARDUINO AI Prompt for Ceramic Kiln

## Project Description

I want you to generate code to operate the thermostat of an electric kiln for firing pottery and ceramics. Start by analyzing the prompt data, use all the information, then, as an Arduino expert, create optimized code. Deliver an archive with all files.

The kiln program will follow these steps:

- Phase 0, called `prog_OFF` which is a settings screen.
- Phase 1, to reach the first plateau temperature, then wait for the plateau duration
- Phase 2, to reach the second plateau temperature, then wait for the plateau duration
- Phase 3, to reach the third plateau temperature, then wait for the plateau duration
- Phase 4 `cooldown`, to cool down to the target temperature named `cool_state`, then the program returns to prog_OFF state

The transition between phases occurs as soon as the conditions are met. Rotating the encoder allows selecting values during the prog_OFF state. A click allows editing them, then rotation changes the parameter. A new click exits the editing function. The push button toggles between prog_OFF and prog_ON states.

## Thermostat Management

The thermostat must compensate for the kiln's inertia. It heats with a proportional and integral function. These values control a heating proportionality function `PowerHold`, a software PWM with a period `CycleLength` defaulting to one second with a duty cycle controlled from 0 to 100% over a duration `Pcycle`.

The components used are:

- An Arduino Uno
- A **SH1106 OLED screen** 128x64 pixels with 4 wires in I2C
- An **Adafruit Universal Thermocouple Amplifier MAX31856**
- A Type S temperature probe
- A clickable encoder
- A push button, to start or stop the program
- A control LED that indicates if the kiln's heating element is powered
- A solid-state relay that controls the kiln's heating element

## The Two States of the SH1106 Screen

### State `prog_OFF` when the kiln is not executing a program

The information visible on the screen are:

- First plateau temperature in °C, heating rate in °C/h and plateau duration in minutes
- Second plateau temperature, heating rate and plateau duration in minutes
- Third plateau temperature, heating rate and plateau duration in minutes
- Cooling rate, cooling target temperature
- Current temperature
- The prog_OFF state
- The relay control state ON/OFF as well as its heating rate `Pcycle` (when stopped the relay must be OFF and its rate at 0%)

**The information selectable and editable with the clickable encoder are:**

- First plateau temperature in °C, heating rate in °C/h and plateau duration in minutes
- Second plateau temperature, heating rate and plateau duration in minutes
- Third plateau temperature, heating rate and plateau duration in minutes
- Cooling rate, cooling target temperature

### State `prog_ON` when the program is running

The information visible on the screen are:

- Setpoint temperature
- First plateau temperature in °C, heating rate in °C/h and plateau duration in minutes
- Second plateau temperature, heating rate and plateau duration in minutes
- Third plateau temperature, heating rate and plateau duration in minutes
- Cooling rate, cooling target temperature
- The prog_ON state
- The relay control state ON/OFF as well as its heating rate `Pcycle`

**The information selectable and editable with the clickable encoder are:**

- none

### Example of prog_ON screen, in phase 3:

Phase 3 (current phase displayed)  
250C/h->980C, 10m  
────────────────────────────  
Temp Read            155C  
Temp Target          167C  
Heat Power            60%  
Phase                 45%

## File Organization

I want to isolate the main functions in different files:

- `lucia.ino` for initialization
- `display` for screen controls
- `Temperature` for temperature acquisition, setpoint temperature and the software PWM function that controls the kiln with a duty cycle.

## Complete Wiring for Ceramic Kiln Thermostat

### Electronic Notes

#### Component List

- Arduino Uno
- SH1106 OLED Screen 128x64 pixels (I2C, 4 wires)
- Adafruit Universal Thermocouple Amplifier MAX31856
- Type S temperature probe
- Clickable rotary encoder (5 wires)
- A push button
- Control LED
- Solid-state relay (2-wire input)
- Small breadboard for power distribution

#### Detailed Connections

**1. SH1106 OLED Screen (I2C)**

- VCC → Breadboard 5V rail
- GND → Breadboard GND rail
- SCL → Arduino A5 (I2C clock line)
- SDA → Arduino A4 (I2C data line)

**2. MAX31856 Thermocouple Amplifier**

- VIN → Breadboard 5V rail (power supply)
- GND → Breadboard GND rail (ground)
- SCK → Arduino D13 (Serial Clock for SPI)
- SDO → Arduino D12 (MISO - Master In Slave Out)
- SDI → Arduino D11 (MOSI - Master Out Slave In)
- CS → Arduino D10 (Chip Select for SPI peripheral selection)
- DRDY → Arduino D9 (Data Ready - interrupt signal when data is ready)
- FLT → Arduino D8 (Fault - interrupt signal in case of error)

**3. Type S Thermocouple**

- Positive wire (+) → MAX31856 T+ terminal
- Negative wire (-) → MAX31856 T- terminal

**4. Clickable Rotary Encoder**

- GND → Breadboard GND rail (ground)
- VCC → Breadboard 5V rail (power supply)
- CLK (or A) → Arduino D2 (first encoder signal, used for interrupts)
- DT (or B) → Arduino D3 (second encoder signal, used to determine direction)
- SW (button) → Arduino D4 (encoder integrated button, used as validation)

**5. Push Button**

- one pin → Arduino D5, the other → GND rail (with internal pull-up resistor)

**6. Control LED**

- Anode (+) → Arduino A1 (uses an analog pin with internal pull-up resistor)
- Cathode (-) → Breadboard GND rail (ground)
- Note: Enable the internal pull-up resistor in software, then disable it to turn on the LED

**7. Solid-State Relay (kiln heating element control)**

- Positive terminal → Arduino D6 (control signal)
- Negative terminal → Breadboard GND rail (ground)
- Model used: Fotek SSR-25DA (3-32V DC input, ~10-16 mA at 5V)

**7.1 Protection Circuit with NPN Transistor (Recommended)**

To protect Pin 6 from overcurrent (especially with counterfeit SSRs), use an NPN transistor:

```
+5V Arduino
     │
    [R2 100Ω]  ← Protection against short-circuit
     │
     ▼
SSR (+) input
     │
SSR (-) input
     │
     ▼
Collector (C)
     │
     ├──────────────[R1 1kΩ]──────── Pin 6 Arduino
     │
   Base (B)  ← NPN Transistor (2N2222A or BC547)
     │
Emitter (E)
     │
     ▼
   GND Arduino
```

**Recommended transistors:**
| Reference | Ic max | Gain (hFE) | Notes |
|-----------|--------|------------|-------|
| 2N2222A | 800 mA | 100-300 | Very common |
| BC547 | 100 mA | 110-800 | Very common |
| BC337 | 800 mA | 100-630 | Common |

**Pinout warning:** 2N2222A (E-B-C) and BC547 (C-B-E) have reversed pinouts! Check datasheet.

**Components:**
- R1 = 1kΩ: Limits base current (~4 mA on Pin 6)
- R2 = 100Ω: Limits current to 50 mA max if transistor fails (protects Arduino)

**Breadboard Power Supply**

- Arduino 5V → Breadboard positive rail (5V power supply for all components)
- Arduino GND → Breadboard negative rail (common ground for all components)

## Programming Notes

- Pins D2 and D3 are used for the encoder because they support external interrupts
- The MAX31856 uses the standard SPI interface (pins D11-D13) with D10 as selector
- The OLED screen uses the standard I2C bus (A4 and A5)
- The solid-state relay is controlled by PWM if necessary for proportional heat control
- The LED serves as a visual indicator of the heating state (lit when the kiln is heating)

**Use the libraries:**

- U8g2lib (for the SH1106 OLED screen)
- Adafruit_MAX31856 (for the thermocouple module)
- Encoder (for the rotary encoder)

## Settings Screen

The Phase 0 (prog_OFF) screen includes a settings icon "S" in the top right corner (replacing the power percentage display). This icon is clickable and leads to a Settings screen.

### Settings Screen Contents

The Settings screen displays a list of modifiable parameters using the same typography, size, and visual effects as Phase 0:

1. **Heat Cycle** (named "Heat Cycle", corresponds to `Pcycle`) - PWM cycle duration in milliseconds
2. **Kp** - Proportional gain of the PID controller (displayed on its own line)
3. **Ki** - Integral gain of the PID controller (displayed on its own line)
4. **Max delta** - Maximum temperature tolerance to consider a plateau reached (1 to 50°C, default: 10°C)
5. **Max Temp** - Maximum kiln temperature (500 to 1500°C, default: 1200°C) - Safety protection
6. **Exit** - Button to exit settings and return to Phase 0

**Notes:** 
- Kd (derivative gain) has been removed as it's not suitable for ceramic kiln control due to high thermal inertia.
- **Max Temp** is a safety feature that prevents programming temperatures higher than the kiln's capability. All phase temperatures (Step1Temp, Step2Temp, Step3Temp) are automatically limited to this maximum value.

### Settings Navigation and Editing

- Navigation uses the same encoder mechanism as Phase 0
- Click encoder button to enter/exit edit mode for selected parameter
- Rotate encoder to modify values while in edit mode
- Click on "Exit" to return to Phase 0
- Settings are saved to EEPROM when exiting edit mode
- All settings take effect immediately when modified

## Variables List

Here is the list of variables to use, with their role:

| Variable | Function | Default Value |
|----------|----------|---------------|
| **Pcycle** | Total duration of a PWM cycle in milliseconds | 1000 |
| **PowerHold** | Percentage of time the system must be on (0 to 100) | |
| **PowerON** | Current relay state (1 = on, 0 = off) | 0 |
| **TempSonde** | Probe measurement in degrees Celsius | |
| **Step1Temp** | The temperature to reach during the first heating phase | 100 |
| **Step1Speed** | The heating rate during the first heating phase in degrees per hour | 50 |
| **Step1Wait** | The duration of the first plateau in minutes | 5 |
| **Step2Temp** | The temperature to reach during the second heating phase | 570 |
| **Step2Speed** | The heating rate during the second heating phase in degrees per hour | 250 |
| **Step2Wait** | The duration of the second plateau in minutes | 15 |
| **Step3Temp** | The temperature to reach during the final heating phase | 1100 |
| **Step3Speed** | The heating rate during the final heating phase in degrees per hour | 200 |
| **Step3Wait** | The duration of the final plateau in minutes | 20 |
| **Step4Speed** | Cooling phase rate in °C/h | 150 |
| **Step4Target** | The temperature at which the program ends and displays a success message | 200 |

### Settings Parameters (SettingsParams structure)

| Variable | Function | Default Value |
|----------|----------|---------------|
| **Kp** | PID Proportional gain | 2.5 |
| **Ki** | PID Integral gain | 0.03 |
| **Kd** | PID Derivative gain (not used, always 0) | 0.0 |
| **maxDelta** | Temperature tolerance to consider plateau reached (°C) | 10 |
| **maxTemp** | Maximum kiln temperature - safety limit (°C) | 1200 |
| **pcycle** | PWM cycle duration (milliseconds) | 1000 |

**Note:** All phase temperatures (Step1Temp, Step2Temp, Step3Temp) are automatically constrained to not exceed **maxTemp** value for safety.

## Notes for Programming Choices

- Always use functions that do not stop the program like `millis()` to maintain maximum fluidity. Avoid using the `delay()` function.
- Do not exceed **31,232 bytes** for the program
- **Optional Features:** Use `#define` to enable/disable features:
  - `ENABLE_LOGGING`: Serial logging for debugging (~250 bytes)
  - `ENABLE_GRAPH`: Temperature graph display (~800 bytes)
- **Float to String Conversion:** On Arduino, `snprintf()` with `%f` format specifier is not always supported or may produce "?" characters. **Always use `dtostrf()` function** to convert float values to strings before using them with `snprintf()`.
  - Example: `dtostrf(floatValue, 3, 1, buffer);` converts float to string with 3 total characters and 1 decimal place
  - Then use: `snprintf(finalBuffer, size, "Label:%s", buffer);`

## Additional Specifications (Answered Requirements)

### PID Controller Parameters

- **Kp (Proportional Gain):** 2.5 (modifiable via Settings, range 0.0-10.0)
- **Ki (Integral Gain):** 0.03 (modifiable via Settings, range 0.0-1.0)
- **Kd (Derivative Gain):** Not used (removed - unsuitable for high thermal inertia systems)
- **Power Change Limiting:** Maximum 10% change per PWM cycle to prevent sudden changes
- **Output Limits:** PowerHold constrained between 0% and 100%
- **Max Delta:** Temperature tolerance for phase completion (default 10°C, range 1-50°C)
- **PID Update Interval:** 1000ms (1 Hz, adapted to kiln thermal inertia)

### User Interface & Navigation

#### Encoder Navigation (prog_OFF mode)
- **Navigation Order:** Scroll through all parameters sequentially in this order:
  - Settings → Step1Speed → Step1Temp → Step1Wait → Step2Speed → Step2Temp → Step2Wait → Step3Speed → Step3Temp → Step3Wait → Step4Speed → Step4Target
- **Selection Indication:** Selected item appears with inverted display (black background, white text)
- **Editing Indication:** Thin outline drawn around the selected element being edited
- **End of List Behavior:** Cycle from first to last items (circular navigation)

#### Button Behavior
- **Encoder Click (short press):** Enter/exit edit mode for selected parameter
- **Encoder Click (long press, >1 second):** Switch to temperature graph page
- **Push Button:** Toggle between prog_OFF and prog_ON states
  - When pressed during prog_ON: Stop immediately and return to prog_OFF
  - Turn off relay immediately when stopping
- **Debouncing:** Use default debounce timing (typically 50ms)

### Display Configuration

#### Screen Layout
- **Organization:** Single fixed page displaying all information (no scrolling)
- **Screen Refresh Rate:** 500ms (2 times per second)
- **Font Sizes:** Use defaults from U8g2 library, sized to fit all information on 128x64 screen

#### Display Style (Monochrome OLED)
- **Background:** Black
- **Normal Text:** White pixels
- **Selected Item:** Inverted display (white background, black text) or frame around item
- **Editing Item:** Filled box with inverted text
- **Note:** Display is monochrome (black and white only), no colors or grayscale

#### Display Pages
1. **Main Page:** Standard prog_OFF or prog_ON display
2. **Graph Page:** Full-screen temperature graph showing:
   - Expected temperature curve (target profile)
   - Actual measured temperature
   - Accessible via long press on encoder button
   - Return to main page with another long press or encoder click

### Temperature Control Logic

#### Setpoint Calculation
- **Formula:** `targetTemp = phaseStartTemp + (heatingRate × timeElapsed / 3600000)`
  - Uses `phaseStartTemp` (temperature at phase start) not current temperature
  - `timeElapsed` is in milliseconds
  - This ensures smooth progression independent of actual temperature fluctuations
- **Time Base:** Calculate from phase start time using `millis()`
- **Phase Completion:** Phase is considered "reached" when temperature is within `maxDelta`°C of target AND above target temperature

#### Overshoot Handling
- **Method:** Reduce heating power gradually using PID controller
- **Never:** Turn off completely unless emergency stop

#### Active Cooling Control
- **Mechanism:** Kiln cannot cool faster than natural cooling, but can maintain slower cooling rate by adding heat
- **Control:** During cooldown phase, use same PID control as heating
- **Target Calculation:** Same as heating phases - start from current temp, follow the cooling rate

### Error Handling & Safety

#### Thermocouple Failure
1. **Initial Response:** Display warning message at top of screen, continue operating for 2 minutes
2. **If Persists:** After 2 minutes of failed readings:
   - Display error: "Failed reading temp for 2min, Heating stopped"
   - Stop heating immediately (relay OFF)
   - Wait for push button click to acknowledge and return to prog_OFF

#### Error Display
- **Format:** Error message at top of screen
- **Display:** Standard white text on black background (monochrome)
- **Lower Section:** Continue showing other information when possible
- **Critical Errors:** Replace entire screen with error message

#### Safety Features
- Relay must be OFF when in prog_OFF state
- Relay must turn OFF immediately on any critical error
- Heating stops immediately if temperature reading fails for 2 minutes
- Push button always allows emergency stop

### Data Persistence (EEPROM)

#### Settings to Save
- All phase temperatures (Step1Temp, Step2Temp, Step3Temp)
- All phase speeds (Step1Speed, Step2Speed, Step3Speed, Step4Speed)
- All phase wait times (Step1Wait, Step2Wait, Step3Wait)
- All settings parameters (Pcycle, Kp, Ki, maxDelta)
- Cooldown target (Step4Target)

#### Save Trigger
- Save to EEPROM only when exiting edit mode (encoder click to deselect parameter)
- Do not save on every value change (to reduce EEPROM wear)

#### Startup Behavior
- Load last saved settings from EEPROM on power-up
- If EEPROM has never been written (first use), use default values from variables table
- **Smart Hot Resume:** When starting a program, the system automatically detects the appropriate phase based on current temperature:
  - If temp < Step1Temp: Start at Phase 1
  - If temp < Step2Temp: Start at Phase 2 (hot resume)
  - If temp < Step3Temp: Start at Phase 3 (hot resume)
  - Otherwise: Start cooldown (Phase 4)

### Additional Features

#### Temperature Reading Optimization
- **Reading Interval:** Temperature is read every 500ms (TEMP_READ_INTERVAL)
- **Temperature Cache:** Uses cached value between reads to reduce SPI communication overhead
- **Valid Range:** -200°C to 2000°C (values outside this range are considered invalid)

#### Time Display
- **Phase Elapsed Time:** Display time elapsed in current phase (format: HH:MM or MM:SS)
- **Total Remaining Time:** Calculate and display estimated time remaining for entire program
- **Calculation:** Based on current phase, remaining phases, and heating/cooling rates

#### Temperature Graph
- **Access:** Long press (>1 second) on encoder button
- **Display:**
  - X-axis: Time
  - Y-axis: Temperature
  - Two curves:
    - Expected profile (planned temperature curve)
    - Actual measured temperature (real-time data)
- **Exit:** Long press encoder again or click encoder button
- **Update:** Graph updates in real-time during prog_ON

### State Machine Details

#### prog_OFF State
- Display all firing program parameters
- Allow navigation and editing with encoder
- Relay is OFF
- PowerHold is 0%
- Save changes to EEPROM when exiting edit mode
- Wait for push button to start program

#### prog_ON State
- Display current status and all phases
- Show current phase information clearly
- Display temperature readings, target, and power percentage
- Show current temperature and target temperature
- Show relay status and heating percentage
- No editing allowed
- Long press encoder to view graph
- Push button stops program immediately

#### Phase Transitions
- **Automatic:** Transition occurs when phase completion conditions are met
- **Heating Phases (1-3):** Complete when temperature is within 5°C above target, then wait for duration
- **Cooldown Phase (4):** Complete when temperature reaches Step4Target
- **Final:** Return to prog_OFF when cooldown completes

