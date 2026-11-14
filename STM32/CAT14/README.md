# EXPERIMENT DETAILS

**EXP:** CONTINUOUS ASSESSMENT 14

**DATE:** 09/11/2026

**NAME OF EXP:** BME280 Sensor Interfacing Using I²C and (MQTT) using Custom Built ESP8266 API

**HARDWARE SETUP / PLATFORM:** CubeMX + STM32CubeIDE + STM32 Nucleo Development Board NUCLEO-F401RE (STM32F401RET6) + Bosch BME280 Sensor + Wemos D1 Mini (ESP8266) Wi-Fi Module + MQTT Broker (broker.emqx.io)

---

## Overview

Read compensated temperature, pressure and humidity from a BME280 over I²C with the Bosch API, and publish them as JSON to an MQTT broker (`broker.emqx.io:1883`) through an ESP8266, using a custom-built ESP8266 / MQTT library.

**Expected outcome:** Any MQTT client subscribed to `stm32/bme280` receives the live readings every second.

## Circuit Diagram

- **Development Board:** NUCLEO-F401RE
- **Microcontroller IC:** STM32F401RET6 (ARM Cortex-M4 with FPU, 84 MHz)
- **UART1 Pins (Wemos):** PA9 (USART1_TX) → Wemos RX, PA10 (USART1_RX) ← Wemos TX
- **Peripheral:** Wemos D1 Mini (ESP8266) running AT firmware
- **Network:** Wi-Fi access point → MQTT broker `broker.emqx.io`, TCP port 1883
- **Power Supply:** Wemos powered from 5 V, common GND

![Circuit Diagram](circuit.png)

## Tasks

| Task | What it does | Outcome |
|---|---|---|
| [TASK 14 BME280 Data Publishing over MQTT](TASK14/) | Connects to Wi-Fi and the MQTT broker with the ESP8266 library, then publishes BME280 readings as JSON every second | Readings received on topic `stm32/bme280` |

Each task folder contains its STM32CubeMX / STM32CubeIDE project and a README with the task's hardware, core logic, pseudocode, code base and inference.
