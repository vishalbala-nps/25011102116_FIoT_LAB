# TASK 3 Short-Press and Long-Press Detection

**Experiment:** [CONTINUOUS ASSESSMENT 11 – STM32F401RE Cortex-M4: GPIO and User Input Programming](../README.md)

---

### 1. Hardware Used

- **Development Board:** NUCLEO-F401RE
- **Microcontroller IC:** STM32F401RET6
- **GPIO Pins:** PA5 (LED output), PC13 (button input, pull-up)
- **Peripherals:** Onboard LED LD2, onboard user button B1
- **Power Supply:** 5 V over the ST-LINK USB connector

Circuit diagram: see the [experiment README](../README.md#circuit-diagram).

### 2. Core Logic / Pseudocode / Code Base

**Working Principle:**

The duration of each press is measured with `HAL_GetTick()` time stamps, without blocking delays, and the STM32 performs a different action for each type of press:

| Press duration | Type | Action |
|---|---|---|
| < 30 ms | Bounce | Ignored |
| 30 ms – 499 ms | Short press | Toggle LD2 (when not blinking) |
| ≥ 500 ms | Long press | Turn blinking mode ON / OFF |

**Pseudocode:**

1. On a press edge, record the start time.
2. On a release edge, calculate `duration = release − start`.
3. If the duration is 500 ms or more, toggle the blinking mode.
4. Else, if the duration is at least 30 ms and blinking mode is off, toggle LD2.
5. If blinking mode is on, toggle LD2 every 200 ms.
6. Repeat continuously.

**Code Base:**

- `blink` holds the blinking mode, `press_t` the press start time and `blink_t` the last blink toggle time.
- Unsigned subtraction `now - press_t` gives the correct duration even when the tick counter wraps around.

### 3. Inference

The STM32 distinguishes short and long presses of B1: a short press toggles LD2 and a long press starts or stops blinking. Measuring the press with time stamps keeps the loop running, so the LED blinks and the button is read at the same time.
