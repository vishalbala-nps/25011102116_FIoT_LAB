# EXPERIMENT DETAILS

**EXP:** CONTINUOUS ASSESSMENT 13

**DATE:** DD/MM/YYYY

**NAME OF EXP:** BME280 Sensor Interfacing Using I²C and Bosch Sensor API

**HARDWARE SETUP / PLATFORM:** CubeMX + STM32CubeIDE + STM32 Nucleo Development Board NUCLEO-F401RE (STM32F401RET6) + Bosch BME280 Sensor + Wemos D1 Mini (ESP8266) Wi-Fi Module + USB-to-TTL Serial Converter

---

## Overview

Interface a BME280 environmental sensor with the NUCLEO-F401RE over I²C using the official Bosch Sensortec BME280 Sensor API, obtain compensated temperature, pressure and humidity, serve them as JSON over HTTP through an ESP8266, and analyse them in Node-RED.

**Expected outcome:** Compensated readings appear on the UART terminal, a browser receives them as JSON, and Node-RED computes their minimum, maximum and average.

## Circuit Diagram

- **Development Board:** NUCLEO-F401RE
- **Microcontroller IC:** STM32F401RET6
- **I²C Pins:** PB6 (SCL), PB7 (SDA) → BME280
- **UART1 Pins (Wemos):** PA9 (USART1_TX) → Wemos RX, PA10 (USART1_RX) ← Wemos TX
- **UART6 Pins (PC):** PC6 (USART6_TX) → converter RX, PC7 (USART6_RX) ← converter TX
- **Peripherals:** BME280, Wemos D1 Mini (ESP8266) running AT firmware, USB-to-TTL converter to PuTTY
- **Power Supply:** BME280 from 3.3 V, Wemos from 5 V, common GND

![Circuit Diagram](circuit.png)

## Tasks

| Task | What it does | Outcome |
|---|---|---|
| [TASK 13A BME280 Sensor Interfacing Using I²C and Bosch Sensor API](TASK13A/) | Reads the chip ID and prints compensated T / P / H over UART every second | `Temperature : XX.XX °C`, `Pressure : XXXX.XX hPa`, `Humidity : XX.XX %RH` |
| [TASK 13B Raw HTTP Response with BME280 Data](TASK13B/) | ESP8266 HTTP server answers `GET /` with the readings as JSON (`application/json`) | Browser shows `{"status":"success",…}` |
| [TASK 13C Node-RED Statistics of the BME280 Data](TASK13C/) | Node-RED polls the server and computes running min / max / average | Statistics table of the BME280 data |

Each task folder contains its STM32CubeMX / STM32CubeIDE project and a README with the task's hardware, core logic, pseudocode, code base and inference.
