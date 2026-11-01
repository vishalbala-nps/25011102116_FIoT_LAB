# IoT Lab – Embedded Systems Experiments (8051 and STM32)

Lab experiments for **25CS2741 Fundamentals of Internet of Things + Lab** (B.Tech CSE (IoT), Semester 3, Shiv Nadar University Chennai).

The experiments start with bare-metal register programming on the 8051, move to STM32 microcontrollers with STM32CubeMX and the HAL library, and finish with networked sensing: an STM32 reads a BME280 sensor and serves or publishes the data over Wi-Fi through an ESP8266, using HTTP and MQTT.

Each experiment folder has its own **README.md** covering the experiment name, hardware setup, pin configuration, circuit diagram, code logic (pseudocode), task files and inference.

## Experiments

### 8051 (AT89S52) – Bare-Metal Embedded C, Keil µVision

| # | Experiment | Key concepts | Project |
|---|---|---|---|
| [Cat 2](Cat2/) | Bare-metal LED blink | SFR bit access (`sbit`), software delay, machine-cycle timing | [`day1task`](Cat2/day1task/) |
| [Cat 3](Cat3/) | Button-controlled four-mode LED pattern generator | GPIO input, debouncing, state machine | [`day2`](Cat3/day2/) |
| [Cat 4](Cat4/) | HMI with matrix keypad, LEDs and seven-segment display | Keypad scanning, seven-segment codes, mode flags | [`sevenseg`](Cat4/sevenseg/) |
| [Cat 5](Cat5/) | Hardware timer-based LED blinking | Timer 0 Mode 1, preload calculation, overflow flag | [`day4`](Cat5/day4/) |
| [Cat 6](Cat6/) | UART driver and serial debugging | SCON/TMOD, Timer 1 baud-rate generation, TI/RI flags, echo | [`day5`](Cat6/day5/) |
| [Cat 7](Cat7/) | 16×2 LCD driver and dynamic display control | HD44780 4-bit interface, LCD commands, DDRAM addressing | [`day6`](Cat7/day6/) |

### STM32 – STM32CubeMX + HAL, STM32CubeIDE

| # | Experiment | Board | Key concepts | Project |
|---|---|---|---|---|
| [Cat 8](Cat8/) | GPIO bring-up and switch-controlled LED (short vs long press) | Blue Pill (STM32F103C6) | GPIO input/output, pull-ups, `HAL_GetTick()` timing, debouncing | [`STMButtonLED`](Cat8/STMButtonLED/) |
| [Cat 9](Cat9/) | UART command-controlled LED | Blue Pill (STM32F103C6) | USART with HAL, echo, line-based command parsing, CR/LF | [`STMUartLED`](Cat9/STMUartLED/) |
| [Cat 10](Cat10/) | STM32–Wemos (ESP8266) Wi-Fi modem interface | Blue Pill (STM32F103C8) | Two UARTs, ESP8266 AT commands, joining Wi-Fi, IP/MAC lookup | [`WemosWifi`](Cat10/WemosWifi/) |
| [Cat 11](Cat11/) | Cortex-M4 GPIO and user input | NUCLEO-F401RE | F4 clock tree (HSE bypass, PLL), on-board LED/button, short/long press | [`Ex5`](Cat11/Ex5/) |
| [Cat 12](Cat12/) | BME280 over I²C with the Bosch API, served over HTTP | NUCLEO-F401RE | I²C, Bosch Sensor API, ESP8266 TCP server, HTTP responses, UART interrupts | [`BME280Nucleo`](Cat12/BME280Nucleo/) |
| [Cat 13](Cat13/) | MQTT publishing of BME280 data | NUCLEO-F401RE | MQTT CONNECT/PUBLISH packets, public broker, JSON payload | [`NucleoMQTT`](Cat13/NucleoMQTT/) |

## Hardware

| Hardware | Used in | Notes |
|---|---|---|
| AT89S52 development kit | Cat 2 – Cat 7 | 11.0952 MHz crystal; on-board LED (P1.0, active-low), matrix keypad, seven-segment display and 16×2 LCD; programmed over ISP |
| STM32 "Blue Pill" (STM32F103C6 / C8) | Cat 8 – Cat 10 | 8 MHz crystal, 72 MHz system clock; on-board LED on PC13 (active-low); programmed with an ST-Link over SWD |
| NUCLEO-F401RE (STM32F401RE) | Cat 11 – Cat 13 | 8 MHz from the on-board ST-LINK (HSE bypass), 84 MHz system clock; LD2 on PA5, B1 on PC13 |
| Wemos D1 Mini (ESP8266) | Cat 10, Cat 12, Cat 13 | Wi-Fi modem running Espressif AT-command firmware, 115200 baud |
| Bosch BME280 module | Cat 12, Cat 13 | Temperature, pressure and humidity sensor on I²C (address 0x76) |
| USB-to-TTL serial converter | Cat 6, Cat 9 – Cat 12 | PC terminal (PuTTY) |

## Tools

| Tool | Used for |
|---|---|
| **Keil µVision 5** with the C51 compiler | 8051 projects (`.uvproj`) |
| **STM32CubeIDE** with STM32CubeMX | STM32 projects (`.ioc`, `.project`, `.cproject`) |
| STM32Cube HAL (FW_F1, FW_F4) | STM32 peripheral drivers (in each project's `Drivers/` folder) |
| Bosch Sensortec BME280 Sensor API v3.5.1 | BME280 driver (Cat 12, Cat 13) |
| PuTTY | Serial terminal |

## Repository Layout

```
CatN/
├── README.md             experiment write-up
└── <project>/            Keil µVision or STM32CubeIDE project
    ├── *.c / Core/       source code
    ├── *.uvproj / *.ioc  project / CubeMX configuration
    └── Objects/*.hex     (8051 only) built HEX file, ready to flash
```

Build output is not committed (see [.gitignore](.gitignore)): Keil's `Listings/` and the rest of `Objects/`, STM32CubeIDE's `Debug/` and `Release/`, and per-user IDE files. The 8051 projects keep their `.hex` file so they can be flashed without rebuilding.

## Building

**8051 (Keil µVision):** open the experiment's `.uvproj` file and choose **Project → Build Target (F7)**. The HEX file is written to the project's `Objects/` folder. Flash it to the AT89S52 with an ISP programmer.

**STM32 (STM32CubeIDE):** choose **File → Import → General → Existing Projects into Workspace**, select the experiment's project folder, then **Project → Build Project**. Flash and debug with **Run → Debug** through the ST-Link. For the Wi-Fi experiments (Cat 10, Cat 12, Cat 13), set your own Wi-Fi network name and password in `main.c` first.
