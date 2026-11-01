# CAT 9 – STM32 UART Command-Controlled LED

## Aim

Set up the STM32F103's USART with STM32CubeMX and the HAL library, communicate with a PC terminal (PuTTY), echo every character received, and build a text-command interface that turns the on-board LED on, off or toggles it.

## Hardware Setup

| Item | Details |
|---|---|
| Development board | STM32 "Blue Pill" board |
| Microcontroller | STMicroelectronics **STM32F103C6T6A** (ARM Cortex-M3, 32 KB Flash, 10 KB RAM, LQFP48) |
| Clock source | 8 MHz external crystal (HSE) on the board |
| System clock | 72 MHz (see [Clock Configuration](#clock-configuration)) |
| Supply | 3.3 V |
| Serial link | **USART2** on PA2 (TX) / PA3 (RX), to the PC through a USB-to-TTL (3.3 V) serial converter |
| PC terminal | PuTTY: Serial, **115200** baud, 8 data bits, no parity, 1 stop bit, no flow control |
| Output | On-board user LED on **PC13** (active-low) |
| Programmer / debugger | ST-Link over SWD (PA13 = SWDIO, PA14 = SWCLK) |
| Software | STM32CubeIDE with STM32CubeMX, STM32Cube FW_F1 (HAL) |

## Pin Configuration

| Pin | Function | CubeMX signal | Mode | Pull | Speed | Notes |
|---|---|---|---|---|---|---|
| **PA2** | UART transmit | `USART2_TX` | Alternate function, push-pull (`GPIO_MODE_AF_PP`) | — | High (`GPIO_SPEED_FREQ_HIGH`) | Driven by the USART2 peripheral; connects to the converter's RX |
| **PA3** | UART receive | `USART2_RX` | Input (`GPIO_MODE_INPUT`) | None (`GPIO_NOPULL`) | — | Read by the USART2 peripheral; connects to the converter's TX |
| **PC13** | User LED | `GPIO_Output` | Output, push-pull (`GPIO_MODE_OUTPUT_PP`) | None (`GPIO_NOPULL`) | Low (`GPIO_SPEED_FREQ_LOW`) | On-board LED, active-low: `RESET` = LED ON, `SET` = LED OFF |

### UART Configuration (USART2)

| Parameter | Value | HAL setting |
|---|---|---|
| Mode | Asynchronous, transmit and receive | `UART_MODE_TX_RX` |
| Baud rate | **115200** | `huart2.Init.BaudRate = 115200` |
| Word length | 8 bits | `UART_WORDLENGTH_8B` |
| Parity | None | `UART_PARITY_NONE` |
| Stop bits | 1 | `UART_STOPBITS_1` |

**Baud-rate divider:** `USARTDIV = PCLK1 / (16 × baud) = 36,000,000 / (16 × 115,200) ≈ 19.53`. HAL writes 19 + 8/16 = 19.5 into the BRR register, which gives 36 MHz / (16 × 19.5) ≈ **115,385 baud**. That's an error of +0.16 %, far inside what a UART tolerates.

## Clock Configuration

```
 HSE 8 MHz ──► PLL (source HSE, ÷1) ──► ×9 ──► SYSCLK 72 MHz
                                                  │
                                         AHB ÷1 ──┴──► HCLK 72 MHz ──► Cortex-M3 core, SysTick, memory
                                                  │
                                    ┌─────────────┴──────────────┐
                              APB1 ÷2                       APB2 ÷1
                         PCLK1 36 MHz                  PCLK2 72 MHz
                        (USART2 clock)                (GPIOA/C/D, AFIO)
```

## Circuit Diagram

```
                STM32F103C6T6A (Blue Pill)                 USB-to-TTL
          +-----------------------------+                  converter              PC
          |                             |                +-----------+       +----------+
          |  PA2  (USART2_TX, AF PP)    |--------------->| RX        |       |          |
          |                             |                |           |  USB  |  PuTTY   |
          |  PA3  (USART2_RX, input)    |<---------------| TX        |<=====>|  115200  |
          |                             |                |           |       |  8-N-1   |
          |                             |                +-----------+       +----------+
          |                             |            +3.3V
          |                             |              |
          |                             |             [R]
          |                             |              |
          |                             |             LED  (on-board, active-low)
          |                             |              |
          |  PC13  (output, push-pull)  |--------------+
          |                             |
          |  3V3                        |------ +3.3V
          +-----------------------------+
```

- **Crossed serial lines:** the STM32's TX (PA2) goes to the converter's RX, and the converter's TX goes to the STM32's RX (PA3).
- **Common ground:** the converter's GND must be connected to the board's GND.
- **3.3 V logic:** the converter must use 3.3 V TTL levels to match the STM32's I/O.
- **LED (on-board, active-low):** driving PC13 low turns the LED on, and driving it high turns it off.
- The 8 MHz crystal, the reset circuit and the ST-Link SWD connection are part of the board and debug setup and are not shown.

## Code Logic

The application is in [main.c](STMUartLED/Core/Src/main.c). CubeMX generates the peripheral setup (`SystemClock_Config()`, `MX_GPIO_Init()`, `MX_USART2_UART_Init()`).

### HAL Functions Used

| Function | Use in this program |
|---|---|
| `HAL_UART_Init()` | Sets up USART2 (called from `MX_USART2_UART_Init()`) |
| `HAL_UART_Receive(&huart2, &ch, 1, HAL_MAX_DELAY)` | Waits for and reads one character from the PC |
| `HAL_UART_Transmit(&huart2, &ch, 1, HAL_MAX_DELAY)` | Sends a character back to the PC (echo) |
| `HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, …)` | Turns the LED on (`RESET`) or off (`SET`) |
| `HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13)` | Toggles the LED |

### Commands

| Command typed in PuTTY (then Enter) | Action | PC13 |
|---|---|---|
| `LED ON` | LED turns on | `GPIO_PIN_RESET` (low) |
| `LED OFF` | LED turns off | `GPIO_PIN_SET` (high) |
| `LED TOGGLE` | LED changes state | toggled |

Commands are case-sensitive and must match exactly. Any other line is ignored.

### How a Command Is Processed

1. **Receive:** `HAL_UART_Receive()` waits for one character from PuTTY.
2. **Echo:** the character is sent straight back with `HAL_UART_Transmit()`, so the user sees what they type. PuTTY doesn't show typed characters by itself.
3. **Build the line:** ordinary characters are added to the buffer `buf` at position `lastInd`.
4. **End of line:** when a carriage return (`\r`) or line feed (`\n`) arrives, the buffer is ended with `'\0'` and compared with the known commands using `strcmp()`. The matching LED action runs, and `lastInd` is reset to 0 for the next command.
5. **CR + LF pairs:** if a terminal sends both `\r` and `\n`, the second one arrives when the buffer is empty (`lastInd == 0`) and is ignored, so each command is run only once.

### Pseudocode

```
LED <- OFF (PC13 high)
lastInd <- 0

LOOP forever:
    ch <- UART receive 1 character (wait until it arrives)
    UART transmit ch                      // echo
    IF ch == '\r' OR ch == '\n':          // end of a command
        IF lastInd > 0:
            buf[lastInd] <- '\0'
            IF buf == "LED ON":     PC13 <- LOW      // LED on
            ELSE IF buf == "LED OFF":    PC13 <- HIGH     // LED off
            ELSE IF buf == "LED TOGGLE": toggle PC13
            lastInd <- 0
    ELSE:
        buf[lastInd] <- ch; lastInd <- lastInd + 1
```

### Carriage Return (CR) and Line Feed (LF)

| Character | Code | Effect on the terminal |
|---|---|---|
| `\r` – Carriage Return | 0x0D | Moves the cursor back to the **start of the current line** |
| `\n` – Line Feed | 0x0A | Moves the cursor **down one line**, keeping its column |

Typewriter-style terminals such as PuTTY treat these as two separate movements:

| Test string | Result in PuTTY |
|---|---|
| `"HELLO\r\n"` | `HELLO`, then the cursor sits at the start of the next line |
| `"HELLO\r"` | The cursor returns to the start of the **same** line, so the next text overwrites `HELLO` |
| `"HELLO\n"` | The cursor moves down but stays in column 6, so the next text starts on the next line, indented under the end of `HELLO` |
| `"HELLO\r\nWORLD\r\n"` | `HELLO` and `WORLD` on two separate lines, each starting at column 1 |

**Why both CR and LF:** CR returns the cursor to column 1 and LF moves it to the next line. Only together do they start a fresh line. For incoming commands, the Enter key in PuTTY sends `\r`, which is why the program treats either character as the end of a command.

## Task Files

| File | Purpose |
|---|---|
| [`STMUartLED/Core/Src/main.c`](STMUartLED/Core/Src/main.c) | Application code (receive, echo, command parsing, LED control), clock, GPIO and USART2 setup |
| [`STMUartLED/Core/Src/stm32f1xx_hal_msp.c`](STMUartLED/Core/Src/stm32f1xx_hal_msp.c) | MSP setup: USART2 clock and the PA2/PA3 pin modes, JTAG disable (SWD only) |
| [`STMUartLED/Core/Src/stm32f1xx_it.c`](STMUartLED/Core/Src/stm32f1xx_it.c) | Interrupt handlers, including SysTick for the HAL time base |
| [`STMUartLED/Core/Inc/`](STMUartLED/Core/Inc/) | Project headers and the HAL configuration (`stm32f1xx_hal_conf.h`, UART module enabled) |
| [`STMUartLED/Core/Startup/startup_stm32f103c6tx.s`](STMUartLED/Core/Startup/startup_stm32f103c6tx.s) | Startup code and vector table |
| [`STMUartLED/STMUartLED.ioc`](STMUartLED/STMUartLED.ioc) | STM32CubeMX configuration: pins, USART2, debug interface and clock tree |
| [`STMUartLED/STM32F103C6TX_FLASH.ld`](STMUartLED/STM32F103C6TX_FLASH.ld) | Linker script for the 32 KB Flash / 10 KB RAM device |
| [`STMUartLED/Drivers/`](STMUartLED/Drivers/) | STM32F1 HAL driver and CMSIS headers |

To build, open the `STMUartLED` folder in STM32CubeIDE (**File → Import → Existing Projects into Workspace**) and choose **Project → Build Project**. The firmware is written to `Debug/STMUartLED.elf`, and **Run → Debug** flashes it through the ST-Link. Then open PuTTY on the converter's COM port at 115200-8-N-1.

## Inference

1. **CubeMX and HAL hide the register work.** Choosing USART2 in asynchronous mode generates the pin, clock and peripheral setup, and `HAL_UART_Transmit()` / `HAL_UART_Receive()` take care of the status flags that the 8051 driver handled by hand (TI/RI).
2. **The baud rate comes from the bus clock.** USART2 runs from PCLK1 (36 MHz), and the BRR divider of 19.5 gives 115,385 baud, a +0.16 % error. Both ends must agree on 115200-8-N-1 for the bytes to be read correctly.
3. **UART pins use alternate functions.** PA2 is handed to the USART as an alternate-function push-pull output, and PA3 is left as an input for the receiver. The GPIO block only routes the signals; the USART peripheral generates and samples them.
4. **Echo makes the link visible and testable.** Sending each received character back proves that reception and transmission both work, and shows the user what they are typing.
5. **A command is a line, not a character.** Characters are collected until CR or LF, then compared as a whole string. That lets readable commands like `LED TOGGLE` control the hardware, and new commands only need another `strcmp()` branch.
6. **CR and LF are separate movements.** CR returns to column 1 and LF moves down one line. A terminal needs both to start a clean new line, which is why output lines end with `\r\n`.
7. **Blocking receive is simple but waits.** `HAL_UART_Receive()` with `HAL_MAX_DELAY` stops the CPU until a character arrives. That suits a program that only reacts to commands. The interrupt-driven `HAL_UART_Receive_IT()` would let the CPU do other work while waiting.
