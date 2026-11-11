# EXPERIMENT DETAILS

**EXP:** CONTINUOUS ASSESSMENT 10

**DATE:** DD/MM/YYYY

**NAME OF EXP:** STM32–Wemos Wi-Fi Modem Interface Using AT Commands

**HARDWARE SETUP / PLATFORM:** CubeMX + STM32CubeIDE + STM32 Development Board Blue Pill STM32F103C8T6 + Wemos D1 Mini (ESP8266) Wi-Fi Module + USB-to-TTL Serial Converter + PuTTY

---

## TASK 9B Explore the ESP8266 AT Command Interface

### 1. Circuit Diagram

The same circuit as Task 9A is used.

- **Development Board:** STM32 Blue Pill
- **Microcontroller IC:** STM32F103C8T6
- **UART Pins:** PA9/PA10 (USART1, Wemos), PA2/PA3 (USART2, PC)
- **Peripherals:** Wemos D1 Mini (ESP8266), USB-to-TTL converter
- **Power Supply:** 5 V to the Wemos, common GND

![Circuit Diagram](../circuit.png)

### 2. Core Logic / Pseudocode / Code Base

**Working Principle:**

Every AT command is a text line ending in CR/LF, and every reply ends with a status line (`OK` on success, `ERROR` or `FAIL` on failure). The commands needed for each requirement were identified from the ESP8266 AT-command documentation:

| Requirement | Command |
|---|---|
| Check modem response | `AT` |
| Check firmware/version | `AT+GMR` |
| Set Wi-Fi station mode | `AT+CWMODE=1` |
| Scan available networks | `AT+CWLAP` |
| Connect to an AP | `AT+CWJAP="<ssid>","<password>"` |
| Obtain network information | `AT+CIFSR` |
| Establish TCP connection | `AT+CIPSTART="TCP","<host>",<port>` |
| Send application data | `AT+CIPSEND=<length>` |

The STM32 sends the commands for the first four requirements to the Wemos one after another and displays each command and its response on the PC terminal. Joining an AP and reading the network information are carried out in Task 9C.

**Pseudocode:**

1. Initialize the Wemos as in Task 9A (boot delay, `ATE0`).
2. For each command `AT`, `AT+GMR`, `AT+CWMODE=1` and `AT+CWLAP`:
   - print `>> <command>` on the PC terminal;
   - send the command to the Wemos followed by CR/LF;
   - collect the response lines until `OK`, `ERROR` or `FAIL`;
   - print each response line as `<< <line>`.

**Code Base:**

- `showATCommand(cmd)` prints the command, sends it with `sendATCommand()` and prints the collected response.
- `sendATCommand(huart, cmd, output, size)` implements the request–response loop and is used for every command.
- The response is collected into the buffer `atResponse[1024]` first and printed afterwards, so no characters from the Wemos are missed while the PC terminal is being written.

**PC Terminal Output:**

```
STM32-WiFi MODEM TEST
>> AT
<< OK
>> AT+GMR
<< AT version:...
<< SDK version:...
<< OK
>> AT+CWMODE=1
<< OK
>> AT+CWLAP
<< +CWLAP:(3,"<ssid>",-60,"xx:xx:xx:xx:xx:xx",1,...)
<< OK
```

### 3. Inference

The ESP8266 AT interface follows a simple text request–response protocol: every command ends with CR/LF and every response ends with a status line. A single function that sends a command and waits for the final status line is enough to check the modem, read its firmware version, set station mode and scan the available networks.
