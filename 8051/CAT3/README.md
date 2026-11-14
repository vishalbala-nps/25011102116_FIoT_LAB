# EXPERIMENT DETAILS

**EXP:** CONTINUOUS ASSESSMENT 3

**DATE:** 27/07/2026

**NAME OF EXP:** Timer-Based LED Pattern Generator and Button Interrupt Controller

**HARDWARE SETUP / PLATFORM:** AT89S52 Development Kit (11.0952 MHz crystal) + Keil µVision (C51) + 4 LEDs + Push-Button

---

## Overview

Design a register-level GPIO controller in bare-metal Embedded C for the AT89S52: four LEDs are digital outputs and a push-button is a digital input. Each debounced button press moves the system to the next LED mode, so the program behaves as a four-state machine.

**Expected outcome:** Each press advances the LED behaviour: running LED → binary counter → alternating pattern → all-LED flash → running LED.

## Circuit Diagram

- **Microcontroller:** AT89S52 (8051 core) on the AT89S52 development kit
- **GPIO Port / Pins (LEDs):** Port 1 – P1.1 (LED1), P1.3 (LED2), P1.5 (LED3), P1.7 (LED4)
- **GPIO Port / Pins (Button):** Port 2 – P2.0 (keypad row, input), P2.5 (keypad column, held low)
- **Peripherals:** 4 external LEDs (active-high, anode to the port pin, cathode to GND); one key of the kit's built-in keypad as the push-button
- **Power Supply:** 5V DC

![Circuit Diagram](circuit.png)

## Tasks

| Task | What it does | Outcome |
|---|---|---|
| [TASK 3A Four-State LED Control System](TASK3A/) | Debounced button cycles through running LED, binary counter, alternating and flashing modes | Four-state LED controller |

Each task folder contains its Keil µVision project (`.uvproj`, C source, `STARTUP.A51`, `Objects/*.hex`) and a README with the task's hardware, core logic, pseudocode, code base and inference.
