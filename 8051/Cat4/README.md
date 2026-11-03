# CAT 4 – Human-Machine Interface using Matrix Keypad, LEDs and Seven-Segment Display on the 8051 (AT89S52)

## Aim

Build a simple embedded user interface in bare-metal Embedded C. A matrix keypad is the input, a seven-segment display shows the key that was pressed, and four LEDs show the operating mode selected by that key.

## Hardware Setup

| Item | Details |
|---|---|
| Development board | AT89S52 development kit |
| Microcontroller | Microchip (Atmel) AT89S52, 40-pin DIP |
| Clock | 11.0952 MHz (on-board crystal) |
| Power | 5 V DC |
| Programming | ISP (In-System Programming) over SPI |
| Input | Dev kit's built-in matrix keypad on **Port 2** |
| Display | Dev kit's built-in common-cathode seven-segment display on **Port 0** |
| Outputs | 4 external LEDs on **P1.1, P1.3, P1.5, P1.7** (active-high) |
| Host software | Keil µVision 5 with the C51 compiler (C51 V9.60) |
| Hardware debug | Digital multimeter |

### Pin Assignment

| Signal | Port pins | Direction | Active state |
|---|---|---|---|
| Keypad rows 1–4 | P2.7, P2.6, P2.5, P2.4 | Output (scan) | One row driven low at a time |
| Keypad columns 1–3 | P2.3, P2.2, P2.1 | Input (internal pull-up) | Low when a key in the driven row is pressed |
| Seven-segment a–g | P0.0 – P0.6 | Output | High (1 = segment on) |
| LED1 – LED4 | P1.1, P1.3, P1.5, P1.7 | Output | High (1 = ON) |

### Keypad Layout

|  | Col 1 (P2.3) | Col 2 (P2.2) | Col 3 (P2.1) |
|---|---|---|---|
| **Row 1 (P2.7)** | 1 | 2 | 3 |
| **Row 2 (P2.6)** | 4 | 5 | 6 |
| **Row 3 (P2.5)** | 7 | 8 | 9 |
| **Row 4 (P2.4)** | — | 0 | — |

## Circuit Diagram

```
                     AT89S52
              +---------------------+                 LED1-4
              |  2  P1.1            |--------------------|>|------+
              |  4  P1.3            |--------------------|>|------+
              |  6  P1.5            |--------------------|>|------+
              |  8  P1.7            |--------------------|>|------+
              |                     |                             |
              |                     |                            GND
              |                     |
              | 39  P0.0  (a)       |------+
              | 38  P0.1  (b)       |------+
              | 37  P0.2  (c)       |------+      +-----------+
              | 36  P0.3  (d)       |------+----->|  7-SEG    |
              | 35  P0.4  (e)       |------+      | (built-in)|
              | 34  P0.5  (f)       |------+      +-----------+
              | 33  P0.6  (g)       |------+
              |                     |
              | 28  P2.7  (row 1)   |------+
              | 27  P2.6  (row 2)   |------+
              | 26  P2.5  (row 3)   |------+      +-----------+
              | 25  P2.4  (row 4)   |------+----->|  4x4      |
              | 24  P2.3  (col 1)   |------+      |  KEYPAD   |
              | 23  P2.2  (col 2)   |------+      | (built-in)|
              | 22  P2.1  (col 3)   |------+      +-----------+
              |                     |
              | 40  VCC             |------ +5V
              +---------------------+
```

- **LEDs (external, active-high):** each port pin connects directly to an LED's anode, and the LED's cathode goes to GND. Writing `1` turns the LED on.
- **Seven-segment display (built-in, common-cathode):** segments a–g are driven from P0.0–P0.6. Writing `1` to a bit lights that segment.
- **Keypad (built-in):** the four rows are on P2.7–P2.4 and the columns on P2.3–P2.1.
- The crystal, reset circuit, the display's and keypad's on-board wiring are built into the dev kit and are not shown.

## Code Logic

The program, [sevensg.c](sevenseg/sevensg.c), uses only the `reg51.h` register definitions. The keypad lines and LEDs are mapped onto single port bits with `sbit`, and the display is driven by writing a whole byte to `P0`.

### Key Functions

