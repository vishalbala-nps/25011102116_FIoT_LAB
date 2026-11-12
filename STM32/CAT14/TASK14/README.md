# TASK 14 Server Rack Monitor

**Experiment:** [CONTINUOUS ASSESSMENT 14 – Server Rack Monitor](../README.md)

---

## STM32 Sensor Node

### 1. Hardware Used

- **Development Board:** STM32 Blue Pill
- **Microcontroller IC:** STM32F103C6T6A (ARM Cortex-M3, 72 MHz)
- **DHT11 Data Pin:** PB15 – output while sending the start signal, input while reading the sensor's reply
- **UART1 Pins (Wemos):** PA9 (USART1_TX) → Wemos RX, PA10 (USART1_RX) ← Wemos TX, 115200-8-N-1
- **GPIO Outputs:** PA5 warning LED (active-high), PA6 buzzer (active-high), PC13 on-board status LED (active-low)
- **Timer:** TIM2 with prescaler 71 → 72 MHz / 72 = 1 MHz, so one count = 1 µs for the DHT11 bit timing
- **Peripherals:** DHT11 sensor module, Wemos D1 Mini (ESP8266), warning LED, buzzer
- **Power Supply:** 3.3 V to the DHT11, 5 V to the Wemos, common GND

Circuit diagram: see the [experiment README](../README.md#circuit-diagram).

### 2. Core Logic / Pseudocode / Code Base

**Working Principle:**

The DHT11 uses a single-wire protocol. The STM32 pulls the data line low for 18 ms as a start signal, then releases it and times the sensor's response with TIM2 at 1 µs resolution. Each of the 40 data bits is sent as a ≈ 50 µs low pulse followed by a high pulse: ≈ 26 µs means 0 and ≈ 70 µs means 1. The five received bytes are humidity (integer, decimal), temperature (integer, decimal) and a checksum, which must match the sum of the first four bytes. Interrupts are disabled during the timed section so the bit timing is not disturbed.

The ESP8266 is driven with AT commands through the custom ESP8266 / MQTT library (ESPLib): it joins the Wi-Fi network, opens a TCP connection to `broker.emqx.io:1883` and the STM32 sends an MQTT CONNECT packet. Every valid reading is published as JSON to the topic `rack1/readings`. Locally, a temperature of 30 °C or more turns on the warning LED and sounds the buzzer at most once every 2 s; the status LED is on once the broker is connected and blinks off for each published reading.

| Condition | Warning LED (PA5) | Buzzer (PA6) | Status LED (PC13) |
|---|---|---|---|
| Starting up / connecting | off | off | off |
| Connected, temperature < 30 °C | off | off | on, blinks on each publish |
| Temperature ≥ 30 °C | on | 200 ms beep, at most every 2 s | on, blinks on each publish |

**Pseudocode:**

1. Initialize HAL, the 72 MHz clock, GPIO, TIM2 (1 µs tick) and USART1; turn the status LED off.
2. Initialize the DHT11 handle on PB15 with TIM2.
3. Initialize the ESP8266 in station mode, join the Wi-Fi network and open a TCP connection to `broker.emqx.io:1883`.
4. Send the MQTT CONNECT packet (client ID, clean session, keep-alive 500 s) and turn the status LED on.
5. Loop:
   - read the DHT11; if the read and checksum succeed:
     - store the temperature and humidity;
     - if the temperature is ≥ 30 °C, turn the warning LED on and enable beeping, else turn it off;
     - publish `{"t":<temperature>,"h":<humidity>}` to `rack1/readings` with QoS 0;
     - blink the status LED;
     - if beeping is enabled and 2 s have passed since the last beep, beep the buzzer;
   - wait 1.5 s.

**Code Base:**

| File | Purpose |
|---|---|
| [`Core/Src/main.c`](Core/Src/main.c) | Application: sensor reading, local alarm, MQTT publishing |
| [`Core/Src/DHT11.c`](Core/Src/DHT11.c), [`Core/Inc/DHT11.h`](Core/Inc/DHT11.h) | DHT11 driver: start signal, µs pulse timing with TIM2, 40-bit decoding, checksum |
| [`Core/Src/ESPLib.c`](Core/Src/ESPLib.c), [`Core/Inc/ESPLib.h`](Core/Inc/ESPLib.h) | ESP8266 AT-command driver and MQTT packet builder (CONNECT, PUBLISH) |
| [`ServerRackMonitor2.ioc`](ServerRackMonitor2.ioc) | STM32CubeMX configuration: pins, TIM2, USART1, clock tree |
| [`MDK-ARM/ServerRackMonitor2.uvprojx`](MDK-ARM/ServerRackMonitor2.uvprojx) | Keil MDK-ARM project |

- `DHT11_Init(&dht, &htim2, GPIOB, GPIO_PIN_15)` and `DHT11_Read(&dht)` read the sensor; `dht.temperature` and `dht.humidity` hold the result.
- `ESP8266_Init()`, `ESP8266_wifiConnect()`, `ESP8266_portConnect()` and `MQTT_connectBroker()` connect to the broker; `MQTT_publishTopic("rack1/readings", buf, 0)` publishes each reading.
- `WARNING_TEMP` (30 °C), `TEMP_ALARM` (2000 ms between beeps) and `WARNING_ENABLE` are set with `#define`s at the top of `main.c`, together with the Wi-Fi network, broker and client ID.

**Published Data (topic `rack1/readings`):**

```
{"t":28.40,"h":61.00}
```

### 3. Inference

The STM32 reads the DHT11 by timing its single-wire pulses with a 1 µs hardware timer and validates every frame with the checksum. Publishing each reading over MQTT through the ESP8266 makes the rack conditions available to any subscriber, while the local LED and buzzer still warn people at the rack even without the network.

---

## Node-RED Dashboard

### 1. Hardware Used

- **Server / PC:** Node-RED with Node-RED Dashboard 2.0, connected to the same MQTT broker
- **MQTT Broker:** `broker.emqx.io:1883`, topic `rack1/readings`
- **Computer in the rack:** reachable over SSH from the Node-RED host, for the automatic shutdown
- **Weather data:** OpenWeatherMap current-weather and forecast APIs

Circuit diagram: see the [experiment README](../README.md#circuit-diagram).

### 2. Core Logic / Pseudocode / Code Base

**Working Principle:**

An MQTT-in node subscribes to `rack1/readings` and feeds every reading to the dashboard and the alert logic:

| Dashboard part | How it works |
|---|---|
| Rack Temperature / Humidity gauges | Temperature 15–45 °C (green < 27, amber 27–30, red ≥ 30); humidity 0–100 % (green 30–60 %, amber outside, red ≥ 75 %) |
| Temperature chart | Line chart of the rack temperature, plotted together with the outside temperature |
| Last Updated | Time of the latest reading (Asia/Kolkata) |
| Status card | "Everything is fine" below 30 °C, otherwise a warning; updated only when the state changes |
| Spoken alert | Text-to-speech "Temperature too high" when the rack reaches 30 °C |
| Computer Status / shutdown | Alert after **3 consecutive readings above 30 °C**, cleared at ≤ 29 °C (hysteresis); on alert, an `exec` node runs `sudo shutdown -h now` on the rack computer over SSH |
| Outside weather card | OpenWeatherMap current weather every 10 min: temperature, feels-like, humidity, pressure, wind |
| Forecast advice | OpenWeatherMap forecast every 30 min: hottest period and rain chance in the next 24 h, with cooling / humidity advice |
| Rack vs outside | How much warmer or cooler the rack is than outside |

**Pseudocode:**

1. Subscribe to `rack1/readings` on `broker.emqx.io`.
2. For each reading `{t, h}`:
   - update the gauges, the chart and the "Last Updated" time;
   - set `ok = (t < 30)`; on a change of `ok`, update the status card, and announce "Temperature too high" when `ok` becomes false;
   - count consecutive readings above 30 °C; at 3 set `alert = true`, at ≤ 29 °C set `alert = false`; on a change, update "Computer Status" and, when `alert` becomes true, run the SSH shutdown command;
   - compare `t` with the latest outside temperature and show the difference.
3. Every 10 min, fetch the current weather and update the outside card and chart.
4. Every 30 min, fetch the forecast and update the advice.

**Code Base:**

- [`flows.json`](flows.json) – the complete Node-RED flow (Dashboard 2.0). Import it with **Menu → Import**.
- Flow settings used by the weather nodes (flow tab → **Properties → Environment variables**): `OWM_KEY` (OpenWeatherMap API key), `LAT`, `LON` (location) and `HOT_DAY` (forecast temperature that counts as a hot day). In the repository they hold the placeholders `YOUR_OPENWEATHERMAP_API_KEY`, `YOUR_LATITUDE` and `YOUR_LONGITUDE`; replace them after importing.
- The shutdown `exec` node uses an SSH key so the command runs without a password prompt. Replace `USER@RACK_PC_IP` with the user and IP address of the computer in the rack.

### 3. Inference

MQTT decouples the sensor node from the dashboard: the STM32 only publishes, while Node-RED adds visualisation, alerting with hysteresis (to avoid reacting to a single noisy reading), automatic protective action and external weather context without any change to the firmware. Combining local alarms on the device with remote monitoring and automation gives a complete IoT monitoring system for the rack.
