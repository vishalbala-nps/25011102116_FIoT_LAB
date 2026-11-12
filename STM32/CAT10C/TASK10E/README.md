# EXPERIMENT DETAILS

**EXP:** CONTINUOUS ASSESSMENT 10C

**DATE:** DD/MM/YYYY

**NAME OF EXP:** HTTP Server - STM32–Wemos Wi-Fi Modem Using AT Commands

**HARDWARE SETUP / PLATFORM:** CubeMX + STM32CubeIDE + STM32 Development Board Blue Pill STM32F103C8T6 + Wemos D1 Mini (ESP8266) Wi-Fi Module + USB-to-TTL Serial Converter + PuTTY + Browser / Node-RED

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

![Circuit Diagram](../circuit.png)

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

- STM32 firmware (same as Task 10D) and the Node-RED flow [`node-red-flow.json`](node-red-flow.json).
- Import the flow in Node-RED (**Menu → Import**), replace `<ESP8266_IP>` in the HTTP Request node with the IP address printed on the PC terminal, and **Deploy**.
- Nodes: Inject → HTTP Request (GET, UTF-8 string) → Debug (complete message).

### 3. Inference

Node-RED receives the same HTTP response as the browser, showing that the STM32/ESP8266 node behaves as a standard HTTP server that any HTTP client can query.
