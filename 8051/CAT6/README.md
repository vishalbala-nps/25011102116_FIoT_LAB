# EXPERIMENT DETAILS

**EXP:** CONTINUOUS ASSESSMENT 6

**DATE:** DD/MM/YYYY

**NAME OF EXP:** UART Driver and Serial Debugging

**HARDWARE SETUP / PLATFORM:** AT89S52 Development Kit (11.0952 MHz crystal) + Keil µVision (C51) + USB-to-TTL Serial Converter + PuTTY

---

## Overview

Write a UART driver for the 8051 and use it for serial debugging with a PC terminal (PuTTY) through a USB-to-TTL converter at 9600-8-N-1, with Timer 1 as the baud-rate generator.

**Expected outcome:** The 8051 transmits text to PuTTY continuously, and echoes back every character typed in the terminal.

## Circuit Diagram

- **Microcontroller:** AT89S52 (8051 core) on the AT89S52 development kit
- **UART Pins:** P3.1 (TXD) → USB-to-TTL RX, P3.0 (RXD) ← USB-to-TTL TX
- **Peripheral:** USB-to-TTL serial converter connected to the PC running PuTTY (9600-8-N-1)
- **Power Supply:** 5V DC; converter GND connected to board GND

![Circuit Diagram](circuit.png)

## Tasks

| Task | What it does | Outcome |
|---|---|---|
| [TASK 6A UART Initialization and Character Transmission](TASK6A/) | Initializes the UART and sends `Hello from 8051` repeatedly | Text appears in PuTTY; transmit path verified |
| [TASK 6B UART Echo](TASK6B/) | Receives each character from PuTTY and sends it back | Typed text is echoed; receive and transmit verified |

Each task folder contains its Keil µVision project (`.uvproj`, C source, `STARTUP.A51`, `Objects/*.hex`) and a README with the task's hardware, core logic, pseudocode, code base and inference.
