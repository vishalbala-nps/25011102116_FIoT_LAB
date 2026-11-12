# IoT Lab – Embedded Systems Experiments (8051 and STM32)

Lab experiments for **25CS2741 Fundamentals of Internet of Things + Lab** (B.Tech CSE (IoT), Semester 3, Shiv Nadar University Chennai).

The experiments start with bare-metal register programming on the 8051, move to STM32 microcontrollers with STM32CubeMX and the HAL library, and finish with networked sensing: an STM32 reads a BME280 sensor and serves or publishes the data over Wi-Fi through an ESP8266, using HTTP and MQTT.

Each experiment folder (`CATn`) has a **README.md** index with the experiment details, date, overview, circuit diagram and a table of its tasks. Each task folder (`TASKnX`) contains the project and its own **README.md** with the hardware used, core logic, pseudocode, code base and inference.

## Experiments

### 8051 (AT89S52) – Bare-Metal Embedded C, Keil µVision

| # | Experiment | Key concepts | Project |
|---|---|---|---|
| [Cat 2](8051/CAT2/) | Bare-metal LED blink and custom delay library | SFR bit access (`sbit`), software delay, `delay_ms()` library | [`TASK2B`](8051/CAT2/TASK2B/), [`TASK2C`](8051/CAT2/TASK2C/) |
| [Cat 3](8051/CAT3/) | Button-controlled four-mode LED pattern generator | GPIO input, debouncing, state machine | [`TASK3A`](8051/CAT3/TASK3A/) |
| [Cat 4](8051/CAT4/) | HMI with matrix keypad, LEDs and seven-segment display | Keypad scanning, seven-segment codes, mode flags | [`TASK4A`](8051/CAT4/TASK4A/) |
| [Cat 5](8051/CAT5/) | Hardware timer-based LED blinking | Timer 0 Mode 1, preload calculation, overflow flag | [`TASK5A`](8051/CAT5/TASK5A/) |
| [Cat 6](8051/CAT6/) | UART driver and serial debugging | SCON/TMOD, Timer 1 baud-rate generation, TI/RI flags, echo | [`TASK6A`](8051/CAT6/TASK6A/), [`TASK6B`](8051/CAT6/TASK6B/) |
| [Cat 7](8051/CAT7/) | 16×2 LCD driver, dynamic display and keypad control | HD44780 4-bit interface, DDRAM addressing, keypad scanning | [`TASK7A`](8051/CAT7/TASK7A/), [`TASK7B`](8051/CAT7/TASK7B/), [`TASK7C`](8051/CAT7/TASK7C/) |

### STM32 – STM32CubeMX + HAL, Keil MDK-ARM / STM32CubeIDE

| # | Experiment | Board | Key concepts | Project |
|---|---|---|---|---|
| [Cat 8](STM32/CAT8/) | GPIO bring-up and switch-controlled LED (short vs long press) | Blue Pill (STM32F103C6) | GPIO input/output, pull-ups, `HAL_GetTick()` timing, debouncing | [`TASK8A`](STM32/CAT8/TASK8A/), [`TASK8B`](STM32/CAT8/TASK8B/), [`TASK8C`](STM32/CAT8/TASK8C/) |
| [Cat 9](STM32/CAT9/) | UART command-controlled LED | Blue Pill (STM32F103C6) | USART with HAL, echo, line-based command parsing, CR/LF | [`TASK9A`](STM32/CAT9/TASK9A/), [`TASK9B`](STM32/CAT9/TASK9B/) |
| [Cat 10](STM32/CAT10/) | STM32–Wemos (ESP8266) Wi-Fi modem interface and HTTP server | Blue Pill (STM32F103C8) | Two UARTs, ESP8266 AT commands, joining Wi-Fi, TCP server, `+IPD` requests, static HTTP response, Node-RED client | [`TASK10A`](STM32/CAT10/TASK10A/), [`TASK10B`](STM32/CAT10/TASK10B/), [`TASK10C`](STM32/CAT10/TASK10C/), [`TASK10D`](STM32/CAT10/TASK10D/), [`TASK10E`](STM32/CAT10/TASK10E/) |
| [Cat 11](STM32/CAT11/) | Cortex-M4 GPIO and user input | NUCLEO-F401RE | F4 clock tree (HSE bypass, PLL), on-board LED/button, short/long press | [`TASK11A`](STM32/CAT11/TASK11A/), [`TASK11B`](STM32/CAT11/TASK11B/), [`TASK11C`](STM32/CAT11/TASK11C/) |
| [Cat 12](STM32/CAT12/) | BME280 over I²C with the Bosch API, served over HTTP | NUCLEO-F401RE | I²C, Bosch Sensor API, UART output, JSON HTTP response, Node-RED statistics | [`TASK12A`](STM32/CAT12/TASK12A/), [`TASK12B`](STM32/CAT12/TASK12B/), [`TASK12C`](STM32/CAT12/TASK12C/) |
| [Cat 13](STM32/CAT13/) | MQTT publishing of BME280 data | NUCLEO-F401RE | MQTT CONNECT/PUBLISH packets, public broker, JSON payload | [`TASK13`](STM32/CAT13/TASK13/) |
| [Cat 14](STM32/CAT14/) | IoT project: server rack monitor | Blue Pill (STM32F103C6) | DHT11 single-wire timing, MQTT publishing, local LED/buzzer alarm, Node-RED dashboard, alert hysteresis, automatic shutdown, OpenWeatherMap | [`TASK14`](STM32/CAT14/TASK14/) |

