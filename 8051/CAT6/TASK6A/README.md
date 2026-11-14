# TASK 6A UART Initialization and Character Transmission

**Experiment:** [CONTINUOUS ASSESSMENT 6 – UART Driver and Serial Debugging](../README.md)

---

### 1. Hardware Used

- **Microcontroller:** AT89S52 (8051 core) on the AT89S52 development kit
- **UART Pins:** P3.1 (TXD) → USB-to-TTL RX, P3.0 (RXD) ← USB-to-TTL TX
- **Peripheral:** USB-to-TTL serial converter connected to the PC running PuTTY (9600-8-N-1)
- **Power Supply:** 5V DC; converter GND connected to board GND

![Circuit Diagram](../circuit.png)

### 2. Core Logic / Pseudocode / Code Base

**Working Principle:**

The serial port is set to Mode 1, an 8-bit UART with a variable baud rate, with reception enabled (`SCON = 0x50`). Timer 1 in Mode 2 (8-bit auto-reload, `TMOD = 0x20`) generates the baud rate; with `TH1 = 0xFD` the rate is 11.0952 MHz / (12 × 32 × 3) ≈ 9631 baud, within tolerance of 9600. A character is transmitted by writing it to SBUF and waiting for the TI flag, and received by waiting for the RI flag and reading SBUF. The program transmits the string `Hello from 8051` continuously, followed by CR/LF and a short pause.

**Pseudocode:**

1. Configure the UART: `SCON = 0x50`, `TMOD = 0x20`, `TH1 = TL1 = 0xFD`, start Timer 1.
2. For each character of `Hello from 8051`: write it to SBUF, wait until TI = 1, clear TI.
3. Send `\r\n` to move to a new line.
4. Wait for a short software delay.
5. Repeat continuously.

**Code Base:**

- `initUART()` configures 9600-8-N-1.
- `sendChar()` writes SBUF and waits for TI; `sendString()` sends a string followed by `\r\n`.

### 3. Inference

`Hello from 8051` appears repeatedly in PuTTY, verifying the UART configuration, the baud rate and the transmit path.
