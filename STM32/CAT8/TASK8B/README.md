# TASK 8B Event-Based Control

**Experiment:** [CONTINUOUS ASSESSMENT 8 – STM32 GPIO Bring-Up and Switch-Controlled LED](../README.md)

---

### 1. Hardware Used

- **Development Board:** STM32 Blue Pill
- **Microcontroller IC:** STM32F103C6T6A (ARM Cortex-M3, 72 MHz from the 8 MHz HSE crystal)
- **GPIO Pins:** PC13 (output, push-pull), PA15 (input, internal pull-up)
- **Peripherals:** Onboard LED on PC13 (active-low), push-button between PA15 and GND
- **Power Supply:** 3.3 V from the board; push-button returns to GND

Circuit diagram: see the [experiment README](../README.md#circuit-diagram).

### 2. Core Logic / Pseudocode / Code Base

**Working Principle:**

PA15 is configured as a digital input with the internal pull-up enabled, so it reads HIGH when the button is released and LOW when it is pressed. PA15 is normally a JTAG pin, so the debug interface is set to Serial Wire only to free it. The program detects the press and release edges of the button and toggles the PC13 LED once per valid press. Switch bounce is handled in software: a press must last at least 30 ms before it is accepted, so the short pulses caused by contact bounce are ignored.

**Pseudocode:**

1. Initialize the HAL library and the system clock.
2. Configure PC13 as output and PA15 as input with pull-up.
3. Read the button state and the current time (`HAL_GetTick()`) on every loop.
4. On a press edge (HIGH → LOW), record the press time.
5. On a release edge (LOW → HIGH), calculate the press duration.
6. If the duration is at least 30 ms, toggle the LED; otherwise ignore it as bounce.
7. Store the current state as the previous state and repeat.

**Code Base:**

- `HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_15)` reads the button.
- `HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13)` toggles the LED.
- `HAL_GetTick()` provides the millisecond time stamps for the press and release edges.
- The variables `prev` (previous button state) and `press_t` (press time) are used to detect edges and measure the press.

### 3. Inference

Each press of the button toggles the onboard LED exactly once. Acting only on the release edge and ignoring presses shorter than 30 ms removes the effect of switch bounce, demonstrating GPIO input handling and software debouncing using STM32 HAL functions.