## Hardware

| Hardware | Used in | Notes |
|---|---|---|
| AT89S52 development kit | Cat 2 – Cat 7 | 11.0952 MHz crystal; on-board LED (P1.0, active-low), matrix keypad, seven-segment display and 16×2 LCD; programmed over ISP |
| STM32 "Blue Pill" (STM32F103C6 / C8) | Cat 8 – Cat 10, Cat 14 | 8 MHz crystal, 72 MHz system clock; on-board LED on PC13 (active-low); programmed with an ST-Link over SWD |
| NUCLEO-F401RE (STM32F401RE) | Cat 11 – Cat 13 | 8 MHz from the on-board ST-LINK (HSE bypass), 84 MHz system clock; LD2 on PA5, B1 on PC13 |
| Wemos D1 Mini (ESP8266) | Cat 10, Cat 12 – Cat 14 | Wi-Fi modem running Espressif AT-command firmware, 115200 baud |
| Bosch BME280 module | Cat 12, Cat 13 | Temperature, pressure and humidity sensor on I²C (address 0x76) |
| DHT11 module, LED, buzzer | Cat 14 | Rack temperature/humidity sensor (single-wire) and local alarm |
| USB-to-TTL serial converter | Cat 6, Cat 9 – Cat 12 | PC terminal (PuTTY) |

## Tools

| Tool | Used for |
|---|---|
| **Keil µVision 5** with the C51 compiler | 8051 projects (`.uvproj`) |
| **STM32CubeMX** | STM32 pin, clock and peripheral configuration (`.ioc`) and code generation |
| **Keil MDK-ARM (µVision 5)** | STM32 projects (`MDK-ARM/*.uvprojx`) |
| **STM32CubeIDE** | STM32 projects (`.project`, `.cproject`), alternative to Keil MDK-ARM |
| STM32Cube HAL (FW_F1, FW_F4) | STM32 peripheral drivers (in each project's `Drivers/` folder) |
| Bosch Sensortec BME280 Sensor API v3.5.1 | BME280 driver (Cat 12, Cat 13) |
| Node-RED + Node-RED Dashboard 2.0 | MQTT dashboards and flows (Cat 10, Cat 12 – Cat 14) |
| OpenWeatherMap API | Outside weather in the Cat 14 dashboard |
| PuTTY | Serial terminal |

## Repository Layout

```
<platform>/CATn/
├── README.md                 experiment index: details, date, overview, circuit, task table
├── circuit.png               circuit diagram
└── TASKnX/                   one folder per task
    ├── README.md             task details: hardware, logic, pseudocode, code, inference
    ├── 8051:  *.c, *.uvproj, STARTUP.A51, Objects/*.hex
    └── STM32: *.ioc, Core/, Drivers/, MDK-ARM/*.uvprojx + startup_*.s, .project / .cproject
```

Build output is not committed (see [.gitignore](.gitignore)): Keil's `Listings/` and the rest of `Objects/`, STM32CubeIDE's `Debug/` and `Release/`, and per-user IDE files. The 8051 projects keep their `.hex` file so they can be flashed without rebuilding.

## Building

**8051 (Keil µVision):** open the experiment's `.uvproj` file and choose **Project → Build Target (F7)**. The HEX file is written to the project's `Objects/` folder. Flash it to the AT89S52 with an ISP programmer.

**STM32 (Keil MDK-ARM):** open the task's `MDK-ARM/<task>.uvprojx` in Keil µVision 5 and choose **Project → Build Target (F7)**, then **Flash → Download** through the ST-Link.

**STM32 (STM32CubeIDE):** choose **File → Import → General → Existing Projects into Workspace**, select the experiment's project folder, then **Project → Build Project**. Flash and debug with **Run → Debug** through the ST-Link. For the Wi-Fi experiments (Cat 10, Cat 12, Cat 13), set your own Wi-Fi network name and password in `main.c` first.
