# CAT 2 – Bare-Metal LED Blink on the 8051 (AT89S52)

## Aim

Compile a minimal bare-metal C program, program it onto the AT89S52, and blink an LED at about 1 Hz by writing directly to the microcontroller's port registers. 

## Hardware Setup

| Item | Details |
|---|---|
| Development board | AT89S52 development kit |
| Microcontroller | Microchip (Atmel) AT89S52, 40-pin DIP |
| Clock | 11.0952 MHz (on-board crystal) |
| Power | 5 V DC |
| Programming | ISP (In-System Programming) over SPI: P1.5 (MOSI), P1.6 (MISO), P1.7 (SCK), RST |
| Output | Built-in user LED on **P1.0** (active-low) |
| Host software | Keil µVision 5 with the C51 compiler (C51 V9.60) |

### Hardware Bring-Up Sheet

| Parameter | Student Entry |
|---|---|
| MCU | AT89S52 |
| CPU Core | 8051 (MCS-51), 8-bit |
| Operating Voltage | 5 V (4.0 V – 5.5 V) |
| Debug Interface | No on-chip debugger; programmed through ISP over SPI |
| LED GPIO Port | Port 1 |
| LED GPIO Pin | P1.0 (pin 1 -> Built in) |
| LED Active State | Active-low (the LED turns on when P1.0 = 0) |
| LED Frequency (Hz) | ≈ 1 Hz (0.5 s ON, 0.5 s OFF) |

### Bring-Up Steps

1. Identified the MCU, the programmer, the power input, the user LED and its GPIO pin on the board.
2. Checked the 5 V supply between VCC (pin 40) and GND (pin 20) with a digital multimeter.
3. Identified the built-in user LED's port and pin (P1.0) and its active-low wiring from the board schematic.
4. Connected the board to the host PC, built the project in Keil to produce `day1task.hex`, and flashed it to the AT89S52 over ISP.

## Circuit Diagram

```
                         +5V
                          |
                     +----+----+
                     |         |
                   [330R]      |
                     |         |
                   LED (anode up, cathode down, built in)
                     |         |
      AT89S52        |         |
   +-------------+   |         |
   |  1  P1.0    |---+         |
   |             |             |
   | 40  VCC     |-------------+
   +-------------+
```

- **LED (built-in, active-low):** the LED's anode connects to +5 V through a current-limiting resistor and its cathode to P1.0. Writing `0` to P1.0 pulls the pin low, current flows through the LED, and it turns on. Writing `1` turns it off.
- The crystal, reset circuit and other support wiring are built into the dev kit and are not shown.

## Code Logic

The program, [prog1.c](day1task/prog1.c), uses only the `reg51.h` register definitions. `sbit led = P1^0` maps the name `led` straight onto bit 0 of the Port 1 register (SFR address 0x90). Writing to `led` therefore changes the pin directly, with no library in between.

The 8051 has no pin-direction register. A port pin works as an output as soon as you write to it, so writing to the bit is the whole "configure as output" step.

### Pseudocode

```
DEFINE led AS bit 0 of Port 1 (P1.0)

FUNCTION delay():
    REPEAT 200 times:              // outer loop
        REPEAT 400 times:          // inner loop
            do nothing             // only uses up CPU cycles
    // about 0.5 s in total at 11.0952 MHz

MAIN:
    LOOP forever:
        led <- 1      // LED OFF (active-low)
        delay()
        led <- 0      // LED ON
        delay()
```

### Timing Calculation

An 8051 instruction cycle (machine cycle) lasts 12 oscillator periods.

| Quantity | Value |
|---|---|
| Oscillator frequency | 11.0952 MHz |
| Machine cycle | 12 / 11.0952 MHz ≈ **1.0815 µs** |
| Inner loop (400 passes) | ≈ 2,300 machine cycles, counted from the compiled assembly in `prog1.lst` |
| `delay()` = 200 × inner loop | ≈ 460,200 machine cycles ≈ **0.498 s** |
| One blink period = 2 × `delay()` | ≈ 0.996 s |
| **Blink frequency** | **≈ 1.0 Hz** |

## Task Files

| File | Purpose |
|---|---|
| [`day1task/prog1.c`](day1task/prog1.c) | Bare-metal C source: the LED blink and the software delay |
| [`day1task/STARTUP.A51`](day1task/STARTUP.A51) | Standard C51 startup code: clears internal RAM, sets up the stack, then jumps to `main` |
| [`day1task/day1task.uvproj`](day1task/day1task.uvproj) | Keil µVision project file for the AT89S52 target |
| [`day1task/Objects/day1task.hex`](day1task/Objects/day1task.hex) | Built Intel HEX file, ready to flash onto the board |

To rebuild, open `day1task.uvproj` in Keil µVision and choose **Project → Build Target (F7)**. The HEX file is written to `day1task/Objects/`.

## Inference

1. **Register access is all it takes.** Blinking an LED on the 8051 needs one SFR bit write. `reg51.h` only names the registers, so the compiled program is tiny: about 41 bytes of code for `delay` and `main` together.
2. **A software delay depends on the clock and the compiler.** One loop pass takes a fixed number of machine cycles, and each machine cycle takes 12 clock ticks. The delay therefore only comes out right once the code's loop counts are matched to the board's crystal. The first version used 400 × 400 passes and blinked at about 0.5 Hz. Working out the cycle count from the compiled assembly gave 200 × 400 passes for 1 Hz.
3. **A busy-wait delay blocks the CPU.** The processor can do nothing else while `delay()` runs, and changing the crystal or compiler settings changes the timing. In a real application, a hardware timer (Timer 0 or Timer 1) gives accurate timing without blocking.
4. **Port 1 drives the LED by sinking current.** The 8051's Port 1 pins have weak internal pull-ups, so the LED is wired active-low and turns on when the program writes `0`.
