# EXPERIMENT DETAILS

**EXP:** CONTINUOUS ASSESSMENT 8

**DATE:** DD/MM/YYYY

**NAME OF EXP:** STM32 GPIO Bring-Up and Switch-Controlled LED

**HARDWARE SETUP / PLATFORM:** CubeMX + STM32CubeIDE + STM32 Development Board Blue Pill STM32F103C6T6A + Push-Button

---

## TASK 8C Short Press vs Long Press

### 1. Circuit Diagram

The same circuit as Task 8A is used.

- **Development Board:** STM32 Blue Pill
- **Microcontroller IC:** STM32F103C6T6A
- **GPIO Pins:** PC13 (LED output), PA15 (button input, pull-up)
- **Peripherals:** Onboard LED (active-low), push-button to GND
- **Power Supply:** 3.3 V from the board

![Circuit Diagram](../circuit.png)

### 2. Core Logic / Pseudocode / Code Base

**Working Principle:**

The press is classified by how long the button is held, measured without any blocking delay. The press time is recorded on the press edge and the duration is calculated on the release edge. A press shorter than 1 second is a short press and a press of 1 second or more is a long press. A long press switches the LED into continuous blinking mode, where the LED is toggled every 200 ms by comparing `HAL_GetTick()` with the time of the last toggle.

| User action | System response |
|---|---|
| Short press | Toggle LED |
| Long press | LED enters continuous blinking mode |
| Next short press | Stop blinking |
| Long press again | Return to blinking mode |

**Pseudocode:**

1. Initialize the HAL library, clock and GPIO pins.
2. On a press edge, record the start time.
3. Continue executing the application while the button is held.
4. On a release edge, record the release time and calculate `duration = release − start`.
5. If the duration is 1000 ms or more, toggle the blinking mode.
6. Else, if the duration is at least 30 ms:
   - if blinking mode is off, toggle the LED;
   - if blinking mode is on, stop blinking.
7. If blinking mode is on and 200 ms have passed since the last toggle, toggle the LED and record the time.
8. Repeat continuously.

**Code Base:**

- `HAL_GetTick()` is used for both press timing and the blink interval; `HAL_Delay()` is not used.
- `blink` holds the blinking mode, `press_t` the press start time and `blink_t` the last blink toggle time.
- `HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13)` is used for both the short-press toggle and the blinking.
- The application code is placed inside the CubeMX `USER CODE` sections of `Core/Src/main.c`.

### 3. Inference

A short press toggles the LED and a long press of 1 second or more starts or stops continuous blinking. Measuring the press duration with `HAL_GetTick()` time stamps instead of a blocking delay keeps the program responsive, so the LED keeps blinking while the button is being read.
