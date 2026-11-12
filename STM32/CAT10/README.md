# EXPERIMENT DETAILS

**EXP:** CONTINUOUS ASSESSMENT 10

**DATE:** DD/MM/YYYY

**NAME OF EXP:** STM32–Wemos Wi-Fi Modem Interface Using AT Commands

**HARDWARE SETUP / PLATFORM:** CubeMX + STM32CubeIDE + STM32 Development Board Blue Pill STM32F103C8T6 + Wemos D1 Mini (ESP8266) Wi-Fi Module + USB-to-TTL Serial Converter + PuTTY

---

## Overview

Interface the STM32 with a Wemos D1 Mini (ESP8266) through UART and configure the Wi-Fi modem using AT commands. One UART carries the AT commands to the Wemos and a second UART shows the results on the PC terminal.

**Expected outcome:** The STM32 communicates with the ESP8266, explores its AT command set, and connects it to the Wi-Fi network, reporting the IP and MAC address on the PC terminal.

## Circuit Diagram

- **Development Board:** STM32 Blue Pill
- **Microcontroller IC:** STM32F103C8T6 (ARM Cortex-M3, 72 MHz)
- **UART1 Pins (Wemos):** PA9 (USART1_TX) → Wemos RX, PA10 (USART1_RX) ← Wemos TX
- **UART2 Pins (PC):** PA2 (USART2_TX) → converter RX, PA3 (USART2_RX) ← converter TX
- **Peripherals:** Wemos D1 Mini (ESP8266) running AT firmware, USB-to-TTL (3.3 V) converter to PuTTY
- **Power Supply:** Wemos powered from 5 V; STM32, Wemos and converter share a common GND

| UART | Connection | Purpose |
|---|---|---|
| USART1 | STM32 ↔ Wemos | AT commands and responses |
| USART2 | STM32 ↔ PC | Debug / display |

![Circuit Diagram](circuit.png)

## Tasks

| Task | What it does | Outcome |
|---|---|---|
| [TASK 9A Establish STM32 ↔ Wemos UART Communication](TASK10A/) | Sends `AT` to the Wemos and shows `>> AT` / `<< OK` on the PC terminal | STM32 ↔ Wemos UART link proven |
| [TASK 9B Explore the ESP8266 AT Command Interface](TASK10B/) | Sends `AT`, `AT+GMR`, `AT+CWMODE=1`, `AT+CWLAP` and displays each response | AT command interface explored |
| [TASK 9C Connect Wemos to Wi-Fi](TASK10C/) | Sets station mode, joins the Wi-Fi network and reads the IP / MAC with `AT+CIFSR` | Wemos connected to Wi-Fi; IP and MAC reported |

Each task folder contains its STM32CubeMX / STM32CubeIDE project and a README with the task's hardware, core logic, pseudocode, code base and inference.
