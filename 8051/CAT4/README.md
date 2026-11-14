# EXPERIMENT DETAILS

**EXP:** CONTINUOUS ASSESSMENT 4

**DATE:** 03/08/2026

**NAME OF EXP:** Human-Machine Interface using Matrix Keypad, LEDs, and Seven-Segment Display

**HARDWARE SETUP / PLATFORM:** AT89S52 Development Kit (11.0952 MHz crystal) + Keil µVision (C51) + 4×4 Matrix Keypad + Seven-Segment Display + 4 LEDs

---

## Overview

Build a simple human-machine interface on the AT89S52: a 4×4 matrix keypad is the input, the seven-segment display shows the key pressed, and four LEDs perform the operation selected by that key.

**Expected outcome:** Every key pressed is shown on the seven-segment display and triggers its LED operation (single LEDs, running pattern, alternating pattern, binary counter, blink, all off, reset).

## Circuit Diagram

- **Microcontroller:** AT89S52 (8051 core) on the AT89S52 development kit
- **Keypad:** Built-in matrix keypad on Port 2 – rows P2.7–P2.4, columns P2.3–P2.1
- **Seven-Segment Display:** Built-in common-cathode display on Port 0 – segments a–g on P0.0–P0.6
- **LEDs:** 4 external LEDs on P1.1, P1.3, P1.5, P1.7 (active-high, cathode to GND)
- **Power Supply:** 5V DC

![Circuit Diagram](circuit.png)

## Tasks

| Task | What it does | Outcome |
|---|---|---|
| [TASK 4A Keypad-Controlled Seven-Segment Display and LED Operations](TASK4A/) | Scans the keypad, shows the key on the 7-segment display and runs the matching LED operation | Keypad-driven display and LED modes |

Each task folder contains its Keil µVision project (`.uvproj`, C source, `STARTUP.A51`, `Objects/*.hex`) and a README with the task's hardware, core logic, pseudocode, code base and inference.
