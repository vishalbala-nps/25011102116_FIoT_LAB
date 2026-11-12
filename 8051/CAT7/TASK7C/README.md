# TASK 7C 4×4 Keypad-Based Character Movement

**Experiment:** [CONTINUOUS ASSESSMENT 7 – 16×2 LCD Driver Development and Dynamic Display Control](../README.md)

---

### 1. Hardware Used

- **Microcontroller:** AT89S52 (8051 core) on the AT89S52 development kit
- **LCD Pins:** LCD plugged into the DS1 header – RS → P3.2 (D8), E → P3.3 (D7), D4–D7 → P3.4–P3.7 (D5–D2), RW → GND (4-bit mode)
- **Keypad Pins:** Built-in 4×4 matrix keypad on Port 2 – rows P2.7–P2.4, columns P2.3–P2.0
- **Peripherals:** Built-in 16×2 character LCD, built-in 4×4 matrix keypad
- **Power Supply:** 5V DC

Circuit diagram: see the [experiment README](../README.md#circuit-diagram).

### 2. Core Logic / Pseudocode / Code Base

**Working Principle:**

A `*` character starts at the first position of row 1. The keypad is scanned row by row, and keys 2, 8, 4 and 6 move the character up, down, left and right. The character stays within the 16×2 display: rows 0–1 and columns 0–15. On each move, the old position is overwritten with a space and the character is drawn at the new position with `lcd_goto()`. The program waits for the key to be released, so one press moves one step.

| Key | Action |
|---|---|
| 2 | ↑ Move Up |
| 8 | ↓ Move Down |
| 4 | ← Move Left |
| 6 | → Move Right |

**Pseudocode:**

1. Initialize the LCD and draw `*` at (0, 0).
2. Scan the keypad; if no key is pressed, repeat the scan.
3. Erase the character at the current position.
4. Update the row or column for the key, keeping it within the display.
5. Draw `*` at the new position.
6. Wait until the key is released, then repeat.

**Code Base:**

- `btn()` drives each keypad row low, reads the columns and returns the debounced key 2, 4, 6 or 8 (−1 if none).
- `lcd_goto()` and `lcd_data()` from the LCD driver erase and redraw the character.

### 3. Inference

The character moves around the 16×2 display one position per key press. Combining the keypad scan with the LCD driver's cursor positioning creates a simple interactive display application.
