# CAT 11 – STM32F401RE Cortex-M4: GPIO and User Input Programming (NUCLEO-F401RE)

## Aim

Bring up the NUCLEO-F401RE board and program its GPIO with STM32CubeMX and the HAL library. The on-board user button controls the on-board LED: a short press toggles the LED, which keeps its state after the button is released, and a long press switches the LED into a fixed-interval blinking mode. Press length is measured with the HAL millisecond tick, without blocking delays.

## Hardware Setup

| Item | Details |
|---|---|
| Development board | **NUCLEO-F401RE** (STM32 Nucleo-64) |
| Microcontroller | STMicroelectronics **STM32F401RET6** (ARM Cortex-M4 with FPU, LQFP64) |
| Clock source | 8 MHz clock from the on-board ST-LINK (MCO), fed to the STM32 as an external clock (HSE bypass) |
| System clock | 84 MHz (see [Clock Configuration](#clock-configuration)) |
| Supply | 5 V over USB (ST-LINK connector); the MCU runs at 3.3 V |
| Output | On-board green user LED **LD2** on **PA5** (active-high) |
| Input | On-board blue user button **B1** on **PC13** (active-low) |
| Programmer / debugger | On-board ST-LINK/V2-1 over SWD |
| Software | STM32CubeIDE with STM32CubeMX 6.18.1, STM32Cube FW_F4 V1.28.3 (HAL) |

### STM32F401RE at a Glance

| Feature | STM32F401RE |
|---|---|
| Core | ARM Cortex-M4 with FPU and DSP instructions |
| Maximum clock | 84 MHz |
| Flash / SRAM | 512 KB / 96 KB |
| GPIO ports (LQFP64) | GPIOA, GPIOB, GPIOC (16 pins each), GPIOD (PD2), GPIOH (PH0/PH1): 50 I/Os |
| UART / USART | 3 (USART1, USART2, USART6); USART2 is wired to the ST-LINK virtual COM port |
| I²C | 3 |
| SPI | 4 (two can also be used as I²S) |
| Timers | 1 advanced-control (TIM1), 7 general-purpose (TIM2–TIM5, TIM9–TIM11; TIM2/TIM5 are 32-bit), 2 watchdogs, SysTick |
| ADC | 1 × 12-bit, up to 16 channels |
| Other | USB 2.0 OTG Full-Speed, SDIO, RTC, DMA |

## Pin Configuration

| Pin | Function | CubeMX signal | Mode | Pull | Speed | Initial level | Notes |
|---|---|---|---|---|---|---|---|
| **PA5** | User LED LD2 | `GPIO_Output` | Output, push-pull (`GPIO_MODE_OUTPUT_PP`) | None (`GPIO_NOPULL`) | Low (`GPIO_SPEED_FREQ_LOW`) | Low (`GPIO_PIN_RESET`) | Active-high: `SET` = LED ON, `RESET` = LED OFF |
| **PC13** | User button B1 | `GPIO_Input` | Input (`GPIO_MODE_INPUT`) | **Pull-up** (`GPIO_PULLUP`) | — | — | Reads `SET` (1) when released, `RESET` (0) when pressed |

## Clock Configuration

```
 ST-LINK MCO 8 MHz ──► HSE (bypass) ──► PLLM ÷4 ──► 2 MHz ──► PLLN ×84 ──► VCO 168 MHz ──► PLLP ÷2 ──► SYSCLK 84 MHz
                                                                                │
                                                                                └─► PLLQ ÷4 ──► 42 MHz (USB/SDIO domain, unused)
                                                  SYSCLK 84 MHz
                                                        │
                                               AHB ÷1 ──┴──► HCLK 84 MHz ──► Cortex-M4 core, SysTick, memory, GPIO
                                                        │
                                          ┌─────────────┴──────────────┐
                                    APB1 ÷2                       APB2 ÷1
                               PCLK1 42 MHz                  PCLK2 84 MHz
                          (APB1 timers 84 MHz)          (APB2 timers 84 MHz)
```

## Circuit Diagram

```
                 STM32F401RET6 (NUCLEO-F401RE)
          +-----------------------------+
          |                             |
          |  PA5   (output, push-pull)  |------[R]------|>|------ GND
          |                             |              LD2 (green, on-board, active-high)
          |                             |
          |                             |            +3.3V
          |                             |              |
          |                             |           [pull-up]
          |                             |              |
          |  PC13  (input, pull-up)     |--------------+------o  o------ GND
          |                             |                    B1 (blue, on-board)
          |  3V3                        |------ +3.3V
          +-----------------------------+
```

- **LED LD2 (on-board, active-high):** PA5 drives the LED through a resistor to GND. Writing `GPIO_PIN_SET` turns it on and `GPIO_PIN_RESET` turns it off. (On the Nucleo-64, PA5 is also Arduino pin D13.)
- **Button B1 (on-board, active-low):** pressing B1 connects PC13 to GND. The board's pull-up resistor, plus the internal pull-up enabled in CubeMX, holds PC13 at 1 while the button is released.
- Both parts are on the Nucleo board, so no external wiring is needed. The ST-LINK, its 8 MHz clock output and the reset button are part of the board and are not shown.

## Code Logic

The application is in [main.c](Ex5/Core/Src/main.c). CubeMX generates the clock and GPIO setup (`SystemClock_Config()`, `MX_GPIO_Init()`).

### HAL Functions Used

| Function | Use in this program |
|---|---|
| `HAL_Init()` | Resets peripherals, sets up Flash and starts the 1 ms SysTick |
| `HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13)` | Reads button B1 |
| `HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5)` | Toggles LED LD2 |
| `HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET)` | Sets the LED's initial level, OFF (in `MX_GPIO_Init()`) |
| `HAL_GetTick()` | Millisecond time base, used to time button presses and the blink interval |

### Press Classification

The button is checked on every pass of the main loop, and the program never waits inside a delay:

1. **Press edge** (released → pressed, `SET` → `RESET`): store the start time `press_t = HAL_GetTick()`.
2. **Release edge** (pressed → released, `RESET` → `SET`): work out `duration = now − press_t` and classify the press:

| Press duration | Classified as | Action |
|---|---|---|
| < 30 ms | Switch bounce | Ignored |
| 30 ms – 499 ms | **Short press** | Toggle LD2, if blinking mode is off. The LED keeps its new state after release. |
| ≥ 500 ms | **Long press** | Toggle blinking mode: start blinking, or stop if already blinking |

While blinking mode is on, the loop toggles LD2 every **200 ms** (a fixed 400 ms period, 2.5 Hz) by comparing `HAL_GetTick()` with the time of the last toggle (`blink_t`).

### How the PDF Tasks Map to the Program

| Task | In the program |
|---|---|
| Task 1 – on-board LED bring-up | PA5 set up as a push-pull output; LD2 blinks at a fixed 200 ms interval in blinking mode |
| Task 2 – button control with state remembrance | A short press of B1 toggles LD2, and the LED stays in that state after the button is released |
| Task 3 – short vs long press | The press duration is measured with `HAL_GetTick()`. A short press toggles the LED and a long press switches blinking mode on or off. |

### Pseudocode

```
blink <- 0; prev <- RELEASED; press_t <- 0; blink_t <- 0

LOOP forever:
    cur <- read PC13 (B1)
    now <- HAL_GetTick()                         // milliseconds since start

    IF prev == RELEASED AND cur == PRESSED:      // press edge
        press_t <- now

    IF prev == PRESSED AND cur == RELEASED:      // release edge
        duration <- now - press_t
        IF duration >= 500:                      // long press
            blink <- NOT blink
        ELSE IF duration >= 30:                  // short press (not bounce)
            IF NOT blink: toggle PA5 (LD2)

    prev <- cur

    IF blink AND now - blink_t >= 200:           // non-blocking fixed-interval blink
        toggle PA5 (LD2)
        blink_t <- now
```

## Task Files

| File | Purpose |
|---|---|
| [`Ex5/Core/Src/main.c`](Ex5/Core/Src/main.c) | Application code (button handling, press classification, LED control), clock setup and GPIO setup |
| [`Ex5/Core/Src/stm32f4xx_hal_msp.c`](Ex5/Core/Src/stm32f4xx_hal_msp.c) | MSP setup (system clock enables) |
| [`Ex5/Core/Src/stm32f4xx_it.c`](Ex5/Core/Src/stm32f4xx_it.c) | Interrupt handlers, including the SysTick handler that drives `HAL_GetTick()` |
| [`Ex5/Core/Inc/`](Ex5/Core/Inc/) | Project headers and the HAL configuration (`stm32f4xx_hal_conf.h`) |
| [`Ex5/Core/Startup/startup_stm32f401retx.s`](Ex5/Core/Startup/startup_stm32f401retx.s) | Startup code and vector table |
| [`Ex5/Ex5.ioc`](Ex5/Ex5.ioc) | STM32CubeMX configuration: pins, debug interface and clock tree |
| [`Ex5/STM32F401RETX_FLASH.ld`](Ex5/STM32F401RETX_FLASH.ld) | Linker script for running from the 512 KB Flash |
| [`Ex5/STM32F401RETX_RAM.ld`](Ex5/STM32F401RETX_RAM.ld) | Alternative linker script for running from RAM |
| [`Ex5/Drivers/`](Ex5/Drivers/) | STM32F4 HAL driver and CMSIS headers |

To build, open the `Ex5` folder in STM32CubeIDE (**File → Import → Existing Projects into Workspace**) and choose **Project → Build Project**. Connect the Nucleo with its USB cable, then use **Run → Debug** to flash it through the on-board ST-LINK.

## Inference

1. **The Nucleo gets its clock from the ST-LINK.** The board has no crystal fitted for the STM32 by default. Instead, the ST-LINK's 8 MHz clock output drives the OSC_IN pin, so CubeMX sets the HSE to **bypass** mode (external clock, not crystal).
2. **The F4 PLL has more stages than the F1.** The 8 MHz input is divided by PLLM to 2 MHz, multiplied by PLLN to a 168 MHz VCO, then divided by PLLP to the 84 MHz maximum system clock. APB1 runs at half that (42 MHz), and APB2 and the core run at 84 MHz with 2 Flash wait states.
3. **Board-level details decide the GPIO logic.** On the Nucleo, LD2 on PA5 is active-high and B1 on PC13 is active-low. On the Blue Pill, the PC13 LED is active-low. The same HAL calls give opposite visible results on different boards, so the schematic has to be checked first.
4. **The same application ports across STM32 families.** The press-classification logic from the Blue Pill (STM32F1) experiment runs unchanged on the Cortex-M4 STM32F401. Only the pins (PA5/PC13) and the CubeMX clock settings change, because the HAL keeps the same API across families.
5. **Time stamps replace delays.** Recording `HAL_GetTick()` at the press and release edges measures how long the button was held while the loop keeps running, so the blinking and the button stay responsive together.
6. **A minimum duration is a simple debounce.** Presses shorter than 30 ms are treated as contact bounce and ignored, so one physical press is counted once and the LED keeps the state it was toggled to.
