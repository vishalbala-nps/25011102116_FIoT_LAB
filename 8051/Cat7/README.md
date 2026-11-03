# CAT 7 – 16×2 LCD Driver Development and Dynamic Display Control on the 8051 (AT89S52)

## Aim

Write a 4-bit LCD driver in bare-metal Embedded C for a 16×2 HD44780-compatible character LCD. The driver sets up the LCD, positions the cursor, displays characters, strings and numbers on both rows, updates a value on the display, and scrolls the display.

## Hardware Setup

| Item | Details |
|---|---|
| Development board | AT89S52 development kit |
| Microcontroller | Microchip (Atmel) AT89S52, 40-pin DIP |
| Clock | 11.0952 MHz (on-board crystal) |
| Power | 5 V DC |
| Programming | ISP (In-System Programming) over SPI |
| Display | Dev kit's built-in 16×2 character LCD (HD44780-compatible) on **Port 3**, 4-bit interface |
| Host software | Keil µVision 5 with the C51 compiler (C51 V9.60) |
| Hardware debug | Multimeter, DSO |

### Pin Assignment

| LCD signal | Port pin | DIP pin | Function |
|---|---|---|---|
| RS | P3.2 | 12 | Register Select: 0 = command, 1 = character data |
| E | P3.3 | 13 | Enable: the LCD reads the data lines when E goes from 1 to 0 |
| D4 | P3.4 | 14 | Data bit 4 |
| D5 | P3.5 | 15 | Data bit 5 |
| D6 | P3.6 | 16 | Data bit 6 |
| D7 | P3.7 | 17 | Data bit 7 |

In 4-bit mode only D4–D7 are used, and each byte is sent as two halves (nibbles), high nibble first. This saves four port pins compared with the 8-bit interface.

## Circuit Diagram

```
                 AT89S52                          16x2 LCD
          +------------------+                 (HD44780, built-in)
          | 12  P3.2         |---------------->| RS             |
          | 13  P3.3         |---------------->| E              |
          |                  |                 |                |
          | 14  P3.4         |---------------->| D4             |
          | 15  P3.5         |---------------->| D5             |
          | 16  P3.6         |---------------->| D6             |
          | 17  P3.7         |---------------->| D7             |
          |                  |                 +----------------+
          | 40  VCC          |------ +5V
          +------------------+
```

- **RS and E** are the control lines. RS tells the LCD whether a byte is a command or a character, and the falling edge of E tells it to read the data lines.
- **D4–D7** carry each byte as two 4-bit nibbles.
- The LCD's power, contrast (V0) and RW (tied low for write-only use), together with the crystal and reset circuit, are built into the dev kit and are not shown.

## Code Logic

The program, [lcd.c](day6/lcd.c), uses only the `reg52.h` register definitions. The six LCD lines are mapped onto single Port 3 bits with `sbit`, and every LCD operation is built from writes to those bits.

### Functions

| Function | Purpose |
|---|---|
| `void delay_us(unsigned int us)` | Software delay loop used to meet the LCD's timing requirements |
| `void send_nibble(bit b3, bit b2, bit b1, bit b0)` | Puts 4 bits on D7–D4 and pulses E (1 → 0) so the LCD reads them |
| `void lcd_write(unsigned char value)` | Sends a byte as two nibbles, high nibble first |
| `void lcd_cmd(unsigned char cmd)` | Sends a command (RS = 0). Clear (0x01) and Return Home (0x02) get a longer delay because they take longer to execute. |
| `void lcd_data(unsigned char ch)` | Sends a character (RS = 1), which appears at the current cursor position |
| `void lcd_init()` | Runs the HD44780 start-up sequence for 4-bit, 2-line operation |
| `void lcd_goto(unsigned char row, unsigned char col)` | Moves the cursor to a row (0 or 1) and column (0–15) |
| `void lcd_puts(char *s)` | Writes a string, one character at a time |
| `void lcd_print_number(unsigned int value)` | Writes an unsigned number in decimal |
| `void writeText()` | Writes "HI DA" on row 1 and "Shift:" on row 2 |
| `void main()` | Sets up the LCD, writes the text, then repeatedly updates the shift count and scrolls the display |

### LCD Initialisation (from the HD44780 datasheet)

The LCD powers up in 8-bit mode, so the first steps are single nibbles that switch it into 4-bit mode. After that, every command is a full byte sent as two nibbles.

| Step | Sent | Meaning |
|---|---|---|
| 1 | wait | Let the LCD finish its internal power-on reset |
| 2 | nibble `0011`, wait | Function Set (8-bit), first time |
| 3 | nibble `0011`, wait | Function Set (8-bit), second time |
| 4 | nibble `0011`, wait | Function Set (8-bit), third time. The LCD is now in a known state. |
| 5 | nibble `0010`, wait | Function Set: switch to **4-bit** mode |
| 6 | `0x28` | Function Set: 4-bit, **2 lines**, 5×8 font |
| 7 | `0x0C` | Display Control: display ON, cursor OFF, blink OFF |
| 8 | `0x06` | Entry Mode: cursor moves right after each character, display does not shift |
| 9 | `0x01` | Clear Display: clears all characters and returns the cursor home |

