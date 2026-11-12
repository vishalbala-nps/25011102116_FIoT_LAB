# TASK 12C Node-RED Statistics of the BME280 Data

**Experiment:** [CONTINUOUS ASSESSMENT 12 – BME280 Sensor Interfacing Using I²C and Bosch Sensor API](../README.md)

---

### 1. Hardware Used

- **Development Board:** NUCLEO-F401RE
- **Microcontroller IC:** STM32F401RET6
- **I²C Pins:** PB6 (SCL), PB7 (SDA) → BME280
- **UART1 Pins (Wemos):** PA9 (USART1_TX) → Wemos RX, PA10 (USART1_RX) ← Wemos TX
- **Peripherals:** BME280, Wemos D1 Mini (ESP8266) HTTP server, PC running Node-RED
- **Power Supply:** BME280 from 3.3 V, Wemos from 5 V, common GND

Circuit diagram: see the [experiment README](../README.md#circuit-diagram).

### 2. Core Logic / Pseudocode / Code Base

**Working Principle:**

Node-RED acts as an HTTP client of the STM32/ESP8266 server. An Inject node triggers an HTTP Request node every 5 seconds, which sends `GET http://<ESP8266_IP>/` and parses the JSON response. A Function node keeps the running minimum, maximum and average of each reading in its context and outputs the statistics table.

| Parameter | Minimum | Maximum | Average |
|---|---|---|---|
| Temperature (°C) | running min | running max | running average |
| Humidity (%RH) | running min | running max | running average |
| Pressure (hPa) | running min | running max | running average |

**Pseudocode:**

1. Every 5 seconds, send `GET /` to the ESP8266 IP address.
2. Parse the JSON response; ignore it if `status` is not `success`.
3. For each of temperature, humidity and pressure: update the minimum, maximum, sum and count.
4. Compute the average as sum / count.
5. Output the table of minimum, maximum and average to the Debug sidebar.

**Code Base:**

- Import the flow in Node-RED (**Menu → Import**), replace `<ESP8266_IP>` in the HTTP Request node with the IP address printed on the PC terminal, and **Deploy**.
- Nodes: Inject (every 5 s) → HTTP Request (GET, parsed JSON object) → Function (min / max / average in `context`) → Debug.

### 3. Inference

Node-RED polls the STM32 HTTP server and builds the minimum, maximum and average of the BME280 readings over time. Serving the data as JSON lets a standard HTTP client consume it directly, so analytics can be added on the PC without changing the firmware.
