# CAT 3 – Button-Controlled Four-Mode LED Pattern Generator on the 8051 (AT89S52)

## Aim

Build a register-level GPIO controller in bare-metal Embedded C. Four LEDs are used as digital outputs and a push-button as a digital input. Each debounced button press moves the system to the next of four LED modes: running LED, binary counter, alternating pattern and all-LED flash.

## Hardware Setup

| Item | Details |
|---|---|
| Development board | AT89S52 development kit |
| Microcontroller | Microchip (Atmel) AT89S52, 40-pin DIP |
| Clock | 11.0952 MHz (on-board crystal) |
| Power | 5 V DC |
| Programming | ISP (In-System Programming) over SPI |
| Outputs | 4 external LEDs on **P1.1, P1.3, P1.5, P1.7** (active-high) |
| Input | One key of the dev kit's built-in matrix keypad: row **P2.0**, column **P2.5** |
| Host software | Keil µVision 5 with the C51 compiler (C51 V9.60) |
| Hardware debug | Digital multimeter |

### Pin Assignment

| Signal | Port pin | DIP pin | Direction | Active state |
|---|---|---|---|---|
| LED1 | P1.1 | 2 | Output | High (1 = ON) |
| LED2 | P1.3 | 4 | Output | High (1 = ON) |
| LED3 | P1.5 | 6 | Output | High (1 = ON) |
| LED4 | P1.7 | 8 | Output | High (1 = ON) |
| Keypad row | P2.0 | 21 | Input (internal pull-up) | Low when the key is pressed |
| Keypad column | P2.5 | 26 | Output, held at 0 | — |

## Circuit Diagram

```
                 AT89S52
          +------------------+                   LED1-4
          |  2  P1.1         |--------------------|>|------+
          |  4  P1.3         |--------------------|>|------+
          |  6  P1.5         |--------------------|>|------+
          |  8  P1.7         |--------------------|>|------+
          |                  |                             |
          |                  |                            GND
          |                  |
          | 21  P2.0  (row)  |-----o
          |                  |      /    key on the built-in keypad
          |                  |     o
          |                  |     |
          | 26  P2.5  (col)  |-----+
          |                  |
          | 40  VCC          |------ +5V
          +------------------+
```

- **LEDs (external, active-high):** each port pin connects directly to an LED's anode, and the LED's cathode goes to GND. Writing `1` to the pin turns the LED on and writing `0` turns it off.
- **Button (built-in keypad key):** the key connects keypad row P2.0 to column P2.5. The program holds the column at `0` and reads the row. P2.0 reads `1` through its internal pull-up while the key is released, and `0` while it is pressed.
- The crystal, reset circuit and keypad matrix are built into the dev kit and are not shown.

## Code Logic

The program, [day2.c](day2/day2.c), uses only the `reg51.h` register definitions. Each LED and the two keypad lines are mapped onto single port bits with `sbit`, so every read and write goes straight to the Port 1 and Port 2 registers.

The system is a **state machine** held in one variable, `mode`:

| `mode` | Behaviour | LED sequence (LED1 LED2 LED3 LED4) |
|---|---|---|
| 0 | Idle at power-on: all LEDs off | 0000 |
| 1 | Running LED | 1000 → 0100 → 0010 → 0001 → 1000 … |
| 2 | Binary counter, 0 to 15 (LED1 = least significant bit) | count 0000 → 0001 → … → 1111 → 0000 … |
| 3 | Alternating pattern | 1010 → 0101 → 1010 … |
| 4 | All-LED flash | 1111 → 0000 → 1111 … |

Each button press moves `mode` one step: 0 → 1 → 2 → 3 → 4 → 1 → … On every change, the step counter of each mode (`pos`, `count`, `alt`, `flash`) is reset, so each mode starts from its first pattern.

**Debouncing:** when `btn()` sees the row line at `0`, it waits about 10 ms with a software loop and reads the line again. It reports a press only if the line is still `0`, which ignores the short bounces of the key contacts.

### Pseudocode

```
DEFINE led1..led4 AS P1.1, P1.3, P1.5, P1.7
DEFINE row AS P2.0, col AS P2.5

FUNCTION btn():
    IF row == 0:
        busy-wait ~10 ms             // debounce
        IF row == 0: RETURN pressed
    RETURN not pressed

FUNCTION wait(n):
    REPEAT n times: REPEAT 120 times: do nothing   // ~1 ms per count

MAIN:
    col <- 0, row <- 1                // drive column low, row as input
    all LEDs <- 0; mode <- 0
    LOOP forever:
        IF btn():
            mode <- (mode < 4) ? mode + 1 : 1
            reset pos, count, alt, flash
        SWITCH mode:
            0: all LEDs off;                            wait(20)
            1: light only LED[pos]; pos <- next (1..4); wait(300)
            2: LEDs <- bits of count; count <- (count+1) mod 16; wait(300)
            3: show 1010 or 0101 by alt; toggle alt;    wait(300)
            4: show 1111 or 0000 by flash; toggle flash; wait(300)
```

### Timing

| Quantity | Value |
|---|---|
| Oscillator frequency | 11.0952 MHz |
| Machine cycle | 12 / 11.0952 MHz ≈ 1.0815 µs |
| Debounce delay in `btn()` | 1,400 loop passes ≈ 10 ms |
| Pattern step in modes 1–4 | `wait(300)` ≈ 0.3 s per step |
| Button check while idle (mode 0) | every `wait(20)` ≈ 20 ms |

## Task Files

| File | Purpose |
|---|---|
| [`day2/day2.c`](day2/day2.c) | Bare-metal C source: keypad reading with debounce, the four-mode state machine and the software delays |
| [`day2/STARTUP.A51`](day2/STARTUP.A51) | Standard C51 startup code: clears internal RAM, sets up the stack, then jumps to `main` |
| [`day2/day2.uvproj`](day2/day2.uvproj) | Keil µVision project file for the AT89S52 target |
| [`day2/Objects/day2.hex`](day2/Objects/day2.hex) | Built Intel HEX file, ready to flash onto the board |

To rebuild, open `day2.uvproj` in Keil µVision and choose **Project → Build Target (F7)**. The HEX file is written to `day2/Objects/`. The last build reported 0 errors and 0 warnings, with a program size of 337 bytes of code and 15 bytes of data.

## Inference

1. **A single state variable organises the whole system.** Representing the four modes as values of `mode` turns "what should the LEDs do now" into one variable check. Advancing the state on each press, and resetting the per-mode counters at that point, gives the predictable sequence Mode 1 → 2 → 3 → 4 → 1.
2. **A matrix keypad key can be read with two port pins.** Driving the key's column low and reading its row, with the 8051's internal pull-up on the row, gives a clean active-low input without any extra circuitry.
3. **Mechanical switches need debouncing.** Key contacts bounce for a few milliseconds when pressed. Reading the input a second time after about 10 ms filters these bounces out, so they are not counted as extra presses.
4. **Bit-addressable ports make per-LED control simple.** `sbit` lets each LED be set on its own. The binary counter is just the four lowest bits of `count`, each sent to one LED (`count % 2`, `(count / 2) % 2`, and so on).
5. **Active-high wiring maps patterns directly to code.** Because writing `1` turns an LED on, the bit patterns in the mode table (1010, 1111, …) are exactly the values the program writes to the pins.
