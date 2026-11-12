# TASK 11A On-board LED Bring-Up

**Experiment:** [CONTINUOUS ASSESSMENT 11 – STM32F401RE Cortex-M4: GPIO and User Input Programming](../README.md)

---

### 1. Hardware Used

- **Development Board:** NUCLEO-F401RE
- **Microcontroller IC:** STM32F401RET6 (ARM Cortex-M4 with FPU, 84 MHz)
- **GPIO Pin:** PA5 (output, push-pull)
- **Peripheral:** Onboard green LED LD2 (active-high)
- **Power Supply:** 5 V over the ST-LINK USB connector

Circuit diagram: see the [experiment README](../README.md#circuit-diagram).

### 2. Core Logic / Pseudocode / Code Base

**Working Principle:**

The NUCLEO-F401RE has no crystal for the STM32; it receives an 8 MHz clock from the onboard ST-LINK, so CubeMX sets the HSE to bypass mode. The PLL (PLLM ÷4, PLLN ×84, PLLP ÷2) raises this to the 84 MHz maximum system clock. PA5 is configured as a push-pull output to drive LD2, and the LED is toggled every 500 ms using `HAL_Delay()`, so it blinks at a fixed 1 Hz.

**Pseudocode:**

1. Initialize the HAL library and configure the 84 MHz system clock.
2. Configure PA5 as output and turn LD2 OFF.
3. Toggle PA5.
4. Wait 500 ms.
5. Repeat continuously.

**Code Base:**

- `HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5)` toggles LD2.
- `HAL_Delay(500)` sets the fixed blink interval (based on the 1 ms SysTick).

### 3. Inference

LD2 blinks at a fixed interval, verifying the NUCLEO-F401RE hardware, the clock configuration and the GPIO output configuration.
