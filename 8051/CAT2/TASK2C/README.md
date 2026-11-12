# TASK 2C Customed Library Function for Delay Functionality

**Experiment:** [CONTINUOUS ASSESSMENT 2 – 8051 Tool Chain Setup – Development Board – AT89S52](../README.md)

---

### 1. Hardware Used

- **Microcontroller:** AT89S52 (8051 core) on the AT89S52 development kit
- **GPIO Port:** Port 1
- **GPIO Pin:** P1.0
- **Peripheral:** Built-in user LED (active-low)
- **Power Supply:** 5V DC

Circuit diagram: see the [experiment README](../README.md#circuit-diagram).

### 2. Core Logic / Pseudocode / Code Base

**Working Principle:**

The delay is moved into a separate library, `delay.h` / `delay.c`, so any program can include it. `delay_ms(ms)` repeats a calibrated inner loop of about 1 ms: two 153-pass loops of 3 machine cycles each (about 925 cycles ≈ 1 ms at 11.0952 MHz). The LED blink program calls `delay_ms(500)` to blink at 1 Hz.

**Pseudocode:**

1. Declare `delay_ms(unsigned int ms)` in `delay.h`.
2. In `delay.c`, repeat `ms` times: run the two calibrated inner loops (≈ 1 ms).
3. In the main program, include `delay.h`.
4. Set P1.0 HIGH (LED OFF) and call `delay_ms(500)`.
5. Set P1.0 LOW (LED ON) and call `delay_ms(500)`.
6. Repeat continuously.

**Code Base:**

- `delay.h` declares `delay_ms()`; `delay.c` implements it; both are added to the Keil project.
- `prog1.c` blinks the LED with `delay_ms(500)`.

### 3. Inference

Moving the delay into its own library with a millisecond parameter makes the timing reusable and readable: `delay_ms(500)` states the intended time directly, instead of loop counts inside the application.
