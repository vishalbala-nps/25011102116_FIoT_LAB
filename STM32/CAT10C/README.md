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

![Circuit Diagram](circuit.png)

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

- **Project folder:** [`TASK10C`](TASK10C/)
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

---

## TASK 10D Observe the HTTP Request and Construct a Static HTTP Response

### 1. Circuit Diagram

The same circuit as Task 10C is used.

- **Development Board:** STM32 Blue Pill
- **Microcontroller IC:** STM32F103C8T6 (ARM Cortex-M3, 72 MHz)
- **UART1 Pins (Wemos):** PA9 (USART1_TX) → Wemos RX, PA10 (USART1_RX) ← Wemos TX
- **UART2 Pins (PC):** PA2 (USART2_TX) → converter RX, PA3 (USART2_RX) ← converter TX
- **Peripherals:** Wemos D1 Mini (ESP8266) running AT firmware, USB-to-TTL (3.3 V) converter to PuTTY (115200-8-N-1)
- **Power Supply:** Wemos powered from 5 V; STM32, Wemos and converter share a common GND

![Circuit Diagram](circuit.png)

### 2. Core Logic / Pseudocode / Code Base

**Working Principle:**

When a browser sends `GET /`, the ESP8266 passes it to the STM32 as an `+IPD` frame. The USART1 receive interrupt stores the request for that link, and the main loop extracts the requested path and prints it on the PC terminal. For `/`, the STM32 constructs a static HTTP response (status line, headers, blank line, HTML body) with the Content-Length calculated from the body, and sends it in two steps: `AT+CIPSEND=<link>,<length>`, wait for the `>` prompt, then the response bytes.

```
HTTP/1.1 200 OK
Content-Type: text/html
Content-Length: <body_length>
Connection: close

<html><body><h1>STM32 IoT Node</h1><p>Hello from STM32</p></body></html>
```

**Pseudocode:**

1. Join the Wi-Fi network and start the TCP server on port 80 (Task 10C).
2. In the interrupt, collect lines from the ESP8266; on `+IPD,<link>,<len>:<data>` store the request for that link.
3. In the main loop, for each link holding a request, extract the path after `GET` and print it.
4. If the path is `/`, build the static HTML response with `Content-Length = strlen(body)`; otherwise send `Page not found`.
5. Send `AT+CIPSEND=<link>,<length>`, wait for `>`, send the response, wait for `SEND OK`, then `AT+CIPCLOSE=<link>`.
6. Repeat continuously.

**Code Base:**

- **Project folder:** [`TASK10D`](TASK10D/)
- Uses the same ESP8266 / HTTP server functions as Task 10C (from the BME280 HTTP experiment).
- `handleRouting(url, client)` prints `<link>: GET <path>` and calls `sendHTML(html, "text/html", client)` for `/`.
- `sendHTML()` builds the response with `snprintf()`, sends `AT+CIPSEND=<link>,<length>`, waits for `>` and transmits the bytes.

**PC Terminal Output:**

```
TCP server started on port 80
0: GET /
Static HTML response sent
```

### 3. Inference

The browser at `http://<IP Address>/` shows the "STM32 IoT Node" page served by the STM32. An HTTP response is just text with an exact length: the STM32 builds the status line, headers and body itself, and the ESP8266 only transports the bytes over TCP.

---

## TASK 10E Node-RED as a Second HTTP Client of STM32/ESP8266 HTTP Server

### 1. Circuit Diagram

The circuit and firmware from Task 10D are used; Node-RED runs on a PC on the same Wi-Fi network.

- **Development Board:** STM32 Blue Pill
- **Microcontroller IC:** STM32F103C8T6 (ARM Cortex-M3, 72 MHz)
- **UART1 Pins (Wemos):** PA9 (USART1_TX) → Wemos RX, PA10 (USART1_RX) ← Wemos TX
- **UART2 Pins (PC):** PA2 (USART2_TX) → converter RX, PA3 (USART2_RX) ← converter TX
- **Peripherals:** Wemos D1 Mini (ESP8266) running AT firmware, USB-to-TTL (3.3 V) converter to PuTTY (115200-8-N-1)
- **Power Supply:** Wemos powered from 5 V; STM32, Wemos and converter share a common GND

![Circuit Diagram](circuit.png)

### 2. Core Logic / Pseudocode / Code Base

**Working Principle:**

Node-RED acts as a second HTTP client of the same STM32/ESP8266 server. An Inject node triggers an HTTP Request node, which sends `GET http://<ESP8266_IP>/`, and the Debug node displays the HTTP response received from the STM32.

**Pseudocode:**

1. Run the Task 10D firmware so the STM32 serves the static page on port 80.
2. In Node-RED, click the Inject node to trigger a request.
3. The HTTP Request node sends `GET /` to the ESP8266 IP address.
4. The STM32 answers with the static HTML response.
5. The Debug node shows the response body, status code and headers.

**Code Base:**

- **Project folder:** [`TASK10E`](TASK10E/) – STM32 firmware (same as Task 10D) and the Node-RED flow [`node-red-flow.json`](TASK10E/node-red-flow.json).
- Import the flow in Node-RED (**Menu → Import**), replace `<ESP8266_IP>` in the HTTP Request node with the IP address printed on the PC terminal, and **Deploy**.
- Nodes: Inject → HTTP Request (GET, UTF-8 string) → Debug (complete message).

### 3. Inference

Node-RED receives the same HTTP response as the browser, showing that the STM32/ESP8266 node behaves as a standard HTTP server that any HTTP client can query.
