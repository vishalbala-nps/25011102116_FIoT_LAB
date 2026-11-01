# CAT 5 – Hardware Timer-Based LED Blinking on the 8051 (AT89S52)

## Aim

Use the 8051's hardware Timer 0 as the time base for blinking an LED. The timer generates a fixed 4 ms tick, and a software counter of those ticks decides when to toggle the LED, without any software delay loops.

## Hardware Setup

| Item | Details |
|---|---|
| Development board | AT89S52 development kit |
| Microcontroller | Microchip (Atmel) AT89S52, 40-pin DIP |
| Clock | 11.0952 MHz (on-board crystal) |
| Power | 5 V DC |
| Programming | ISP (In-System Programming) over SPI |
| Output | External LED on **P1.1** (active-high) |
| Timer | Timer 0, Mode 1 (16-bit) |
| Host software | Keil µVision 5 with the C51 compiler (C51 V9.60) |
| Hardware debug | Multimeter, oscilloscope |

### Pin Assignment

| Signal | Port pin | DIP pin | Direction | Active state |
|---|---|---|---|---|
| LED | P1.1 | 2 | Output | High (1 = ON) |

## Circuit Diagram

```
                 AT89S52
          +------------------+                   LED
          |  2  P1.1         |--------------------|>|------+
          |                  |                             |
          |                  |                            GND
          |                  |
          | 40  VCC          |------ +5V
          +------------------+
```

- **LED (external, active-high):** P1.1 connects to the LED's anode, and the LED's cathode goes to GND. Writing `1` turns the LED on and writing `0` turns it off.
- **Timer 0** is inside the AT89S52 and needs no external wiring. It counts machine cycles derived from the crystal.
- The crystal and reset circuit are built into the dev kit and are not shown.

## Code Logic

The program, [timer.c](day4/timer.c), uses only the `reg51.h` register definitions. Timer 0 is set up and read directly through its registers.

### Timer Registers

| Register / bit | Value | Meaning |
|---|---|---|
| `TMOD` | `0x01` | Timer 0 in **Mode 1**: 16-bit timer, counting machine cycles (C/T = 0, GATE = 0) |
| `TH0`, `TL0` | `0xF1`, `0x99` | Start value (preload) **0xF199**, so the timer overflows after a fixed number of counts |
| `TR0` | `1` | Starts Timer 0 |
| `TF0` | set by hardware | Overflow flag: set to 1 when the timer rolls over from 0xFFFF to 0x0000. The program clears it. |

### Timing Calculation

| Quantity | Value |
|---|---|
| Oscillator frequency | 11.0952 MHz |
| Machine cycle (timer count period) | 12 / 11.0952 MHz ≈ **1.0815 µs** |
| Preload | 0xF199 = 61,849 |
| Counts until overflow | 65,536 − 61,849 = **3,687** |
| Tick (one overflow) | 3,687 × 1.0815 µs ≈ **3.99 ms ≈ 4 ms** |
| Ticks per LED toggle | 200 |
| LED toggle interval | 200 × 4 ms ≈ **0.8 s** |
| Blink period (ON + OFF) | ≈ 1.6 s |

The preload comes from the tick length wanted:

```
Counts  = Tick / Machine cycle  = 4 ms / 1.085 µs ≈ 3,687
Preload = 65,536 − 3,687        = 61,849 = 0xF199   →  TH0 = 0xF1, TL0 = 0x99
```

### Functions

| Function | Purpose |
|---|---|
| `void initTimer()` | Loads the preload 0xF199 into TH0/TL0 and clears the overflow flag TF0. This starts a new 4 ms tick. |
| `int main()` | Starts Timer 0 in Mode 1, then loops: counts each overflow, and toggles the LED every 200 overflows |

### Pseudocode

```
DEFINE led AS P1.1
count <- 0

FUNCTION initTimer():
    TH0 <- 0xF1; TL0 <- 0x99      // preload for a 4 ms tick
    TF0 <- 0                      // clear the overflow flag

MAIN:
    TR0 <- 1                      // start Timer 0
    TMOD <- 0x01                  // Timer 0, Mode 1 (16-bit)
    initTimer()
    led <- 1                      // LED ON
    LOOP forever:
        IF TF0 == 1:              // the hardware timer has overflowed (4 ms passed)
            count <- count + 1
            initTimer()           // reload for the next tick
        IF count == 200:          // 200 x 4 ms = 0.8 s
            led <- NOT led
            count <- 0
```

### Timing Flow

```
Crystal (11.0952 MHz)
   → ÷12 → machine cycle (1.0815 µs)
   → Timer 0 counts 3,687 cycles from 0xF199
   → overflow, TF0 = 1  (every ~4 ms)
   → software counter: 200 overflows
   → LED toggles (every ~0.8 s)
```

## Task Files

| File | Purpose |
|---|---|
| [`day4/timer.c`](day4/timer.c) | Bare-metal C source: Timer 0 setup and the timer-based LED toggle |
| [`day4/STARTUP.A51`](day4/STARTUP.A51) | Standard C51 startup code: clears internal RAM, sets up the stack, then jumps to `main` |
| [`day4/timers.uvproj`](day4/timers.uvproj) | Keil µVision project file for the AT89S52 target |
| [`day4/Objects/timers.hex`](day4/Objects/timers.hex) | Built Intel HEX file, ready to flash onto the board |

To rebuild, open `timers.uvproj` in Keil µVision and choose **Project → Build Target (F7)**. The HEX file is written to `day4/Objects/`. The last build reported 0 errors and 0 warnings, with a program size of 197 bytes of code.

## Inference

1. **The hardware generates the timing; the software responds to it.** Timer 0 counts machine cycles on its own, independently of the instructions the CPU is running. The program only reacts when the overflow flag TF0 says a tick has passed.
2. **The tick length depends only on the crystal and the preload.** Each count is one machine cycle (12 oscillator periods), so the preload 0xF199 always gives the same ~4 ms tick. Changing the program's code does not change the timing, unlike a software delay loop.
3. **A software counter turns a short tick into a long interval.** A 16-bit timer at this clock can measure at most 65,536 × 1.0815 µs ≈ 71 ms. Counting 200 overflows of a 4 ms tick extends this to 0.8 s.
4. **Mode 1 must be reloaded by software.** In Mode 1 the timer restarts from 0x0000 after an overflow, so the program writes the preload back into TH0/TL0 and clears TF0 after every tick. A few machine cycles pass between the overflow and the reload, which adds a small amount to each tick.
5. **The CPU is free between ticks.** The main loop never waits inside a delay. It only checks a flag, so other work could be added to the loop without changing the LED timing, as long as each pass of the loop takes less than one 4 ms tick.
