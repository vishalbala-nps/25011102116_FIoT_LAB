# TASK 13B Raw HTTP Response with BME280 Data

**Experiment:** [CONTINUOUS ASSESSMENT 13 – BME280 Sensor Interfacing Using I²C and Bosch Sensor API](../README.md)

---

### 1. Hardware Used

- **Development Board:** NUCLEO-F401RE
- **Microcontroller IC:** STM32F401RET6
- **I²C Pins:** PB6 (SCL), PB7 (SDA) → BME280
- **UART1 Pins (Wemos):** PA9 (USART1_TX) → Wemos RX, PA10 (USART1_RX) ← Wemos TX
- **UART6 Pins (PC):** PC6 (USART6_TX) → converter RX, PC7 (USART6_RX) ← converter TX
- **Peripherals:** BME280, Wemos D1 Mini (ESP8266) running AT firmware, USB-to-TTL converter to PuTTY
- **Power Supply:** BME280 from 3.3 V, Wemos from 5 V, common GND

Circuit diagram: see the [experiment README](../README.md#circuit-diagram).

### 2. Core Logic / Pseudocode / Code Base

**Working Principle:**

The ESP8266 joins the Wi-Fi network using AT commands and is set up as a TCP server on port 80 (`AT+CIPMUX=1`, `AT+CIPSERVER=1,80`). Bytes from the ESP8266 are received with a UART interrupt, which tracks client connections (`CONNECT`, `CLOSED`) and stores incoming requests (`+IPD`). The main loop answers each pending request with a raw HTTP response whose Content-Length is calculated from the actual body length, and sends it with `AT+CIPSEND`. A request for `/` returns the compensated BME280 readings as a JSON payload:

```
{"status":"success","temperature":28.45,"humidity":62.30,"pressure":1008.72}
```

**Pseudocode:**

1. Initialize the BME280 (Task 13A).
2. Initialize the ESP8266 (`ATE0`, `AT`), set station mode and join the Wi-Fi network.
3. Print the IP address on the PC terminal (USART6).
4. Start the TCP server on port 80 and enable interrupt reception on USART1.
5. In the loop:
   - for each connection holding a request, extract the path after `GET`;
   - if the path is `/`, build the JSON payload from the temperature, humidity and pressure, else `Page not found`;
   - build the HTTP response with `Content-Type: application/json` and `Content-Length = strlen(json)`;
   - send `AT+CIPSEND=<link>,<length>`, wait for `>`, send the response, wait for `SEND OK`, then `AT+CIPCLOSE=<link>`;
   - read new sensor data.
6. Repeat every 2 seconds.

**Code Base:**

- `HAL_UART_RxCpltCallback()` builds lines from the ESP8266 and updates the `connections[]` state.
- `handleResponse()` and `handleRouting()` extract the requested path and choose the response; the JSON payload is built with `snprintf(json, sizeof(json), "{\"status\":\"success\",\"temperature\":%.2f,…}", …)`.
- `sendHTML(content, type, link)` builds the raw response with `snprintf()`:

```
HTTP/1.1 200 OK
Content-Type: application/json
Content-Length: <strlen(json)>
Connection: close

{"status":"success","temperature":28.45,"humidity":62.30,"pressure":1008.72}
```

- `onWifiDisconnect()` waits for `WIFI GOT IP` and restarts the server if the Wi-Fi connection drops.

### 3. Inference

Opening `http://<IP Address>/` in a browser on the same network shows the temperature, humidity and pressure measured by the BME280 as JSON. The STM32 builds a valid raw HTTP response with the Content-Length calculated from the actual JSON string length and sends it through the ESP8266, showing how sensor data can be served over Wi-Fi using AT commands.
