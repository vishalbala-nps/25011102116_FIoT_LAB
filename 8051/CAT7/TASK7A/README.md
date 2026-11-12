# TASK 7A LCD Initialization and Character/String Display

**Experiment:** [CONTINUOUS ASSESSMENT 7 – 16×2 LCD Driver Development and Dynamic Display Control](../README.md)

---

### 1. Hardware Used

- **Microcontroller:** AT89S52 (8051 core) on the AT89S52 development kit
- **LCD Pins:** LCD plugged into the DS1 header – RS → P3.2 (D8), E → P3.3 (D7), D4–D7 → P3.4–P3.7 (D5–D2), RW → GND (4-bit mode)
- **Peripheral:** Built-in 16×2 character LCD (HD44780-compatible)
- **Power Supply:** 5V DC

Circuit diagram: see the [experiment README](../README.md#circuit-diagram).

### 2. Core Logic / Pseudocode / Code Base

**Working Principle:**

`lcd_init()` follows the HD44780 4-bit start-up sequence: three `0011` nibbles, then `0010` to switch to 4-bit mode, followed by Function Set `0x28` (4-bit, 2 lines, 5×8 font), Display Control `0x0C` (display ON, cursor OFF), Entry Mode `0x06` (cursor moves right) and Clear Display `0x01`. `lcd_cmd()` sends a command with RS = 0 and `lcd_data()` sends a character with RS = 1; each byte is sent as two nibbles, latched on the falling edge of E. The cursor is placed with Set DDRAM Address commands: `0x80` is the first position of row 1 and `0xC0` the first position of row 2.

**Pseudocode:**

1. Initialize the LCD in 4-bit, 2-line mode with `lcd_init()`.
2. Send `0x80` and display the character `A` at the first position of the first row.
3. Send `0x82` and display the string `HELLO` character by character.
4. Send `0xC0` and display the string `8051 LCD DRIVER` on the second row.

**Code Base:**

- `send_nibble()` drives D7–D4 and pulses E; `lcd_write()` sends a byte as two nibbles.
- `lcd_init()`, `lcd_cmd()` and `lcd_data()` form the driver.

### 3. Inference

The character `A` and the two strings appear on both rows of the LCD. The commands and their bit fields come directly from the HD44780 datasheet.
