# EXPERIMENT DETAILS

**EXP:** CONTINUOUS ASSESSMENT 10C

**DATE:** DD/MM/YYYY

**NAME OF EXP:** HTTP Server - STM32–Wemos Wi-Fi Modem Using AT Commands

**HARDWARE SETUP / PLATFORM:** CubeMX + STM32CubeIDE + STM32 Development Board Blue Pill STM32F103C8T6 + Wemos D1 Mini (ESP8266) Wi-Fi Module + USB-to-TTL Serial Converter + PuTTY + Browser / Node-RED

---

## Overview

Develop an STM32-based HTTP node: the STM32 drives the ESP8266/Wemos through UART AT commands, configures it as a TCP server, receives an HTTP request through an `+IPD` frame, constructs an HTTP response and transmits it to a browser and to Node-RED. The ESP8266 / HTTP server functions are reused from the BME280 HTTP experiment (CAT12).

**Expected outcome:** A browser and a Node-RED flow both receive the "STM32 IoT Node" page served by the STM32 through the ESP8266.

## Circuit Diagram

- **Development Board:** STM32 Blue Pill
- **Microcontroller IC:** STM32F103C8T6 (ARM Cortex-M3, 72 MHz)
- **UART1 Pins (Wemos):** PA9 (USART1_TX) → Wemos RX, PA10 (USART1_RX) ← Wemos TX
- **UART2 Pins (PC):** PA2 (USART2_TX) → converter RX, PA3 (USART2_RX) ← converter TX
- **Peripherals:** Wemos D1 Mini (ESP8266) running AT firmware, USB-to-TTL (3.3 V) converter to PuTTY (115200-8-N-1)
- **Power Supply:** Wemos powered from 5 V; STM32, Wemos and converter share a common GND

![Circuit Diagram](circuit.png)

## Tasks

| Task | What it does | Outcome |
|---|---|---|
| [TASK 10C Configure ESP8266 as a TCP Server](TASK10C/) | Joins Wi-Fi, starts the TCP server (`AT+CIPMUX=1`, `AT+CIPSERVER=1,80`) and shows the raw ESP8266 traffic | `+IPD` frame of a browser request observed |
| [TASK 10D Observe the HTTP Request and Construct a Static HTTP Response](TASK10D/) | Parses the `+IPD` request and answers `GET /` with a static HTML page using a two-step `AT+CIPSEND` | Browser shows the STM32 IoT Node page |
| [TASK 10E Node-RED as a Second HTTP Client of STM32/ESP8266 HTTP Server](TASK10E/) | Node-RED Inject → HTTP Request → Debug flow queries the STM32 server | Node-RED receives the same HTTP response |

Each task folder contains its STM32CubeMX / STM32CubeIDE project and a README with the task's hardware, core logic, pseudocode, code base and inference.
