# EXPERIMENT DETAILS

**EXP:** CONTINUOUS ASSESSMENT 10C

**DATE:** DD/MM/YYYY

**NAME OF EXP:** HTTP Server - STM32–Wemos Wi-Fi Modem Using AT Commands

**HARDWARE SETUP / PLATFORM:** CubeMX + STM32CubeIDE + STM32 Development Board Blue Pill STM32F103C8T6 + Wemos D1 Mini (ESP8266) Wi-Fi Module + USB-to-TTL Serial Converter + PuTTY + Browser / Node-RED

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

![Circuit Diagram](../circuit.png)

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
