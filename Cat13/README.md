# CAT 13 – MQTT Publishing of BME280 Sensor Data from STM32 via ESP8266

## Aim

Read compensated temperature, pressure and humidity from a Bosch BME280 sensor on the NUCLEO-F401RE over I²C, and publish them once a second as a JSON message to a public **MQTT broker** over Wi-Fi. The STM32 builds the MQTT packets itself and sends them over a TCP connection opened by an ESP8266 (Wemos D1 Mini) using AT commands. Any MQTT client subscribed to the topic receives the live readings.

## Hardware Setup

| Item | Details |
|---|---|
| Development board | **NUCLEO-F401RE** (STM32 Nucleo-64) |
| Microcontroller | STMicroelectronics **STM32F401RET6** (ARM Cortex-M4 with FPU, 512 KB Flash, 96 KB SRAM, LQFP64) |
| Clock source | 8 MHz clock from the on-board ST-LINK (MCO), HSE bypass |
| System clock | 84 MHz (see [Clock Configuration](#clock-configuration)) |
| Sensor | Bosch **BME280** breakout module, I²C address **0x76** (SDO to GND) |
| Wi-Fi modem | Wemos D1 Mini (ESP8266) running Espressif AT-command firmware |
| MQTT broker | Public EMQX broker **`broker.emqx.io`**, TCP port **1883** |
| Programmer / debugger | On-board ST-LINK/V2-1 over SWD |
| Software | STM32CubeIDE with STM32CubeMX, STM32Cube FW_F4 (HAL), Bosch BME280 Sensor API v3.5.1, ESPLib (ESP8266 + MQTT helper library) |

### Peripheral Roles

| Peripheral | STM32 pins | Connected to | Purpose |
|---|---|---|---|
| **I2C1** | PB6 (SCL), PB7 (SDA) | BME280 | Read and configure the sensor |
| **USART1** | PA9 (TX), PA10 (RX) | Wemos D1 Mini (ESP8266) | AT commands, TCP link to the broker, MQTT packets |
| **USART6** | PC6 (TX), PC7 (RX) | PC (USB-to-TTL converter) | Debug UART, set up in CubeMX; this program sends no messages on it |

## Pin Configuration

| Pin | Nucleo header | Function | CubeMX signal | Mode | Pull | Connects to |
|---|---|---|---|---|---|---|
| **PB6** | D10 | I²C clock | `I2C1_SCL` | Alternate function, **open-drain** (`GPIO_MODE_AF_OD`, AF4) | None | BME280 **SCL** |
| **PB7** | Morpho CN7 | I²C data | `I2C1_SDA` | Alternate function, **open-drain** (`GPIO_MODE_AF_OD`, AF4) | None | BME280 **SDA** |
| **PA9** | D8 | USART1 transmit | `USART1_TX` | Alternate function, push-pull (`GPIO_MODE_AF_PP`, AF7) | None | Wemos **RX** |
| **PA10** | D2 | USART1 receive | `USART1_RX` | Alternate function, push-pull (`GPIO_MODE_AF_PP`, AF7) | None | Wemos **TX** |
| **PC6** | Morpho CN10 | USART6 transmit | `USART6_TX` | Alternate function, push-pull (`GPIO_MODE_AF_PP`, AF8) | None | Converter **RX** |
| **PC7** | D9 | USART6 receive | `USART6_RX` | Alternate function, push-pull (`GPIO_MODE_AF_PP`, AF8) | None | Converter **TX** |

The I²C lines use the BME280 module's own pull-up resistors, so the internal pulls are off.

### I²C Configuration (I2C1)

| Parameter | Value |
|---|---|
| Mode | I²C master |
| Clock speed | **100 kHz** (standard mode) |
| Addressing | 7-bit |
| BME280 address | **0x76** (`BME280_I2C_ADDR_PRIM`, SDO tied low) |

### UART Configuration

| Parameter | USART1 (ESP8266) | USART6 (PC) |
|---|---|---|
| Mode | Asynchronous, TX + RX | Asynchronous, TX + RX |
| Baud rate | **115200** | **115200** |
| Word length | 8 bits | 8 bits |
| Parity | None | None |
| Stop bits | 1 | 1 |

**Baud-rate divider:** USART1 and USART6 are on APB2 (PCLK2 = 84 MHz). `USARTDIV = 84,000,000 / (16 × 115,200) ≈ 45.57`. HAL writes 45 + 9/16 = 45.5625, which gives ≈ **115,226 baud** (+0.02 %).

## Clock Configuration

```
 ST-LINK MCO 8 MHz ──► HSE (bypass) ──► PLLM ÷4 ──► 2 MHz ──► PLLN ×84 ──► VCO 168 MHz ──► PLLP ÷2 ──► SYSCLK 84 MHz
                                                  SYSCLK 84 MHz
                                                        │
                                               AHB ÷1 ──┴──► HCLK 84 MHz ──► Cortex-M4 core, SysTick, memory, GPIO
                                                        │
                                          ┌─────────────┴──────────────┐
                                    APB1 ÷2                       APB2 ÷1
                               PCLK1 42 MHz                  PCLK2 84 MHz
                                  (I2C1)                   (USART1, USART6)
```

## Circuit Diagram

```
                                                        BME280 module
       STM32F401RET6 (NUCLEO-F401RE)                  +---------------+
     +-------------------------------+                |               |
     |  PB6  (I2C1_SCL, AF OD)  [D10]|--------------->| SCL           |
     |  PB7  (I2C1_SDA, AF OD)       |<-------------->| SDA           |
     |                               |                | SDO --- GND   |  (address 0x76)
     |  3V3                          |--------------->| VCC           |
     |  GND                          |----------------| GND           |
     |                               |                +---------------+
     |                               |
     |                               |                 Wemos D1 Mini (ESP8266)          Wi-Fi
     |                               |                +---------------+              ┌──────────────────┐
     |  PA9   (USART1_TX)       [D8] |--------------->| RX            |   TCP 1883   │  broker.emqx.io  │
     |  PA10  (USART1_RX)       [D2] |<---------------| TX            |<- - - - - - >│   (MQTT broker)  │
     |  GND                          |----------------| GND           |              └────────┬─────────┘
     |  5V                           |--------------->| 5V            |                       │ subscribe
     |                               |                +---------------+              stm32/bme280 clients
     +-------------------------------+                                              (MQTTX, mosquitto_sub, Node-RED)
```

- **I²C bus:** SCL and SDA connect directly to the BME280 module, which has the pull-ups. SDO to GND selects address 0x76.
- **Serial link:** the STM32's TX goes to the Wemos RX and the Wemos TX goes to the STM32's RX. The two boards share GND, and the Wemos is powered from 5 V.
- **Network path:** the ESP8266 joins the Wi-Fi network and opens a TCP connection to the broker. Subscribers anywhere on the internet receive the published messages through the broker.
- The ST-LINK, its 8 MHz clock output and the reset circuit are on the Nucleo board and are not shown.

## Code Logic

The application is in [main.c](NucleoMQTT/Core/Src/main.c). It uses two libraries:
- **Bosch BME280 Sensor API v3.5.1:** [bme280.c](NucleoMQTT/Core/Src/bme280.c), [bme280.h](NucleoMQTT/Core/Inc/bme280.h), [bme280_defs.h](NucleoMQTT/Core/Inc/bme280_defs.h). It is unchanged and uses floating-point compensation.
- **ESPLib:** [ESPLib.c](NucleoMQTT/Core/Src/ESPLib.c), [ESPLib.h](NucleoMQTT/Core/Inc/ESPLib.h), an ESP8266 AT-command and MQTT helper library by Can Güveren (2022). Its global variables are defined in `ESPLib.c` so it links in this project.

### Start-up Sequence

| Step | Call | What happens on the link |
|---|---|---|
| 1 | `ESP8266_Init(StationMode, huart1)` | `AT` (repeated until `OK`), then `AT+CWMODE=1`: station mode |
| 2 | `ESP8266_wifiConnect(ssid, password)` | `AT+CWQAP` (leave any network), then `AT+CWJAP="<ssid>","<password>"` until `WIFI CONNECTED` / `OK` |
| 3 | `ESP8266_portConnect("TCP", "broker.emqx.io", "1883")` | `AT+CIPCLOSE`, then `AT+CIPSTART="TCP","broker.emqx.io",1883`: TCP connection to the broker |
| 4 | `MQTT_connectBroker(&mqttSetting)` | Sends an MQTT **CONNECT** packet: client ID `STM32_Publisher_vishalbala`, clean session, keep-alive 500 s |
| 5 | `bme280_init()` → `bme280_set_sensor_settings()` → `bme280_set_sensor_mode()` | Chip-ID check (0x60) and calibration data; 2× oversampling for T/P/H, filter off; **normal mode** |

Each ESPLib call repeats its AT command until the modem returns the expected response. Every MQTT packet is sent with `ESP8266_sendMessage()`: `AT+CIPSEND=<n>`, wait for the `>` prompt, then send the `n` packet bytes over the open TCP connection.

### Main Loop (every 1 s)

```
bme280_get_sensor_data(BME280_ALL) → temperature (°C), pressure (Pa → hPa), humidity (%RH)
snprintf → {"t":28.4,"p":1008.7,"h":62.3}
MQTT_publishTopic("stm32/bme280", json, QoS0)
HAL_Delay(1000)
```

### MQTT Packets Built by the STM32

MQTT is a binary protocol. Every packet starts with a fixed header: a type/flags byte, then the **remaining length**, the number of bytes that follow. ESPLib builds the packets byte by byte:

| Packet | First byte | Contents after the remaining length |
|---|---|---|
| **CONNECT** | `0x10` | Protocol name length (`0x00 0x04`) + `"MQTT"`, protocol level `0x04` (MQTT 3.1.1), connect flags (`0x02` = clean session), keep-alive (2 bytes, 500 s), client ID length (2 bytes) + client ID |
| **PUBLISH** (QoS 0) | `0x30` | Topic length (2 bytes) + topic `stm32/bme280` + the JSON message (QoS 0 has no packet ID) |

For example, publishing `{"t":28.4,"p":1008.7,"h":62.3}` (30 bytes) to `stm32/bme280` (12 bytes) gives a remaining length of 2 + 12 + 30 = 44 bytes and a 46-byte packet in total.

### Published Data

| Field | Topic | Value |
|---|---|---|
| Broker | — | `broker.emqx.io:1883` |
| Topic | `stm32/bme280` | — |
| Payload | — | `{"t":<temperature °C>,"p":<pressure hPa>,"h":<humidity %RH>}`, one decimal place each |
| QoS | — | 0 (at most once) |
| Rate | — | About one message per second |

### Viewing the Data

Subscribe to the topic with any MQTT client, for example:

```
mosquitto_sub -h broker.emqx.io -p 1883 -t stm32/bme280
```

Expected output, one line per second:

```
{"t":28.4,"p":1008.7,"h":62.3}
{"t":28.4,"p":1008.6,"h":62.4}
...
```

The same topic can be used as the input of a Node-RED `mqtt in` node, to chart the readings or compute minimum, maximum and average statistics.

### Pseudocode

```
MAIN:
    initialise HAL, clock (84 MHz), GPIO, I2C1, USART1, USART6
    ESP8266: AT → AT+CWMODE=1 → AT+CWQAP → AT+CWJAP="<ssid>","<password>"
    ESP8266: AT+CIPSTART="TCP","broker.emqx.io",1883
    send MQTT CONNECT (client ID, clean session, keep-alive 500 s)
    BME280: connect the I²C read/write/delay functions, init (chip ID + calibration),
            oversampling ×2, filter off, normal mode
    LOOP forever:
        read compensated T, P, H from the BME280
        IF read OK:
            json <- {"t":T,"p":P/100,"h":H}
            send MQTT PUBLISH to "stm32/bme280" with QoS 0
        wait 1 s
```

## Task Files

| File | Purpose |
|---|---|
| [`NucleoMQTT/Core/Src/main.c`](NucleoMQTT/Core/Src/main.c) | Application: Wi-Fi and broker connection, BME280 set-up, the read-and-publish loop, Bosch I²C glue functions, peripheral set-up |
| [`NucleoMQTT/Core/Src/ESPLib.c`](NucleoMQTT/Core/Src/ESPLib.c), [`Core/Inc/ESPLib.h`](NucleoMQTT/Core/Inc/ESPLib.h) | ESP8266 AT-command driver and MQTT packet builder (CONNECT, PUBLISH, SUBSCRIBE, PINGREQ, DISCONNECT) |
| [`NucleoMQTT/Core/Src/bme280.c`](NucleoMQTT/Core/Src/bme280.c), [`Core/Inc/bme280.h`](NucleoMQTT/Core/Inc/bme280.h), [`bme280_defs.h`](NucleoMQTT/Core/Inc/bme280_defs.h) | Bosch Sensortec BME280 Sensor API v3.5.1 |
| [`NucleoMQTT/Core/Src/stm32f4xx_hal_msp.c`](NucleoMQTT/Core/Src/stm32f4xx_hal_msp.c) | MSP setup: I2C1/USART1/USART6 pin modes and clocks, USART1 interrupt enable |
| [`NucleoMQTT/Core/Src/stm32f4xx_it.c`](NucleoMQTT/Core/Src/stm32f4xx_it.c) | Interrupt handlers: SysTick and `USART1_IRQHandler` |
| [`NucleoMQTT/Core/Startup/startup_stm32f401retx.s`](NucleoMQTT/Core/Startup/startup_stm32f401retx.s) | Startup code and vector table |
| [`NucleoMQTT/NucleoMQTT.ioc`](NucleoMQTT/NucleoMQTT.ioc) | STM32CubeMX configuration: pins, I2C1, USART1, USART6, NVIC and clock tree |
| [`NucleoMQTT/STM32F401RETX_FLASH.ld`](NucleoMQTT/STM32F401RETX_FLASH.ld), [`STM32F401RETX_RAM.ld`](NucleoMQTT/STM32F401RETX_RAM.ld) | Linker scripts (Flash and RAM) |
| [`NucleoMQTT/Drivers/`](NucleoMQTT/Drivers/) | STM32F4 HAL driver and CMSIS headers |

To build, open the `NucleoMQTT` folder in STM32CubeIDE (**File → Import → Existing Projects into Workspace**). Set your Wi-Fi network name and password in the `ESP8266_wifiConnect()` call in `main()`, then choose **Project → Build Project** and flash with **Run → Debug**. The project links newlib-nano with float `printf` support, which `snprintf("%.1f")` needs.

## Inference

1. **MQTT decouples the sensor from its consumers.** The STM32 only publishes to a topic on the broker. It doesn't know or care who is listening. Any number of dashboards, loggers or Node-RED flows can subscribe to `stm32/bme280` without any change to the firmware.
2. **MQTT runs on top of TCP.** The ESP8266 provides the network and the TCP connection (`AT+CIPSTART`), and the STM32 sends MQTT packets as raw bytes through that connection (`AT+CIPSEND`). This shows the layering: Wi-Fi → IP → TCP → MQTT → JSON.
3. **MQTT packets are compact binary frames.** One type byte, a remaining-length byte and length-prefixed strings make the fixed header only two bytes. Building CONNECT and PUBLISH by hand shows exactly what a client library does internally.
4. **The broker session starts with CONNECT.** The client ID identifies the device, the clean-session flag discards old state, and the keep-alive tells the broker how long to wait between packets before treating the client as gone. Publishing every second keeps the connection alive.
5. **QoS 0 suits periodic sensor data.** With "at most once" delivery there is no acknowledgement or retransmission. A lost reading is simply replaced by the next one a second later, which keeps the firmware and the link light.
6. **JSON makes the payload self-describing.** `{"t":…,"p":…,"h":…}` can be parsed directly by Node-RED, web dashboards or scripts, at the cost of a few extra bytes over a raw binary format.
7. **Reusing proven drivers speeds up integration.** The Bosch API handles the sensor and ESPLib handles the modem and MQTT framing. The application code is reduced to set-up and one short read-and-publish loop.
