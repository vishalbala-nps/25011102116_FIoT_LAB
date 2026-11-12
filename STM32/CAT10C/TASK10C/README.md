# EXPERIMENT DETAILS

**EXP:** CONTINUOUS ASSESSMENT 10C

**DATE:** DD/MM/YYYY

**NAME OF EXP:** HTTP Server - STM32–Wemos Wi-Fi Modem Using AT Commands

**HARDWARE SETUP / PLATFORM:** CubeMX + STM32CubeIDE + STM32 Development Board Blue Pill STM32F103C8T6 + Wemos D1 Mini (ESP8266) Wi-Fi Module + USB-to-TTL Serial Converter + PuTTY + Browser / Node-RED

---

## TASK 10C Configure ESP8266 as a TCP Server

### 1. Circuit Diagram

The STM32 drives the Wemos over USART1 and shows the results on the PC terminal over USART2.

- **Development Board:** STM32 Blue Pill
- **Microcontroller IC:** STM32F103C8T6 (ARM Cortex-M3, 72 MHz)
- **UART1 Pins (Wemos):** PA9 (USART1_TX) → Wemos RX, PA10 (USART1_RX) ← Wemos TX
- **UART2 Pins (PC):** PA2 (USART2_TX) → converter RX, PA3 (USART2_RX) ← converter TX
- **Peripherals:** Wemos D1 Mini (ESP8266) running AT firmware, USB-to-TTL (3.3 V) converter to PuTTY (115200-8-N-1)
- **Power Supply:** Wemos powered from 5 V; STM32, Wemos and converter share a common GND

![Circuit Diagram](../circuit.png)

### 2. Core Logic / Pseudocode / Code Base

**Working Principle:**

The ESP8266 first joins the Wi-Fi network as a station, as in CAT10. It is then configured as a TCP server with `AT+CIPMUX=1` (multiple connections) and `AT+CIPSERVER=1,80` (listen on port 80). Everything the ESP8266 sends afterwards is forwarded to the PC terminal, so the connection events and the `+IPD` frame generated when a browser opens `http://<IP Address>/` can be observed.

**Pseudocode:**

1. Initialize HAL, the system clock, USART1 (Wemos) and USART2 (PC), with the USART1 interrupt enabled.
2. Send `ATE0` and `AT`, set station mode (`AT+CWMODE=1`) and join the Wi-Fi network (`AT+CWJAP`).
3. Read and print the IP address (`AT+CIFSR`).
4. Send `AT+CIPMUX=1` and `AT+CIPSERVER=1,80` and print the result.
5. Forward every character received from the ESP8266 to the PC terminal.

**Code Base:**

- The ESP8266 / HTTP server functions are reused from the BME280 HTTP experiment (CAT12):

| Function | Purpose |
|---|---|
| `sendUART()` / `recieveUART()` | Send a line / receive a line over a UART |
| `sendATCommand()` | Send an AT command and collect the response until `OK`, `ERROR` or `FAIL` |
| `initWifi()`, `setMode()`, `joinAP()`, `getIP()` | Initialize the ESP8266, set station mode, join the Wi-Fi network, read the IP address |
| `initServer()` | `AT+CIPMUX=1`, `AT+CIPSERVER=1,80`, then start interrupt reception on USART1 |
| `HAL_UART_RxCpltCallback()` | Builds lines from the ESP8266 and tracks `CONNECT`, `+IPD` and `CLOSED` for each link |
| `handleResponse()` | Extracts the path after `GET` for every link holding a request and calls the routing function |
| `sendHTML()` | Builds the raw HTTP response and sends it with `AT+CIPSEND`, then `AT+CIPCLOSE` |

- `HAL_UART_Receive()` on USART1 and `HAL_UART_Transmit()` on USART2 forward the raw ESP8266 traffic to PuTTY.

**PC Terminal Output:**

```
Initializing Wifi...
Initialized Wifi
Connecting to Wifi...
Connected to Wifi Successfully
IP Address: xxx.xxx.xxx.xxx
TCP server started on port 80
Waiting for clients...
0,CONNECT

+IPD,0,XXX:GET / HTTP/1.1
Host: xxx.xxx.xxx.xxx
...
```

### 3. Inference

With `AT+CIPMUX=1` and `AT+CIPSERVER=1,80` the ESP8266 accepts TCP connections on port 80. A browser request arrives at the STM32 as a `+IPD,<link>,<length>:` frame containing the raw HTTP request, showing that the STM32 sees the network data as plain text over the UART.