| Key | Seven-segment display | LED operation |
|---|---|---|
| 1 | 1 | LED1 ON |
| 2 | 2 | LED2 ON |
| 3 | 3 | LED3 ON |
| 4 | 4 | LED4 ON |
| 5 | 5 | Running LED pattern: 1 → 2 → 3 → 4 → 1 … |
| 6 | 6 | Alternate LEDs: 1010 ↔ 0101 |
| 7 | 7 | Binary counter 0–15 (LED1 = least significant bit) |
| 8 | 8 | All LEDs blink |
| 9 | 9 | All LEDs OFF |
| 0 | 0 | Reset system: stop every pattern and turn all LEDs off |

### Keypad Scanning and Debouncing

`btn()` sets all columns high, which makes them inputs. It then drives one row low at a time and keeps the others high. If a column in that row reads `0`, the key where they cross is pressed. To debounce it, `btn()` waits about 10 ms with a software loop and reads the column again. It reports the key only if the column is still `0`.

### Seven-Segment Codes (common cathode, P0.0 = a … P0.6 = g)

| Digit | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 |
|---|---|---|---|---|---|---|---|---|---|---|
| P0 value | 0x3F | 0x06 | 0x5B | 0x4F | 0x66 | 0x6D | 0x7D | 0x07 | 0x7F | 0x6F |

### Pseudocode

```
DEFINE row1..row4 AS P2.7..P2.4, col1..col3 AS P2.3..P2.1
DEFINE led1..led4 AS P1.1, P1.3, P1.5, P1.7

FUNCTION btn():
    FOR each row r in 1..4:
        drive row r low, other rows high
        FOR each column c:
            IF col c == 0:
                busy-wait ~10 ms               // debounce
                IF col c == 0: RETURN key at (r, c)
    RETURN -1                                  // no key

MAIN:
    all LEDs off; display blank
    flags running, alternate, blink, binary <- 0
    LOOP forever:
        key <- btn()
        IF key is valid:
            displayNumber(key)
            clear all mode flags
            SWITCH key:
                1..4: glowLed(key)
                5:    running <- 1, start at LED1
                6:    alternate <- 1, show 1010
                7:    binary <- 1, count <- 0
                8:    blink <- 1, all LEDs on
                9:    all LEDs off
                0:    reset every counter, all LEDs off
        IF running:   move to next LED;           delay(100)
        IF alternate: toggle 1010 / 0101;         delay(100)
        IF blink:     toggle all on / all off;    delay(100)
        IF binary:    glowBinary(count);
                      count <- (count + 1) mod 16; delay(100)
```

### Timing

| Quantity | Value |
|---|---|
| Oscillator frequency | 11.0952 MHz |
| Machine cycle | 12 / 11.0952 MHz ≈ 1.0815 µs |
| Debounce delay in `btn()` | 1,400 loop passes ≈ 10 ms |
| Pattern step (`delay(100)`) | 100 × 100 = 10,000 loop passes ≈ 0.1 s |

## Task Files

| File | Purpose |
|---|---|
| [`sevenseg/sevensg.c`](sevenseg/sevensg.c) | Bare-metal C source: keypad scanning, display driver, LED patterns and mode selection |
| [`sevenseg/STARTUP.A51`](sevenseg/STARTUP.A51) | Standard C51 startup code: clears internal RAM, sets up the stack, then jumps to `main` |
| [`sevenseg/sevenseg.uvproj`](sevenseg/sevenseg.uvproj) | Keil µVision project file for the AT89S52 target |
| [`sevenseg/Objects/sevenseg.hex`](sevenseg/Objects/sevenseg.hex) | Built Intel HEX file, ready to flash onto the board |

To rebuild, open `sevenseg.uvproj` in Keil µVision and choose **Project → Build Target (F7)**. The HEX file is written to `sevenseg/Objects/`.

## Inference

1. **A matrix keypad saves port pins.** Scanning one row at a time and reading the columns lets 12 keys share 7 port pins, instead of needing one pin per key.
2. **The seven-segment display is a lookup table.** Each digit is a fixed byte of segment bits. Showing a number is one write of that byte to P0, so the display costs no CPU time once it has been written.
3. **Separating input, display and output keeps the program clear.** `btn()` only reads keys, `displayNumber()` only drives the display, and the `glow…()` functions only drive the LEDs. `main()` connects them, so each part can be changed without touching the others.
4. **Mode flags let one loop run several behaviours.** Each pattern key sets one flag and clears the others. The main loop advances only the active pattern, so a new key press immediately replaces the previous mode, and key 0 can stop everything by clearing all the flags.
5. **Software debouncing makes mechanical keys reliable.** Reading a key again after about 10 ms ignores contact bounce, so each press is read as a single key value.
