# CAT 6 – UART Driver and Serial Debugging on the 8051 (AT89S52)

## Aim

Write a UART driver for the 8051 and use it to talk to a PC terminal (PuTTY) through a USB-to-TTL serial interface at 9600-8-N-1. The 8051 sends a greeting to the terminal and then echoes back every character typed in PuTTY.

## Hardware Setup

| Item | Details |
|---|---|
| Development board | AT89S52 development kit |
| Microcontroller | Microchip (Atmel) AT89S52, 40-pin DIP |
| Clock | 11.0952 MHz (on-board crystal) |
| Power | 5 V DC |
| Programming | ISP (In-System Programming) over SPI |
| Serial link | USB-to-TTL serial converter between the 8051's UART pins and the PC |
| PC terminal | PuTTY: Serial, 9600 baud, 8 data bits, no parity, 1 stop bit, no flow control |
| Host software | Keil µVision 5 with the C51 compiler (C51 V9.60) |
| Hardware debug | Digital multimeter |

### Pin Assignment

| Signal | Port pin | DIP pin | Direction | Connects to |
|---|---|---|---|---|
| RXD (serial receive) | P3.0 | 10 | Input | TX of the USB-to-TTL converter |
| TXD (serial transmit) | P3.1 | 11 | Output | RX of the USB-to-TTL converter |

P3.0 and P3.1 are not just GPIO pins. Their alternate function is the 8051's built-in serial port, which takes them over once the UART is configured.

## Circuit Diagram

```
                 AT89S52                        USB-to-TTL
          +------------------+                  converter              PC
          |                  |                +-----------+       +----------+
          | 10  P3.0 (RXD)   |<---------------| TX        |       |          |
          |                  |                |           |  USB  |  PuTTY   |
          | 11  P3.1 (TXD)   |--------------->| RX        |<=====>|  9600    |
          |                  |                |           |       |  8-N-1   |
          | 40  VCC          |------ +5V      +-----------+       +----------+
          +------------------+
```

- **Crossed lines:** the 8051's TXD goes to the converter's RX and the converter's TX goes to the 8051's RXD, so each side's transmitter feeds the other side's receiver.
- **Common ground:** the converter's GND must be connected to the board's GND so both sides share the same 0 V reference.
- **TTL levels:** the converter works at the same 5 V logic levels as the AT89S52 and turns the serial data into a USB virtual COM port on the PC.
- The crystal and reset circuit are built into the dev kit and are not shown.

## Code Logic

The program, [prog1.c](day5/prog1.c), uses only the `reg52.h` register definitions. Every step of the driver is a direct read or write of the serial-port and timer registers.

### Register Setup (`initUART`)

| Register | Value | Meaning |
|---|---|---|
| `SCON` | `0x50` | SM0 = 0, SM1 = 1 → **Mode 1**: 8-bit UART with a variable baud rate. REN = 1 → receiving enabled. |
| `TMOD` | `0x20` | Timer 1 in **Mode 2**: 8-bit auto-reload, used as the baud-rate generator |
| `TH1`, `TL1` | `0xFD` | Reload value for 9600 baud (see below) |
| `TR1` | `1` | Starts Timer 1 |

### Baud-Rate Calculation

In Mode 1 the baud rate comes from Timer 1 overflows:

```
Baud = (2^SMOD / 32) × f_osc / (12 × (256 − TH1))
```

With SMOD = 0 and TH1 = 0xFD (256 − 3):

| Crystal | Baud rate | Error from 9600 |
|---|---|---|
| 11.0592 MHz (standard UART crystal) | 11,059,200 / (32 × 12 × 3) = **9,600** | 0 % |
| 11.0952 MHz (this board) | 11,095,200 / (32 × 12 × 3) ≈ **9,631** | +0.33 % |

A UART can tolerate a few percent of mismatch, so 0.33 % communicates reliably with PuTTY set to 9600.

### Functions

