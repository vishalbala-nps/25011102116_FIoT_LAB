# IoT Lab – Embedded Systems Experiments (8051 and STM32)

Lab experiments for **25CS2741 Fundamentals of Internet of Things + Lab** (B.Tech CSE (IoT), Semester 3, Shiv Nadar University Chennai).

The experiments start with bare-metal register programming on the 8051, move to STM32 microcontrollers with STM32CubeMX and the HAL library, and finish with networked sensing: an STM32 reads a BME280 sensor and serves or publishes the data over Wi-Fi through an ESP8266, using HTTP and MQTT.

Each experiment folder (`CATn`) has a **README.md** index with the experiment details, date, overview, circuit diagram and a table of its tasks. Each task folder (`TASKnX`) contains the project and its own **README.md** with the hardware used, core logic, pseudocode, code base and inference.

## Experiments

### 8051 (AT89S52) – Bare-Metal Embedded C, Keil µVision

| # | Experiment | Key concepts | Project |
|---|---|---|---|
| [Cat 2](8051/CAT2/) | Bare-metal LED blink and custom delay library | SFR bit access (`sbit`), software delay, `delay_ms()` library | [`TASK_2`](8051/CAT2/TASK_2_Bare_Metal_LED_Blink/), [`TASK_3`](8051/CAT2/TASK_3_Custom_Delay_Library/) |
| [Cat 3](8051/CAT3/) | Button-controlled four-mode LED pattern generator | GPIO input, debouncing, state machine | [`CAT3`](8051/CAT3/) |
| [Cat 4](8051/CAT4/) | HMI with matrix keypad, LEDs and seven-segment display | Keypad scanning, seven-segment codes, mode flags | [`CAT4`](8051/CAT4/) |
| [Cat 5](8051/CAT5/) | Hardware timer-based LED blinking | Timer 0 Mode 1, preload calculation, overflow flag | [`CAT5`](8051/CAT5/) |
| [Cat 6](8051/CAT6/) | UART driver and serial debugging | SCON/TMOD, Timer 1 baud-rate generation, TI/RI flags, echo | [`TASK_A`](8051/CAT6/TASK_A_UART_Initialization_and_Character_Transmission/), [`TASK_B`](8051/CAT6/TASK_B_UART_Echo/) |
| [Cat 7](8051/CAT7/) | 16×2 LCD driver, dynamic display and keypad control | HD44780 4-bit interface, DDRAM addressing, keypad scanning | [`TASK_7A`](8051/CAT7/TASK_7A_LCD_Initialization_and_Character_String_Display/), [`TASK_7B`](8051/CAT7/TASK_7B_Cursor_Positioning_and_Dynamic_Value_Update/), [`TASK_7C`](8051/CAT7/TASK_7C_Keypad_Based_Character_Movement/) |

### STM32 – STM32CubeMX + HAL, STM32CubeIDE

| # | Experiment | Board | Key concepts | Project |
|---|---|---|---|---|
| [Cat 8](STM32/CAT8/) | GPIO bring-up and switch-controlled LED (short vs long press) | Blue Pill (STM32F103C6) | GPIO input/output, pull-ups, `HAL_GetTick()` timing, debouncing | [`STMButtonLED`](STM32/CAT8/) |
| [Cat 9](STM32/CAT9/) | UART command-controlled LED | Blue Pill (STM32F103C6) | USART with HAL, echo, line-based command parsing, CR/LF | [`STMUartLED`](STM32/CAT9/) |
| [Cat 10](STM32/CAT10/) | STM32–Wemos (ESP8266) Wi-Fi modem interface | Blue Pill (STM32F103C8) | Two UARTs, ESP8266 AT commands, joining Wi-Fi, IP/MAC lookup | [`WemosWifi`](STM32/CAT10/) |
| [Cat 10C](STM32/CAT10C/) | HTTP server – STM32–Wemos Wi-Fi modem using AT commands | Blue Pill (STM32F103C8) | ESP8266 TCP server, `+IPD` requests, static HTTP response, Node-RED client | [`TASK10C`, `TASK10D`, `TASK10E`](STM32/CAT10C/) |
| [Cat 11](STM32/CAT11/) | Cortex-M4 GPIO and user input | NUCLEO-F401RE | F4 clock tree (HSE bypass, PLL), on-board LED/button, short/long press | [`Ex5`](STM32/CAT11/) |
| [Cat 12](STM32/CAT12/) | BME280 over I²C with the Bosch API, served over HTTP | NUCLEO-F401RE | I²C, Bosch Sensor API, ESP8266 TCP server, HTTP responses, UART interrupts | [`BME280Nucleo`](STM32/CAT12/) |
| [Cat 13](STM32/CAT13/) | MQTT publishing of BME280 data | NUCLEO-F401RE | MQTT CONNECT/PUBLISH packets, public broker, JSON payload | [`NucleoMQTT`](STM32/CAT13/) |

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
