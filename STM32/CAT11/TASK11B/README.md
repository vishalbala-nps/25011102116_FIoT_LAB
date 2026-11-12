# TASK 11B On-board Switch Control with State Remembrance

**Experiment:** [CONTINUOUS ASSESSMENT 11 – STM32F401RE Cortex-M4: GPIO and User Input Programming](../README.md)

---

### 1. Hardware Used

- **Development Board:** NUCLEO-F401RE
- **Microcontroller IC:** STM32F401RET6
- **GPIO Pins:** PA5 (LED output), PC13 (button input, pull-up)
- **Peripherals:** Onboard LED LD2 (active-high), onboard blue user button B1 (active-low)
- **Power Supply:** 5 V over the ST-LINK USB connector

Circuit diagram: see the [experiment README](../README.md#circuit-diagram).

### 2. Core Logic / Pseudocode / Code Base

**Working Principle:**

PC13 is configured as an input with pull-up, so B1 reads HIGH when released and LOW when pressed. The program detects the press and release edges of B1 and toggles LD2 on a valid press. Since the LED is only changed on a toggle, it keeps (remembers) its new state after the button is released. Presses shorter than 30 ms are ignored as switch bounce.

**Pseudocode:**

1. Configure PC13 as input with pull-up and PA5 as output.
2. On a press edge (HIGH → LOW), record the press time.
3. On a release edge (LOW → HIGH), calculate the press duration.
4. If the duration is at least 30 ms, toggle LD2; shorter pulses are ignored as bounce.
5. Keep the LED in its new state until the next valid press.
6. Repeat continuously.

**Code Base:**

- `HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13)` reads B1.
- `prev` stores the previous button state for edge detection and `press_t` the press time.
- `HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5)` toggles LD2.

### 3. Inference

Each valid press of B1 toggles LD2, and the LED stays in its new state after the button is released. Edge detection with a minimum press time gives one toggle per press.