| Function | Purpose |
|---|---|
| `void initUART()` | Sets up the serial port (Mode 1, receive enabled) and Timer 1 as the 9600-baud generator |
| `void sendChar(char c)` | Writes `c` to `SBUF`, waits until the transmit-done flag `TI` is set, then clears `TI` |
| `char getChar()` | Waits until the receive flag `RI` is set, reads the character from `SBUF`, clears `RI` and returns it |
| `void sendString(char* st)` | Sends each character of a string, followed by `\r\n` (carriage return and new line) |
| `int main()` | Sends `Hello from 8051!!!` once, then echoes every received character back |

### Transmit and Receive Sequence

```
Transmit:  write SBUF  →  hardware shifts the bits out on TXD  →  TI = 1  →  software clears TI
Receive:   bits arrive on RXD  →  hardware fills SBUF  →  RI = 1  →  software reads SBUF, clears RI
```

`SBUF` is really two registers at the same address: writing to it loads the transmit buffer, and reading from it returns the receive buffer.

### Pseudocode

```
FUNCTION initUART():
    SCON <- 0x50          // Mode 1, receive enabled
    TMOD <- 0x20          // Timer 1, Mode 2 (auto-reload)
    TH1, TL1 <- 0xFD      // 9600 baud
    start Timer 1

FUNCTION sendChar(c):
    SBUF <- c
    WAIT UNTIL TI == 1
    TI <- 0

FUNCTION getChar():
    WAIT UNTIL RI == 1
    c <- SBUF
    RI <- 0
    RETURN c

FUNCTION sendString(s):
    FOR each character ch in s: sendChar(ch)
    sendChar('\r'); sendChar('\n')

MAIN:
    initUART()
    sendString("Hello from 8051!!!")
    LOOP forever:
        sendChar(getChar())   // echo
```

### Expected Terminal Output

```
Hello from 8051!!!
ABC123          <- typed in PuTTY, echoed back by the 8051
```

PuTTY's local echo is off by default, so every character that appears after the greeting is one the 8051 has sent back.

## Task Files

| File | Purpose |
|---|---|
| [`day5/prog1.c`](day5/prog1.c) | Bare-metal C source: the UART driver (initialise, send, receive) and the echo program |
| [`day5/STARTUP.A51`](day5/STARTUP.A51) | Standard C51 startup code: clears internal RAM, sets up the stack, then jumps to `main` |
| [`day5/uartcommunication.uvproj`](day5/uartcommunication.uvproj) | Keil µVision project file for the AT89S52 target |
| [`day5/Objects/uartcommunication.hex`](day5/Objects/uartcommunication.hex) | Built Intel HEX file, ready to flash onto the board |

To rebuild, open `uartcommunication.uvproj` in Keil µVision and choose **Project → Build Target (F7)**. The HEX file is written to `day5/Objects/`.

## Inference

1. **Both ends must agree on the frame format.** UART has no shared clock, so the 8051 and PuTTY must use the same baud rate, data bits, parity and stop bits (9600-8-N-1). Each byte is framed by a start bit and a stop bit, which lets the receiver resynchronise on every character.
2. **Timer 1 in auto-reload mode is a natural baud-rate generator.** Mode 2 reloads TH1 into TL1 in hardware on every overflow, so the timer keeps producing evenly spaced overflows with no software involvement.
3. **The crystal frequency sets the achievable baud rates.** 11.0592 MHz divides exactly into 9600 baud, which is why it is the standard 8051 UART crystal. This board's 11.0952 MHz gives about 9,631 baud, a 0.33 % error that is well within what a UART tolerates.
4. **Status flags make the hardware and software work together.** `TI` tells the program that the previous character has left `SBUF`, so the next one can be written. `RI` tells it that a new character has arrived. Both flags must be cleared by software before the next transfer can be detected.
5. **The echo test checks both directions in one step.** If every character typed in PuTTY comes back unchanged, then the baud rate, framing, wiring, transmit path and receive path are all working.
