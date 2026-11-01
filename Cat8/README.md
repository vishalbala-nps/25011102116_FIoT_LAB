# CAT 8 – STM32 GPIO Bring-Up and Switch-Controlled LED (Short Press vs Long Press)

## Aim

Set up the GPIO pins of an STM32F103 with STM32CubeMX and the HAL library. Read a push-button and control an LED, so that each debounced press toggles the LED. Then measure how long the button is held, without blocking delays: a short press toggles the LED, and a long press switches the LED into a continuous blinking mode.

## Hardware Setup

| Item | Details |
|---|---|
| Development board | STM32 "Blue Pill" board |
| Microcontroller | STMicroelectronics **STM32F103C6T6A** (ARM Cortex-M3, 32 KB Flash, 10 KB RAM, LQFP48) |
| Clock source | 8 MHz external crystal (HSE) on the board |
| System clock | 72 MHz (see [Clock Configuration](#clock-configuration)) |
| Supply | 3.3 V |
| Output | On-board user LED on **PC13** (active-low) |
| Input | External push-button on **PA15**, wired to GND |
| Programmer / debugger | ST-Link over SWD (PA13 = SWDIO, PA14 = SWCLK) |
| Software | STM32CubeIDE with STM32CubeMX 6.18.1, STM32Cube FW_F1 V1.8.7 (HAL) |

## Pin Configuration

| Pin | Function | CubeMX signal | Mode | Pull | Speed | Initial level | Notes |
|---|---|---|---|---|---|---|---|
| **PC13** | User LED | `GPIO_Output` | Output, push-pull (`GPIO_MODE_OUTPUT_PP`) | None (`GPIO_NOPULL`) | Low (`GPIO_SPEED_FREQ_LOW`, 2 MHz) | Low (`GPIO_PIN_RESET`) | On-board LED, active-low: `RESET` = LED ON, `SET` = LED OFF |
| **PA15** | Push-button | `GPIO_Input` | Input (`GPIO_MODE_INPUT`) | **Pull-up** (`GPIO_PULLUP`) | — | — | Reads `SET` (1) when released, `RESET` (0) when pressed |

## Clock Configuration

```
 HSE 8 MHz ──► PLL (source HSE, ÷1) ──► ×9 ──► SYSCLK 72 MHz
                                                  │
                                         AHB ÷1 ──┴──► HCLK 72 MHz ──► Cortex-M3 core, SysTick, memory
                                                  │
                                    ┌─────────────┴──────────────┐
                              APB1 ÷2                       APB2 ÷1
                         PCLK1 36 MHz                  PCLK2 72 MHz
                    (APB1 timers 72 MHz)          (GPIOA/C/D, AFIO, APB2 timers 72 MHz)
```

## Circuit Diagram

```
                STM32F103C6T6A (Blue Pill)
          +-----------------------------+
          |                             |            +3.3V
          |                             |              |
          |                             |             [R]
          |                             |              |
          |                             |             LED  (on-board, active-low)
          |                             |              |
          |  PC13  (output, push-pull)  |--------------+
          |                             |
          |  PA15  (input, pull-up)     |------o  o------ GND
          |                             |       push-button
          |                             |
          |  3V3                        |------ +3.3V
          +-----------------------------+
```

- **LED (on-board, active-low):** the LED and its resistor sit between +3.3 V and PC13. Driving PC13 low (`GPIO_PIN_RESET`) lets current flow through the LED and turns it on. Driving it high turns it off.
- **Button:** connects PA15 to GND when pressed. The internal pull-up holds PA15 at 1 while the button is released, so no external resistor is needed. Pressed = 0, released = 1.
- The 8 MHz crystal, the reset circuit and the ST-Link SWD connection are part of the board and debug setup and are not shown.

## Code Logic

The application is in [main.c](STMButtonLED/Core/Src/main.c), inside the CubeMX `USER CODE` sections, so regenerating code from the `.ioc` file keeps it. The pins and clocks are set up by the CubeMX-generated `SystemClock_Config()` and `MX_GPIO_Init()`.

### HAL Functions Used

| Function | Use in this program |
|---|---|
| `HAL_Init()` | Resets peripherals, sets up Flash and starts the 1 ms SysTick |
| `HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_15)` | Reads the button state |
| `HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13)` | Toggles the LED |
| `HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET)` | Sets the LED's initial level (in `MX_GPIO_Init()`) |
| `HAL_GetTick()` | Millisecond time base, used to time button presses and the blink rate |

### Press Classification

The button is checked on every pass of the main loop. The program never waits inside a delay. It only records time stamps:

1. **Press edge** (released → pressed, `SET` → `RESET`): store the start time `press_t = HAL_GetTick()`.
2. **Release edge** (pressed → released, `RESET` → `SET`): work out `duration = now − press_t` and classify the press:

| Press duration | Classified as | Action |
|---|---|---|
| < 30 ms | Switch bounce | Ignored |
| 30 ms – 999 ms | **Short press** | Not blinking: toggle the LED. Blinking: stop blinking. |
| ≥ 1000 ms | **Long press** | Toggle blinking mode: start blinking, or stop if already blinking |

While blinking mode is on, the loop toggles the LED every **200 ms** (a 400 ms period, 2.5 Hz), again by comparing `HAL_GetTick()` with the time of the last toggle (`blink_t`).

### System Response

| User action | System response |
|---|---|
| Short press | Toggle LED |
| Long press | LED enters continuous blinking mode |
| Next short press | Stop blinking |
| Long press again | Return to blinking mode |

### Debouncing

A mechanical button's contacts bounce for a few milliseconds, producing several very short press/release pulses. Any press shorter than 30 ms is treated as bounce and ignored. A bounce at the start of a press only resets `press_t` to the time of the last bounce. A bounce at release produces a pulse much shorter than 30 ms. Either way, one physical press is classified exactly once, so it produces exactly one toggle.

### Pseudocode

```
blink <- 0; prev <- RELEASED; press_t <- 0; blink_t <- 0

LOOP forever:
    cur <- read PA15
    now <- HAL_GetTick()                         // milliseconds since start

    IF prev == RELEASED AND cur == PRESSED:      // press edge
        press_t <- now

    IF prev == PRESSED AND cur == RELEASED:      // release edge
        duration <- now - press_t
        IF duration >= 1000:                     // long press
            blink <- NOT blink
        ELSE IF duration >= 30:                  // short press (not bounce)
            IF NOT blink: toggle LED
            ELSE:         blink <- 0             // stop blinking

    prev <- cur

    IF blink AND now - blink_t >= 200:           // non-blocking blink
        toggle LED
        blink_t <- now
```

### Event Flow

```
Button pressed → record start time → keep running the loop → button released
→ record release time → duration = release − start → classify (bounce / short / long) → act
```

## Task Files

| File | Purpose |
|---|---|
| [`STMButtonLED/Core/Src/main.c`](STMButtonLED/Core/Src/main.c) | Application code (button handling, press classification, LED control), clock setup and GPIO setup |
| [`STMButtonLED/Core/Src/stm32f1xx_hal_msp.c`](STMButtonLED/Core/Src/stm32f1xx_hal_msp.c) | MSP setup: AFIO clock and the JTAG-disable remap that frees PA15 |
| [`STMButtonLED/Core/Src/stm32f1xx_it.c`](STMButtonLED/Core/Src/stm32f1xx_it.c) | Interrupt handlers, including the SysTick handler that drives `HAL_GetTick()` |
| [`STMButtonLED/Core/Inc/`](STMButtonLED/Core/Inc/) | Project headers and the HAL configuration (`stm32f1xx_hal_conf.h`) |
| [`STMButtonLED/Core/Startup/startup_stm32f103c6tx.s`](STMButtonLED/Core/Startup/startup_stm32f103c6tx.s) | Startup code and vector table |
| [`STMButtonLED/STMButtonLED.ioc`](STMButtonLED/STMButtonLED.ioc) | STM32CubeMX configuration: pins, debug interface and clock tree |
| [`STMButtonLED/STM32F103C6TX_FLASH.ld`](STMButtonLED/STM32F103C6TX_FLASH.ld) | Linker script for the 32 KB Flash / 10 KB RAM device |
| [`STMButtonLED/Drivers/`](STMButtonLED/Drivers/) | STM32F1 HAL driver and CMSIS headers |

To build, open the `STMButtonLED` folder in STM32CubeIDE (**File → Import → Existing Projects into Workspace**) and choose **Project → Build Project**. The firmware is written to `Debug/STMButtonLED.elf`, and **Run → Debug** flashes it to the board through the ST-Link.

## Inference

1. **CubeMX turns a pin plan into setup code.** Choosing the pins, their modes, pull resistors and the clock tree in the `.ioc` file generates `MX_GPIO_Init()` and `SystemClock_Config()`. User code in the `USER CODE` sections survives every regeneration.
2. **The STM32 needs explicit pin and clock setup.** Unlike the 8051's quasi-bidirectional ports, each STM32 pin must be set as input or output, with its pull and output type. Its port clock must also be turned on before the pin can be used.
3. **Internal pull-ups simplify button wiring.** With `GPIO_PULLUP`, PA15 sits at a defined 1 when the button is open and reads 0 when the button shorts it to GND, so no external resistor is needed.
4. **Debug pins can be reused.** Switching the debug interface from full JTAG to Serial Wire frees PA15 (JTDI) for the application, while SWD on PA13/PA14 still allows programming and debugging.
5. **Time stamps replace delays.** Recording `HAL_GetTick()` at the press and release edges measures the press length while the loop keeps running, so the blinking and the button stay responsive. Unsigned subtraction (`now − press_t`) gives the correct duration even when the 32-bit tick counter wraps around.
6. **A minimum duration is a simple debounce.** Ignoring presses shorter than 30 ms removes contact bounce without any extra hardware or blocking wait, and ensures one physical press produces one action.
7. **The clock tree sets every speed in the chip.** The 8 MHz crystal is multiplied to 72 MHz by the PLL, divided for the APB1 bus (36 MHz maximum), and needs 2 Flash wait states at that speed. The 1 ms SysTick behind `HAL_GetTick()` is derived from this clock.
