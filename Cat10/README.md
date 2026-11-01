# CAT 10 – STM32–Wemos (ESP8266) Wi-Fi Modem Interface Using AT Commands

## Aim

Connect an STM32F103 to a Wemos D1 Mini (ESP8266) over UART and drive it as a Wi-Fi modem with AT commands. The STM32 checks that the modem responds, sets it to Wi-Fi station mode, joins a Wi-Fi access point, and reads back the IP and MAC addresses. Its progress and results are reported to a PC terminal on a second UART.

## Hardware Setup

| Item | Details |
|---|---|
| Development board | STM32 "Blue Pill" board |
| Microcontroller | STMicroelectronics **STM32F103C8T6** (ARM Cortex-M3, 64 KB Flash, 20 KB RAM, LQFP48) |
| Clock source | 8 MHz external crystal (HSE) on the board |
| System clock | 72 MHz (see [Clock Configuration](#clock-configuration)) |
| Wi-Fi modem | Wemos D1 Mini (ESP8266) running Espressif AT-command firmware |
| PC link | USB-to-TTL (3.3 V) serial converter to PuTTY |
| PC terminal | PuTTY: Serial, **115200** baud, 8 data bits, no parity, 1 stop bit, no flow control |
| Programmer / debugger | ST-Link over SWD (PA13 = SWDIO, PA14 = SWCLK) |
| Software | STM32CubeIDE with STM32CubeMX 6.18.1, STM32Cube FW_F1 V1.8.7 (HAL) |

### UART Roles

| UART | STM32 pins | Connected to | Purpose |
|---|---|---|---|
| **USART1** | PA9 (TX), PA10 (RX) | Wemos D1 Mini | AT commands out, modem responses in |
| **USART2** | PA2 (TX), PA3 (RX) | PC (USB-to-TTL converter) | Status and results shown in PuTTY |

## Pin Configuration

| Pin | Function | CubeMX signal | Mode | Pull | Speed | Connects to |
|---|---|---|---|---|---|---|
| **PA9** | USART1 transmit | `USART1_TX` | Alternate function, push-pull (`GPIO_MODE_AF_PP`) | — | High (`GPIO_SPEED_FREQ_HIGH`) | Wemos **RX** |
| **PA10** | USART1 receive | `USART1_RX` | Input (`GPIO_MODE_INPUT`) | None (`GPIO_NOPULL`) | — | Wemos **TX** |
| **PA2** | USART2 transmit | `USART2_TX` | Alternate function, push-pull (`GPIO_MODE_AF_PP`) | — | High (`GPIO_SPEED_FREQ_HIGH`) | Converter **RX** |
| **PA3** | USART2 receive | `USART2_RX` | Input (`GPIO_MODE_INPUT`) | None (`GPIO_NOPULL`) | — | Converter **TX** |

### UART Configuration

| Parameter | USART1 (Wemos) | USART2 (PC) |
|---|---|---|
| Mode | Asynchronous, TX + RX | Asynchronous, TX + RX |
| Baud rate | **115200** | **115200** |
| Word length | 8 bits | 8 bits |
| Parity | None | None |
| Stop bits | 1 | 1 |

**Baud-rate dividers:** `USARTDIV = PCLK / (16 × baud)`.
- **USART1** is on APB2 (PCLK2 = 72 MHz): 72,000,000 / (16 × 115,200) = 39.0625, which the BRR register holds exactly (39 + 1/16). That gives **115,200 baud** with 0 % error.
- **USART2** is on APB1 (PCLK1 = 36 MHz): 36,000,000 / (16 × 115,200) ≈ 19.53. HAL writes 19 + 8/16 = 19.5, which gives ≈ **115,385 baud** (+0.16 %).

115200 baud is the ESP8266 AT firmware's default rate, so the two UARTs can use the same setting.

## Clock Configuration

```
 HSE 8 MHz ──► PLL (source HSE, ÷1) ──► ×9 ──► SYSCLK 72 MHz
                                                  │
                                         AHB ÷1 ──┴──► HCLK 72 MHz ──► Cortex-M3 core, SysTick, memory
                                                  │
                                    ┌─────────────┴──────────────┐
                              APB1 ÷2                       APB2 ÷1
                         PCLK1 36 MHz                  PCLK2 72 MHz
                        (USART2 → PC)            (USART1 → Wemos, GPIOA/D, AFIO)
```

## Circuit Diagram

```
                                                         Wemos D1 Mini
          STM32F103C8T6 (Blue Pill)                       (ESP8266)
     +-------------------------------+                 +-------------+
     |                               |                 |             |
     |  PA9   (USART1_TX, AF PP)     |---------------->| RX          |
     |  PA10  (USART1_RX, input)     |<----------------| TX          |
     |                               |                 |             |
     |  GND                          |-----------------| GND         |
     |                               |       +5V ----->| 5V          |
     |                               |                 +-------------+
     |                               |
     |                               |                 USB-to-TTL           PC
     |                               |                 +-----------+    +----------+
     |  PA2   (USART2_TX, AF PP)     |---------------->| RX        |    |  PuTTY   |
     |  PA3   (USART2_RX, input)     |<----------------| TX        |<==>|  115200  |
     |  GND                          |-----------------| GND       |USB |  8-N-1   |
     |                               |                 +-----------+    +----------+
     |  3V3                          |------ +3.3V
     +-------------------------------+
```

- **Crossed serial lines:** on each link, the STM32's TX goes to the other device's RX and the other device's TX goes to the STM32's RX.
- **Common ground:** the Wemos, the USB-to-TTL converter and the STM32 must share GND so their logic levels have the same reference.
- **Logic levels:** the ESP8266 and the STM32 both use 3.3 V logic, so their UART lines connect directly. The Wemos D1 Mini's 5V pin feeds its on-board 3.3 V regulator.
- The 8 MHz crystal, the reset circuit and the ST-Link SWD connection are part of the board and debug setup and are not shown.

## AT Command Investigation

The ESP8266 AT firmware takes text commands that end with CR LF (`\r\n`). It replies with lines of text, ending in `OK` on success or `ERROR` / `FAIL` on failure.

| Requirement | AT command | Typical response | Used in this program |
|---|---|---|---|
| Check modem response | `AT` | `OK` | ✅ |
| Turn command echo off | `ATE0` | `OK` | ✅ |
| Check firmware/version | `AT+GMR` | AT, SDK and firmware version lines, then `OK` | — |
| Set Wi-Fi station mode | `AT+CWMODE=1` (1 = station, 2 = AP, 3 = station + AP) | `OK` | ✅ |
| Scan available networks | `AT+CWLAP` | One `+CWLAP:(…)` line per network, then `OK` | — |
| Connect to an AP | `AT+CWJAP="<ssid>","<password>"` | `WIFI CONNECTED`, `WIFI GOT IP`, `OK` | ✅ |
| Get network information | `AT+CIFSR` | `+CIFSR:STAIP,"<ip>"`, `+CIFSR:STAMAC,"<mac>"`, `OK` | ✅ |
| Establish a TCP connection | `AT+CIPSTART="TCP","<host>",<port>` | `CONNECT`, `OK` | — |
| Send application data | `AT+CIPSEND=<length>`, then the data | `>` prompt, then `SEND OK` | — |

## Code Logic

The application is in [main.c](WemosWifi/Core/Src/main.c). CubeMX generates the peripheral setup (`SystemClock_Config()`, `MX_GPIO_Init()`, `MX_USART1_UART_Init()`, `MX_USART2_UART_Init()`). The Wi-Fi network name and password are passed to `joinAP()` in `main()`.

### Functions

| Function | Layer | Purpose |
|---|---|---|
| `sendUART(huart, text)` | UART | Sends a string followed by `\r\n` with `HAL_UART_Transmit()` |
| `recieveUART(huart, buf, size)` | UART | Reads characters with `HAL_UART_Receive()` until CR or LF ends a non-empty line, and stores the line (at most `size − 1` characters) |
| `sendATCommand(huart, cmd, output, size)` | AT | Sends one AT command, then collects every response line into `output` until it reads `OK` (returns 1) or `ERROR` / `FAIL` (returns 0) |
| `initWifi(huart)` | Wi-Fi | Waits 2 s for the ESP8266 to boot, clears any UART overrun, turns echo off with `ATE0`, and checks the modem with `AT` |
| `setMode(huart, mode, …)` | Wi-Fi | Maps `"STATION"`, `"AP"` or `"STATION_AP"` to `AT+CWMODE=1/2/3` and sends it |
| `joinAP(huart, ssid, password, …)` | Wi-Fi | Builds `AT+CWJAP="<ssid>","<password>"` and sends it |
| `getIP(huart, buf, size)` | Wi-Fi | Sends `AT+CIFSR` and copies the address after `STAIP,"` up to the closing quote |
| `getMAC(huart, buf, size)` | Wi-Fi | Sends `AT+CIFSR` and copies the address after `STAMAC,"` up to the closing quote |

### HAL Functions Used

| Function | Use in this program |
|---|---|
| `HAL_UART_Init()` | Sets up USART1 and USART2 |
| `HAL_UART_Transmit()` | Sends AT commands to the Wemos and status lines to the PC |
| `HAL_UART_Receive()` | Reads the Wemos's responses one character at a time |
| `__HAL_UART_CLEAR_OREFLAG()` | Clears a receive overrun caused by the ESP8266's boot messages |
| `HAL_Delay()` | Waits 2 s for the ESP8266 to finish booting |

### Program Flow

```
STM32
  ├── PC: "Initializing Wifi..."
  ├── ATE0                          → OK      (echo off)
  ├── AT                            → OK      (modem responds)
  ├── PC: "Initialized Wifi"
  ├── AT+CWMODE=1                   → OK      (station mode)
  ├── PC: "Connecting to Wifi..."
  ├── AT+CWJAP="<ssid>","<password>" → WIFI CONNECTED / WIFI GOT IP / OK
  ├── success:
  │     ├── PC: "Connected to Wifi Successfully"
  │     ├── AT+CIFSR → STAIP  → PC: "IP Address: <ip>"
  │     └── AT+CIFSR → STAMAC → PC: "MAC Address: <mac>"
  └── failure:
        └── PC: "Failed to connect to wifi"
```

### Pseudocode

```
FUNCTION sendATCommand(cmd):
    send cmd + "\r\n" on USART1
    output <- ""
    LOOP:
        line <- receive one line from USART1
        append line + "\r\n" to output
        IF line == "OK":                    RETURN success
        IF line == "ERROR" OR line == "FAIL": RETURN failure

MAIN:
    initialise HAL, clock, GPIO, USART1, USART2
    print "Initializing Wifi..."
    wait 2 s; clear UART overrun
    sendATCommand("ATE0"); sendATCommand("AT")
    print "Initialized Wifi"
    sendATCommand("AT+CWMODE=1")
    print "Connecting to Wifi..."
    IF sendATCommand("AT+CWJAP=\"<ssid>\",\"<password>\"") succeeds:
        print "Connected to Wifi Successfully"
        response <- sendATCommand("AT+CIFSR")       // getIP()
        print "IP Address: "  + text after STAIP,"  up to the next quote
        response <- sendATCommand("AT+CIFSR")       // getMAC()
        print "MAC Address: " + text after STAMAC," up to the next quote
    ELSE:
        print "Failed to connect to wifi"
```

### Expected PC Terminal Output

```
Initializing Wifi...
Initialized Wifi
Connecting to Wifi...
Connected to Wifi Successfully
IP Address: xxx.xxx.xxx.xxx
MAC Address: xx:xx:xx:xx:xx:xx
```

## Task Files

| File | Purpose |
|---|---|
| [`WemosWifi/Core/Src/main.c`](WemosWifi/Core/Src/main.c) | Application code: the UART helpers, the AT-command layer, the Wi-Fi functions, and clock/GPIO/USART setup |
| [`WemosWifi/Core/Src/stm32f1xx_hal_msp.c`](WemosWifi/Core/Src/stm32f1xx_hal_msp.c) | MSP setup: USART1/USART2 clocks and pin modes, JTAG disable (SWD only) |
| [`WemosWifi/Core/Src/stm32f1xx_it.c`](WemosWifi/Core/Src/stm32f1xx_it.c) | Interrupt handlers, including SysTick for the HAL time base |
| [`WemosWifi/Core/Inc/`](WemosWifi/Core/Inc/) | Project headers and the HAL configuration (`stm32f1xx_hal_conf.h`) |
| [`WemosWifi/Core/Startup/startup_stm32f103c8tx.s`](WemosWifi/Core/Startup/startup_stm32f103c8tx.s) | Startup code and vector table |
| [`WemosWifi/WemosWifi.ioc`](WemosWifi/WemosWifi.ioc) | STM32CubeMX configuration: pins, USART1, USART2, debug interface and clock tree |
| [`WemosWifi/STM32F103C8TX_FLASH.ld`](WemosWifi/STM32F103C8TX_FLASH.ld) | Linker script for the 64 KB Flash / 20 KB RAM device |
| [`WemosWifi/Drivers/`](WemosWifi/Drivers/) | STM32F1 HAL driver and CMSIS headers |

To build, open the `WemosWifi` folder in STM32CubeIDE (**File → Import → Existing Projects into Workspace**) and choose **Project → Build Project**. Set your own Wi-Fi network name and password in the `joinAP()` call in `main()` first. The firmware is written to `Debug/WemosWifi.elf`, and **Run → Debug** flashes it through the ST-Link. Open PuTTY on the converter's COM port at 115200-8-N-1 before resetting the board, so the whole sequence is visible.

## Inference

1. **The ESP8266 acts as a Wi-Fi modem.** With AT firmware, the ESP8266 handles all the Wi-Fi and TCP/IP work itself. The STM32 only needs a UART and plain-text commands to put it on a network. This splits the application processor from the network processor.
2. **Two UARTs separate control from monitoring.** USART1 carries only the AT conversation with the Wemos, and USART2 carries only human-readable status to the PC. Debug output never mixes with modem traffic.
3. **AT commands follow a request–response protocol.** Every command ends with CR LF, and every reply ends with a final status line (`OK`, `ERROR` or `FAIL`). Reading lines until one of these appears lets one function, `sendATCommand()`, handle any command.
4. **Each layer builds on the one below.** `sendUART()` / `recieveUART()` move bytes, `sendATCommand()` adds the protocol, and `setMode()`, `joinAP()`, `getIP()` and `getMAC()` turn AT commands into meaningful operations. `main()` reads as a list of Wi-Fi steps.
5. **The modem's start-up has to be allowed for.** The ESP8266 prints boot messages when powered and needs time to start. Waiting 2 s and clearing the UART overrun flag discards that noise, and `ATE0` stops the modem echoing commands, so only real responses are read.
6. **Responses are text that must be parsed.** `AT+CIFSR` returns the IP and MAC address inside quoted fields. Searching for the `STAIP` / `STAMAC` tags and copying up to the closing quote extracts each value safely, within the buffer size.
7. **Matching baud rates come from the bus clocks.** USART1 on the 72 MHz APB2 bus gives exactly 115200 baud, and USART2 on the 36 MHz APB1 bus gives 115,385 baud (+0.16 %). Both match the ESP8266's and PuTTY's 115200 setting.