### Command Reference

| Command | Value | Used for |
|---|---|---|
| Function Set | `0x28` | 4-bit, 2-line, 5×8 font |
| Display Control | `0x0C` | Display on, cursor and blink off |
| Entry Mode Set | `0x06` | Move the cursor right after each character |
| Clear Display | `0x01` | Clear the screen |
| Return Home | `0x02` | Cursor to (0, 0) and undo any display shift |
| Display Shift Right | `0x1C` | Shift the whole display one position right |
| Set DDRAM Address | `0x80 + address` | Move the cursor |

### Cursor Positioning (DDRAM Addresses)

Each character on the screen has an address in the LCD's display memory (DDRAM). The two rows start at different addresses:

| Row | DDRAM addresses (columns 0–15) | Set-cursor command |
|---|---|---|
| Row 1 (`row = 0`) | `0x00` – `0x0F` | `0x80 + col` |
| Row 2 (`row = 1`) | `0x40` – `0x4F` | `0xC0 + col` |

### Number Display

`lcd_print_number()` finds the digits of the value by repeatedly taking `value % 10` (the last digit) and dividing by 10. Each digit is converted to its character with `+ '0'`. The digits come out least significant first, so they are stored in a small array and printed in reverse order. A value of 0 is handled separately so that it prints as `0`.

### Display Output

```
+----------------+
|HI DA           |    row 1
|Shift: 7        |    row 2: number updates every step (0 to 22)
+----------------+
```

On every step the program rewrites the number after "Shift:", then shifts the whole display one position right. After 22 shifts it sends Return Home, which puts the text back in its starting position, and the count starts again from 0.

### Pseudocode

```
FUNCTION send_nibble(b3, b2, b1, b0):
    D7..D4 <- b3..b0
    E <- 1; wait; E <- 0                 // LCD reads on the falling edge

FUNCTION lcd_write(value):
    send_nibble(high 4 bits of value)
    send_nibble(low 4 bits of value)

FUNCTION lcd_cmd(cmd):   RS <- 0; lcd_write(cmd); wait (longer for 0x01, 0x02)
FUNCTION lcd_data(ch):   RS <- 1; lcd_write(ch);  wait

FUNCTION lcd_goto(row, col):
    lcd_cmd((row == 0 ? 0x80 : 0xC0) + col)

FUNCTION lcd_puts(s):
    FOR each character ch in s: lcd_data(ch)

FUNCTION lcd_print_number(value):
    IF value == 0: lcd_data('0'); RETURN
    WHILE value > 0: store (value % 10) + '0'; value <- value / 10
    print the stored digits in reverse order

MAIN:
    lcd_init()
    lcd_goto(0, 0); lcd_puts("HI DA")
    lcd_goto(1, 0); lcd_puts("Shift:")
    count <- 0
    LOOP forever:
        lcd_goto(1, 7); lcd_puts("  ")    // erase the old number
        lcd_goto(1, 7); lcd_print_number(count)
        IF count == 22: lcd_cmd(0x02); count <- 0       // return home
        ELSE:           lcd_cmd(0x1C); count <- count + 1 // shift right
        wait
```

## Task Files

| File | Purpose |
|---|---|
| [`day6/lcd.c`](day6/lcd.c) | Bare-metal C source: the 4-bit LCD driver and the scrolling display program |
| [`day6/STARTUP.A51`](day6/STARTUP.A51) | Standard C51 startup code: clears internal RAM, sets up the stack, then jumps to `main` |
| [`day6/lcd.uvproj`](day6/lcd.uvproj) | Keil µVision project file for the AT89S52 target |
| [`day6/Objects/lcd.hex`](day6/Objects/lcd.hex) | Built Intel HEX file, ready to flash onto the board |

To rebuild, open `lcd.uvproj` in Keil µVision and choose **Project → Build Target (F7)**. The HEX file is written to `day6/Objects/`.

## Inference

1. **The datasheet defines the driver.** Every value the program sends, from the start-up sequence to the cursor addresses, comes from the HD44780 command table. Writing the driver is a matter of finding the right command fields and combining their bits.
2. **RS and E give meaning and timing to the data lines.** The same four data lines carry both commands and characters. RS tells the LCD which one it is receiving, and the falling edge of E tells it exactly when to read the lines.
3. **4-bit mode trades speed for pins.** Sending each byte as two nibbles takes twice as many transfers, but frees four port pins. Text output is slow anyway, so the extra transfer time does not matter.
4. **Layering makes the driver reusable.** `send_nibble()` handles the pins, `lcd_write()` handles bytes, `lcd_cmd()` and `lcd_data()` add the meaning, and `lcd_goto()`, `lcd_puts()` and `lcd_print_number()` build on those. Higher-level code never touches the port pins directly.
5. **The two rows are separate memory ranges.** Row 1 starts at DDRAM address 0x00 and row 2 at 0x40, so moving to row 2 means sending `0xC0 + column`, not continuing past column 15.
6. **The LCD keeps its own copy of the text.** Display shift (0x1C) moves what is shown without rewriting any characters, and Return Home (0x02) undoes the shift. Only the changing number on row 2 needs to be rewritten on each step.
