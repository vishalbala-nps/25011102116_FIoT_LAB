# EXPERIMENT DETAILS

**EXP:** CONTINUOUS ASSESSMENT 8

**DATE:** DD/MM/YYYY

**NAME OF EXP:** STM32 GPIO Bring-Up and Switch-Controlled LED

**HARDWARE SETUP / PLATFORM:** CubeMX + STM32CubeIDE + STM32 Development Board Blue Pill STM32F103C6T6A + Push-Button

---

## Overview

Bring up the GPIO of the STM32 Blue Pill with CubeMX and the HAL library, and control the on-board LED from an external push-button. The experiment progresses from reading the switch directly, to event-based control with software debouncing, to classifying short and long presses without blocking delays.

**Expected outcome:** The LED responds reliably to the push-button: one toggle per press, and a long press switches it into a continuous blinking mode.

## Circuit Diagram

- **Development Board:** STM32 Blue Pill
- **Microcontroller IC:** STM32F103C6T6A (ARM Cortex-M3, 72 MHz from the 8 MHz HSE crystal)
- **GPIO Pins:** PC13 (output, push-pull), PA15 (input, internal pull-up)
- **Peripherals:** Onboard LED on PC13 (active-low), push-button between PA15 and GND
- **Power Supply:** 3.3 V from the board; push-button returns to GND

![Circuit Diagram](circuit.png)

## Tasks

| Task | What it does | Outcome |
|---|---|---|
| [TASK 8A GPIO Input/Output](TASK8A/) | Follows the push-button: LED ON while pressed, OFF when released | LED state mirrors the switch |
| [TASK 8B Event-Based Control](TASK8B/) | Toggles the LED once per press, ignoring contact bounce (≥ 30 ms) | One clean toggle per press |
| [TASK 8C Short Press vs Long Press](TASK8C/) | Short press toggles the LED; long press (≥ 1 s) starts / stops 200 ms blinking | Press duration measured with `HAL_GetTick()`, no blocking delay |

Each task folder contains its STM32CubeMX / STM32CubeIDE project and a README with the task's hardware, core logic, pseudocode, code base and inference.
