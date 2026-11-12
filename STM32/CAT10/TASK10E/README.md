# TASK 10E Node-RED as a Second HTTP Client of STM32/ESP8266 HTTP Server

**Experiment:** [CONTINUOUS ASSESSMENT 10 – STM32–Wemos Wi-Fi Modem Interface Using AT Commands and HTTP Server](../README.md)

---

### 1. Hardware Used

- **Development Board:** STM32 Blue Pill
- **Microcontroller IC:** STM32F103C8T6 (ARM Cortex-M3, 72 MHz)
- **UART1 Pins (Wemos):** PA9 (USART1_TX) → Wemos RX, PA10 (USART1_RX) ← Wemos TX
- **UART2 Pins (PC):** PA2 (USART2_TX) → converter RX, PA3 (USART2_RX) ← converter TX
- **Peripherals:** Wemos D1 Mini (ESP8266) running AT firmware, USB-to-TTL (3.3 V) converter to PuTTY, PC running Node-RED on the same Wi-Fi network
- **Power Supply:** Wemos powered from 5 V; STM32, Wemos and converter share a common GND

Circuit diagram: see the [experiment README](../README.md#circuit-diagram).

### 2. Core Logic / Pseudocode / Code Base

**Working Principle:**

The firmware of Task 10D is used, so the STM32 serves the static page on port 80. Node-RED acts as a second HTTP client: an Inject node triggers the request, the HTTP Request node uses `http://<ESP8266_IP>/`, and the Debug node displays the HTTP response received from the STM32.

**Pseudocode:**

1. Run the Task 10D firmware on the STM32.
2. Click the Inject node to trigger a request.
3. The HTTP Request node sends `GET /` to the ESP8266 IP address.
4. The STM32 answers with the static HTML response.
5. The Debug node shows the response.

**Code Base:**

- STM32 firmware: same as Task 10D.
- Node-RED flow: [`node-red-flow.json`](node-red-flow.json) – Inject → HTTP Request (`GET http://<ESP8266_IP>/`) → Debug.
- Import the flow in Node-RED (**Menu → Import**), replace `<ESP8266_IP>` with the IP address printed on the PC terminal, and **Deploy**.

### 3. Inference

Node-RED receives the same HTTP response as the browser, showing that the STM32/ESP8266 node behaves as a standard HTTP server that any HTTP client can query.
