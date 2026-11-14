# EXPERIMENT DETAILS

**EXP:** CONTINUOUS ASSESSMENT 2

**DATE:** 20/07/2026

**NAME OF EXP:** 8051 Tool Chain Setup – Development Board – AT89S52

**HARDWARE SETUP / PLATFORM:** AT89S52 Development Kit (11.0952 MHz crystal) + Keil µVision (C51)

---

## Overview

Set up the 8051 toolchain (Keil µVision + C51) and bring up the AT89S52 development board: identify the board's parts, verify the supply, then write a minimal bare-metal C program that blinks the on-board LED at about 1 Hz by writing directly to the port register, without Arduino or HAL libraries. Finally, move the software delay into a reusable delay library.

**Expected outcome:** The board is verified, the toolchain programs the AT89S52, and the P1.0 LED blinks at about 1 Hz using register-level access and a custom `delay_ms()` library.

## Circuit Diagram

- **Microcontroller:** AT89S52 (8051 core) on the AT89S52 development kit
- **GPIO Port:** Port 1
- **GPIO Pin:** P1.0
- **Peripheral:** Built-in user LED (active-low)
- **Power Supply:** 5V DC

![Circuit Diagram](circuit.png)

## TASK 2A Hardware Board Bring-Up

**Hardware Used:**

- **Microcontroller:** AT89S52 (8051 core) on the AT89S52 development kit
- **Programmer:** ISP (In-System Programming) over SPI: P1.5 (MOSI), P1.6 (MISO), P1.7 (SCK), RST
- **User LED:** Built-in LED on P1.0 (active-low)
- **Power Supply:** 5V DC; VCC on pin 40, GND on pin 20

**Working Principle:**

The board components were identified, the power rail was verified with a digital multimeter, and the LED's GPIO port and pin were identified from the board schematic. The host PC was then connected to the board and the program was flashed to the AT89S52 over ISP.

| Parameter | Student Entry |
|---|---|
| MCU | AT89S52 |
| CPU Core | 8051 (MCS-51), 8-bit |
| Operating Voltage | 5 V (4.0 V – 5.5 V) |
| Debug Interface | No on-chip debugger; programmed through ISP over SPI |
| LED GPIO Port | Port 1 |
| LED GPIO Pin | P1.0 (built-in) |
| LED Active State | Active-low (the LED turns on when P1.0 = 0) |
| LED Frequency (Hz) | ≈ 1 Hz (0.5 s ON, 0.5 s OFF) |

**Bring-up Steps:**

1. Identify the microcontroller, programmer, power source, user LED and its GPIO pin.
2. Measure the 5 V supply between VCC (pin 40) and GND (pin 20) with a digital multimeter.
3. Identify the LED port and pin (P1.0) and its active-low wiring from the schematic.
4. Connect the board to the host PC, build the project in Keil µVision and flash the HEX file over ISP.

**Inference:**

The AT89S52 board was brought up successfully: the supply voltage was verified, the user LED was located on P1.0 (active-low), and the host PC could program the microcontroller.

This task has no program; the board is verified with the Task 2B program.

## Tasks

| Task | What it does | Outcome |
|---|---|---|
| [TASK 2B Bare-Metal LED Blink](TASK2B/) | Blinks the P1.0 LED at ≈ 1 Hz with a nested-loop software delay | LED blinks; register-level GPIO verified |
| [TASK 2C Customed Library Function for Delay Functionality](TASK2C/) | Moves the delay into `delay.h` / `delay.c` as `delay_ms()` and blinks with `delay_ms(500)` | Reusable millisecond delay library |

Each task folder contains its Keil µVision project (`.uvproj`, C source, `STARTUP.A51`, `Objects/*.hex`) and a README with the task's hardware, core logic, pseudocode, code base and inference.
