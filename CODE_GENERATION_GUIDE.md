# Code Generation Guide for Ceramic Kiln Controller

This guide will help you use your prompt file to generate the Arduino code for your kiln controller.

## Step 1: Review Your Prompt File

Before generating code, make sure your prompt file (`Prompt_CalciferAi_EN.md`) contains all necessary information. See the "Missing Information" section below.

## Step 2: Choose Your AI Coding Assistant

You can use:
- **ChatGPT** (chat.openai.com)
- **Claude** (claude.ai)
- **GitHub Copilot** (if you have it)
- **Cursor AI** (if you're using Cursor editor)
- **Google Bard/Gemini**

## Step 3: Prepare Your Prompt

### Basic Prompt Template

Copy this template and customize it:

```
I need you to generate Arduino code for a ceramic kiln temperature controller. 

Please read and analyze the complete specification document I'm providing, then generate optimized Arduino code that follows all requirements.

SPECIFICATIONS:
[Paste the entire content of Prompt_CalciferAi_EN.md here]

REQUIREMENTS:
1. Generate all code files as specified in the "File Organization" section
2. Use only the libraries mentioned: U8g2lib, Adafruit_MAX31856, and Encoder
3. Follow all pin assignments exactly as specified
4. Use millis() instead of delay() for non-blocking code
5. Keep code size under 31,232 bytes
6. Include proper error handling
7. Add comments explaining complex logic

Please provide:
- Complete code for all files
- Brief explanation of the code structure
- Any assumptions you made
```

### Advanced Prompt (More Detailed)

If you want more control, use this:

```
I need Arduino code for a ceramic kiln controller. Here are the specifications:

[Paste Prompt_CalciferAi_EN.md content]

ADDITIONAL REQUIREMENTS:
1. Code Structure:
   - Main file: lucia.ino (setup and loop)
   - Display module: display.h and display.cpp
   - Temperature module: Temperature.h and Temperature.cpp

2. PID Controller:
   - Implement proportional and integral control
   - Use the PowerHold variable to control PWM duty cycle
   - CycleLength = 1000ms (1 second)
   - Calculate setpoint based on heating rate (°C/h)

3. State Machine:
   - prog_OFF: Settings screen, encoder navigation
   - prog_ON: Running program, no editing allowed
   - Phase transitions: automatic when conditions met

4. Display:
   - Use U8g2 library for SH1106
   - Show all specified information
   - Indicate selected/editing items
   - Gray out completed phases

5. Safety:
   - Handle thermocouple faults
   - Emergency stop capability
   - Relay must be OFF when stopped

Please generate complete, compilable code with comments.
```

## Step 4: Generate the Code

1. **Copy your prompt file content**
2. **Paste it into your chosen AI assistant**
3. **Use one of the prompt templates above**
4. **Wait for the code generation**
5. **Review the generated code**

## Step 5: Review Generated Code

Check that the code includes:

- [ ] All three files: `lucia.ino`, `display.h/cpp`, `Temperature.h/cpp`
- [ ] Correct pin assignments
- [ ] All required libraries included
- [ ] No `delay()` functions (only `millis()`)
- [ ] PID controller implementation
- [ ] State machine for prog_OFF/prog_ON
- [ ] Encoder navigation logic
- [ ] Display update functions
- [ ] Temperature reading from MAX31856
- [ ] PWM control for relay
- [ ] Error handling

## Step 6: Test and Iterate

1. **Save the code files** in a folder named `lucia`
2. **Open in Arduino IDE**
3. **Install missing libraries** if needed
4. **Compile** (Verify button)
5. **Fix any errors** - ask the AI to fix them
6. **Test incrementally** - start with display, then encoder, then temperature

## Step 7: Refinement Prompts

If the code needs adjustments, use prompts like:

```
The code you generated has [specific issue]. Please fix:
- [Describe the problem]
- [What should happen instead]

Here's the current code:
[Paste relevant code section]
```

Or:

```
The code compiles but [describe behavior issue]. Please modify:
- [Specific function or section]
- [Expected behavior]
```

---

## Missing Information Analysis

After reviewing your prompt file, here are areas that need clarification before generating optimal code:

### Critical Missing Information:

1. **PID Controller Parameters**
   - What are the Kp (proportional) and Ki (integral) values? use default
   - Should there be derivative (Kd) control? Let it 0
   - What are the output limits? I don't understand the question

2. **Encoder Navigation Details**
   - How many menu items total? Not yet defined
   - What's the navigation order? Selection navigation follow the Temperature, speed and wait from the 1st phase to the last.
   - How is the selected item highlighted on screen? The selected item appears in inverted color.
   - What happens when you reach the first/last item? Selection cycle from first to last items on the screen.

3. **Display Formatting**
   - How should "grayed out" lines be displayed? (lower brightness? different color? strikethrough?) "Greyed out" elements are shown in dimmed color. Background is black. 
   - What font size to use? use default
   - How to fit all information on 128x64 screen? decide the size of the elements to create a readable simple design, colors are White, blue, red, dark background.
   - Screen refresh rate? use default.

4. **Setpoint Calculation**
   - How exactly is the target temperature calculated from heating rate (°C/h)? Start from the actual temperature and follow the rate.
   - Is it based on time elapsed since phase start? yes
   - How to handle overshoot? Decide a default Kp and Ki for a kiln. I will adjust it if necesary.

5. **Cooling Phase Mechanism**
   - Does the kiln cool naturally (just turn off heat)? 
   - Or is there active cooling? Use active cooling. There is no way to cool faster the kiln but keep the cooling speed by heating if necessary.
   - How is cooling rate controlled? Cooling rate follow the same process. There is a target temperature and the "temperature" programm adjust the heating.

6. **Error Handling**
   - What happens if thermocouple fails? If anything fails, program should display an error message. A click on the on/off button will exit the information. Program restart to the first state. The heating stop immediatly. 
   - What to display on screen during errors? the display popup a red message alert and the name of the error.
   - Should program stop or continue? Heating program stop when error.

7. **Settings Persistence**
   - Should settings be saved to EEPROM? Yes all phases informations should be stored.
   - Load defaults on startup? on startup load last settings.

8. **Button Behavior**
   - Debounce timing? use default
   - Long press vs short press? only short press

### Questions to Answer:

Please provide answers to these questions so we can complete the specification:

#### 1. PID Controller Settings

**Question 1.1:** What proportional gain (Kp) value should be used?
- [ x] I don't know - use a default value (suggest: 2.0)
- [ ] Use this specific value: _______

**Question 1.2:** What integral gain (Ki) value should be used?
- [ x] I don't know - use a default value (suggest: 0.5)
- [ ] Use this specific value: _______

**Question 1.3:** Should the controller limit how fast it can change the heating power?
- [ x] Yes, limit to prevent sudden changes (suggest: max 10% change per cycle)
- [ ] No, allow full range changes

#### 2. Encoder Navigation

**Question 2.1:** How should the encoder navigate through settings in prog_OFF mode?
- [x ] Scroll through all parameters in order (Step1Temp → Step1Speed → Step1Wait → Step2Temp → etc.)
- [ ] Group by phase (all Step1 settings together, then Step2, etc.)
- [ ] Other arrangement: _______

**Question 2.2:** How should the selected item be indicated on screen?
- [ ] Arrow (→) before the selected line
- [x ] Highlight/invert the text
- [ ] Blinking cursor
- [ ] Other: _______

**Question 2.3:** When editing a value, how should it be shown?
- [ ] Same as selection, but value changes when rotating
- [ ] Value flashes or changes color
- [ x] Other: thin outline on the selected element

#### 3. Display Details

**Question 3.1:** How should "grayed out" completed phases be displayed?
- [x] Use lower brightness/dimmed text
- [ ] Use a different font style
- [ ] Draw a line through the text
- [ ] Just show them smaller
- [ ] Other: _______

**Question 3.2:** How often should the screen refresh?
- [ ] Every 100ms (10 times per second)
- [ ] Every 250ms (4 times per second)
- [x] Every 500ms (2 times per second)
- [ ] Other:

**Question 3.3:** The screen is 128x64 pixels. How should information be organized?
- [ ] Show all phases in a scrollable list
- [ ] Show only current phase + next phase
- [ ] Use multiple screens/pages
- [ ] Other: Organize all information to fit in 1 fix page_______

#### 4. Temperature Control Logic

**Question 4.1:** How should the target temperature be calculated during heating phases?
- [x] Start from current temp, add (heating_rate × time_elapsed / 3600)
- [ ] Start from previous phase temp, add (heating_rate × time_elapsed / 3600)
- [ ] Other method: _______

**Question 4.2:** What should happen if the temperature overshoots the target?
- [ ] Turn off heating immediately and wait
- [x] Reduce heating power gradually
- [ ] Continue to next phase anyway
- [ ] Other: _______

**Question 4.3:** How close to target temperature should it get before considering the phase "reached"?
- [ ] Within 2°C
- [ ] Within 5°C
- [ ] Within 10°C
- [ ] Exact match only
- [x] Other: Within 5°C and above the target.

#### 5. Cooling Phase

**Question 5.1:** How does cooling work?
- [ ] Natural cooling - just turn off the heating element
- [x] Active cooling - need to control a cooling system
- [ ] Other: _______

**Question 5.2:** If natural cooling, how is the "cooling rate" used?
- [ ] Just monitor and display the rate
- [ ] Use it to calculate when cooling phase will complete
- [ ] Not used - just wait until target temp reached
- [ ] Other: _______

#### 6. Error Handling

**Question 6.1:** What should happen if the thermocouple fails or gives bad readings?
- [ ] Stop the program immediately and show error
- [ ] Continue with last known good temperature
- [ ] Show warning but continue
- [x] Other: Show warning but continue 2 minutes. If the reading fail to continue, display a "Failed rinding temp for 2min, Heating stopped" and stop. Wait for a click to go back to menu.

**Question 6.2:** What should be displayed on screen during an error?
- [ ] Error message replacing normal display
- [x] Error message at top, other info below
- [ ] Blinking error indicator
- [ ] Other: _______

#### 7. Settings Storage

**Question 7.1:** Should the firing program settings be saved?
- [x] Yes, save to EEPROM so they persist after power off
- [ ] No, use defaults each time (settings reset on power cycle)
- [ ] Optional - user can choose

**Question 7.2:** If saving, what should trigger a save?
- [ ] Automatically when any value changes
- [x] Only when exiting edit mode
- [ ] Manual save action
- [ ] Other: _______

#### 8. Button Behavior

**Question 8.1:** How should button presses be handled?
- [x] Simple press detection (any press = action)
- [ ] Debounce needed (ignore rapid presses)
- [x] Long press vs short press (different actions) only for graph page access, on the encoder click.

**Question 8.2:** If using the push button to stop the program, should it:
- [x] Stop immediately and return to prog_OFF
- [ ] Ask for confirmation
- [ ] Pause and allow resume
- [ ] Other: _______

#### 9. Additional Features

**Question 9.1:** Should there be a way to see elapsed time for current phase?
- [x] Yes, display on screen
- [ ] No, not needed

**Question 9.2:** Should there be a way to see total program time remaining?
- [x] Yes, calculate and display
- [ ] No, not needed

**Question 9.3:** Any other features you want?
- [x] Temperature graph/history
- [ ] Multiple saved programs
- [ ] Serial output for logging
- [x] Other: Display an alternative page with full graph expected + real measures. Graph page is accessible with a long click on the encoder button
---

## How to Use These Questions

1. **Answer the questions** that are important to you
2. **Mark "I don't know" or "use default"** for things you're unsure about
3. **Add your answers** to the prompt file or create a separate "ANSWERS.md" file
4. **Include the answers** when generating code so the AI can make informed decisions

## Next Steps

Once you've answered the questions (or decided on defaults):

1. Update your `Prompt_CalciferAi_EN.md` with the answers, OR
2. Create a new section in the prompt file with "Additional Specifications" OR
3. Include the answers directly in your code generation prompt

Then proceed with Step 3 of this guide to generate your code!

