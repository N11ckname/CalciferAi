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

Phase 1 : 100 °C/h >150 °C, wait 60 min (grayed line because phase completed)  
Phase 2 : 150 °C/h > 600 °C, wait 10 min (grayed line because phase completed)  
Phase 3 : 600 °C/h up to 980 °C, wait 10 min (white line, phase in progress)  
Cooldown : 150 °C/h <500°C (grayed line, phase to come)  
HEATING : Temp 155°C, Target 167°C (line in green)  
ON, 60% (line in red)

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
| **Step3Stage** | The duration of the final plateau in minutes | 20 |
| **Step4Speed** | Cooling phase rate in °C/h | 150 |
| **Step4Target** | The temperature at which the program ends and displays a success message | 200 |

## Notes for Programming Choices

- Always use functions that do not stop the program like `millis()` to maintain maximum fluidity. Avoid using the `delay()` function.
- Do not exceed **31,232 bytes** for the program

## Additional Specifications (Answered Requirements)

### PID Controller Parameters

- **Kp (Proportional Gain):** 2.0
- **Ki (Integral Gain):** 0.5
- **Kd (Derivative Gain):** 0 (no derivative control)
- **Power Change Limiting:** Maximum 10% change per PWM cycle to prevent sudden changes
- **Output Limits:** PowerHold constrained between 0% and 100%

### User Interface & Navigation

#### Encoder Navigation (prog_OFF mode)
- **Navigation Order:** Scroll through all parameters sequentially in this order:
  - Step1Temp → Step1Speed → Step1Wait → Step2Temp → Step2Speed → Step2Wait → Step3Temp → Step3Speed → Step3Stage → Step4Speed → Step4Target
- **Selection Indication:** Selected item appears with inverted colors (text and background swapped)
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

#### Color Scheme
- **Background:** Black
- **Normal Text:** White
- **Current Phase:** White (bright)
- **Completed Phases:** Dimmed/grayed out (lower brightness)
- **Future Phases:** Dimmed/grayed out (lower brightness)
- **Heating Status:** Green text for "HEATING"
- **Relay Status:** Red text for relay percentage
- **Error Messages:** Red text

#### Display Pages
1. **Main Page:** Standard prog_OFF or prog_ON display
2. **Graph Page:** Full-screen temperature graph showing:
   - Expected temperature curve (target profile)
   - Actual measured temperature
   - Accessible via long press on encoder button
   - Return to main page with another long press or encoder click

### Temperature Control Logic

#### Setpoint Calculation
- **Formula:** `targetTemp = currentTemp + (heatingRate × timeElapsed / 3600)`
- **Time Base:** Calculate from phase start time using `millis()`
- **Phase Completion:** Phase is considered "reached" when temperature is within 5°C of target AND above target temperature

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
- **Color:** Red text
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
- All phase wait times (Step1Wait, Step2Wait, Step3Stage)
- Cooldown target (Step4Target)

#### Save Trigger
- Save to EEPROM only when exiting edit mode (encoder click to deselect parameter)
- Do not save on every value change (to reduce EEPROM wear)

#### Startup Behavior
- Load last saved settings from EEPROM on power-up
- If EEPROM has never been written (first use), use default values from variables table

### Additional Features

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
- Highlight current phase in white
- Dim completed phases
- Dim future phases
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

