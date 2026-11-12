# EXPERIMENT DETAILS

**EXP:** CONTINUOUS ASSESSMENT 7

**DATE:** DD/MM/YYYY

**NAME OF EXP:** 16×2 LCD Driver Development and Dynamic Display Control

**HARDWARE SETUP / PLATFORM:** AT89S52 Development Kit (11.0952 MHz crystal) + Keil µVision (C51) + 16×2 LCD (HD44780) + 4×4 Matrix Keypad

---

## Overview

Develop a 4-bit driver for a 16×2 HD44780-compatible character LCD on the AT89S52 directly from the datasheet: initialize the LCD, display characters and strings, position the cursor, update values dynamically, and move a character around the display with the 4×4 keypad.

**Expected outcome:** Text and changing values appear on both rows of the LCD, and keys 2 / 8 / 4 / 6 move a character up, down, left and right.

## Circuit Diagram

- **Microcontroller:** AT89S52 (8051 core) on the AT89S52 development kit
- **LCD Pins:** LCD plugged into the DS1 header – RS → P3.2 (D8), E → P3.3 (D7), D4–D7 → P3.4–P3.7 (D5–D2), RW → GND (4-bit mode)
- **Keypad Pins:** Built-in 4×4 matrix keypad on Port 2 – rows P2.7–P2.4, columns P2.3–P2.0
- **Peripherals:** Built-in 16×2 character LCD, built-in 4×4 matrix keypad
- **Power Supply:** 5V DC

![Circuit Diagram](circuit.png)

## Tasks

| Task | What it does | Outcome |
|---|---|---|
| [TASK 7A LCD Initialization and Character/String Display](TASK7A/) | Initializes the LCD in 4-bit mode and shows a character and strings on both rows | LCD driver working; text on both rows |
| [TASK 7B Cursor Positioning and Dynamic Value Update](TASK7B/) | Adds `lcd_goto()`, `lcd_puts()` and `lcd_print_number()`; updates a value while shifting the display | Cursor positioning and dynamic updates |
| [TASK 7C 4×4 Keypad-Based Character Movement](TASK7C/) | Keys 2 / 8 / 4 / 6 of the keypad move a `*` around the 16×2 display | Interactive keypad-controlled character |

Each task folder contains its Keil µVision project (`.uvproj`, C source, `STARTUP.A51`, `Objects/*.hex`) and a README with the task's hardware, core logic, pseudocode, code base and inference.
