# EXPERIMENT DETAILS

**EXP:** CONTINUOUS ASSESSMENT 8

**DATE:** DD/MM/YYYY

**NAME OF EXP:** STM32 GPIO Bring-Up and Switch-Controlled LED

**HARDWARE SETUP / PLATFORM:** CubeMX + STM32CubeIDE + STM32 Development Board Blue Pill STM32F103C6T6A + Push-Button

---

## TASK 8A GPIO Input/Output

### 1. Circuit Diagram

The experiment uses the onboard LED of the STM32 Blue Pill and an external push-button.

- **Development Board:** STM32 Blue Pill
- **Microcontroller IC:** STM32F103C6T6A (ARM Cortex-M3, 72 MHz from the 8 MHz HSE crystal)
- **GPIO Pins:** PC13 (output, push-pull), PA15 (input, internal pull-up)
- **Peripherals:** Onboard LED on PC13 (active-low), push-button between PA15 and GND
- **Power Supply:** 3.3 V from the board; push-button returns to GND

![Circuit Diagram](../circuit.png)

### 2. Core Logic / Pseudocode / Code Base

**Working Principle:**

PC13 is configured as a push-pull output for the onboard LED and PA15 as an input with the internal pull-up enabled, so the button reads HIGH when released and LOW when pressed. PA15 is normally a JTAG pin, so the debug interface is set to Serial Wire only to free it. The program continuously copies the switch state to the LED, so the LED follows the switch.

**Pseudocode:**

1. Initialize the HAL library and the system clock.
2. Configure PC13 as output and PA15 as input with pull-up.
3. Read the button on PA15.
4. If the button is pressed (LOW), turn the LED ON (PC13 LOW).
5. Otherwise, turn the LED OFF (PC13 HIGH).
6. Repeat continuously.

**Code Base:**

- `HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_15)` reads the button.
- `HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET / GPIO_PIN_SET)` turns the LED ON / OFF (active-low).

### 3. Inference

The LED is ON while the button is held and OFF when it is released, demonstrating GPIO input and output configuration with the internal pull-up and active-low LED using STM32 HAL functions.
