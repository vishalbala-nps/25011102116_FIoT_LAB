# TASK 10A Establish STM32 ↔ Wemos UART Communication

**Experiment:** [CONTINUOUS ASSESSMENT 10 – STM32–Wemos Wi-Fi Modem Interface Using AT Commands and HTTP Server](../README.md)

---

### 1. Hardware Used

- **Development Board:** STM32 Blue Pill
- **Microcontroller IC:** STM32F103C8T6 (ARM Cortex-M3, 72 MHz)
- **UART1 Pins (Wemos):** PA9 (USART1_TX) → Wemos RX, PA10 (USART1_RX) ← Wemos TX
- **UART2 Pins (PC):** PA2 (USART2_TX) → converter RX, PA3 (USART2_RX) ← converter TX
- **Peripherals:** Wemos D1 Mini (ESP8266) running AT firmware, USB-to-TTL (3.3 V) converter to PuTTY
- **Power Supply:** Wemos powered from 5 V; STM32, Wemos and converter share a common GND

| UART | Connection | Purpose |
|---|---|---|
| USART1 | STM32 ↔ Wemos | AT commands and responses |
| USART2 | STM32 ↔ PC | Debug / display |

![Circuit Diagram](../circuit.png)

### 2. Core Logic / Pseudocode / Code Base

**Working Principle:**

Both UARTs are configured at **115200 baud, 8-N-1**, the default rate of the ESP8266 AT firmware. The STM32 sends `AT` followed by CR/LF to the Wemos and reads its reply line by line until the final status line (`OK`, `ERROR` or `FAIL`). The command and the Wemos response are then displayed on the PC terminal through USART2.

**Pseudocode:**

1. Initialize the HAL library, clock, USART1 and USART2.
2. Print `STM32-WiFi MODEM TEST` on the PC terminal.
3. Wait 2 s for the ESP8266 to boot and clear the UART overrun flag.
4. Send `ATE0` to turn off command echo.
5. Print `>> AT` on the PC terminal and send `AT\r\n` to the Wemos.
6. Collect the Wemos response until `OK`, `ERROR` or `FAIL`.
7. Print each response line on the PC terminal as `<< <line>`.

**Code Base:**

- `sendUART()` transmits a string followed by `\r\n` using `HAL_UART_Transmit()`.
- `recieveUART()` reads characters with `HAL_UART_Receive()` until a complete line ends with CR or LF.
- `sendATCommand()` sends one AT command and collects every response line until `OK` (returns 1) or `ERROR`/`FAIL` (returns 0).
- `showATCommand()` prints `>> <command>`, sends it with `sendATCommand()` and prints every response line as `<< <line>`.

**PC Terminal Output:**

```
STM32-WiFi MODEM TEST
>> AT
<< OK
```

### 3. Inference

The Wemos replies `OK` to the `AT` command, proving that the two boards communicate over UART. Using a separate UART for the PC keeps the debug messages apart from the AT-command traffic.
