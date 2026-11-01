# CAT 12 – BME280 Sensor over I²C with the Bosch API, Served over Wi-Fi (HTTP) via ESP8266

## Aim

Connect a Bosch BME280 environmental sensor to the NUCLEO-F401RE over I²C. Use the official **Bosch Sensortec BME280 Sensor API** to set up the sensor and get compensated temperature, pressure and humidity readings. Serve the readings over Wi-Fi: the STM32 drives an ESP8266 (Wemos D1 Mini) with AT commands as a small HTTP server, so a browser on the same network can request the current temperature.

## Hardware Setup

| Item | Details |
|---|---|
| Development board | **NUCLEO-F401RE** (STM32 Nucleo-64) |
| Microcontroller | STMicroelectronics **STM32F401RET6** (ARM Cortex-M4 with FPU, 512 KB Flash, 96 KB SRAM, LQFP64) |
| Clock source | 8 MHz clock from the on-board ST-LINK (MCO), HSE bypass |
| System clock | 84 MHz (see [Clock Configuration](#clock-configuration)) |
| Sensor | Bosch **BME280** breakout module (temperature, pressure, humidity), I²C address **0x76** (SDO to GND) |
| Wi-Fi modem | Wemos D1 Mini (ESP8266) running Espressif AT-command firmware |
| PC link | USB-to-TTL (3.3 V) serial converter to PuTTY (debug log) |
| PC terminal | PuTTY: Serial, **115200** baud, 8 data bits, no parity, 1 stop bit |
| Programmer / debugger | On-board ST-LINK/V2-1 over SWD |
| Software | STM32CubeIDE with STM32CubeMX, STM32Cube FW_F4 V1.28.3 (HAL), Bosch BME280 Sensor API v3.5.1 |

### Peripheral Roles

| Peripheral | STM32 pins | Connected to | Purpose |
|---|---|---|---|
| **I2C1** | PB6 (SCL), PB7 (SDA) | BME280 | Read and configure the sensor |
| **USART1** | PA9 (TX), PA10 (RX) | Wemos D1 Mini (ESP8266) | AT commands, Wi-Fi and the HTTP server; receive is interrupt-driven |
| **USART6** | PC6 (TX), PC7 (RX) | PC (USB-to-TTL converter) | Status messages and request log |

## Pin Configuration

| Pin | Nucleo header | Function | CubeMX signal | Mode | Pull | Speed | Connects to |
|---|---|---|---|---|---|---|---|
| **PB6** | D10 | I²C clock | `I2C1_SCL` | Alternate function, **open-drain** (`GPIO_MODE_AF_OD`, AF4) | None (`GPIO_NOPULL`) | Very high | BME280 **SCL** |
| **PB7** | Morpho CN7 | I²C data | `I2C1_SDA` | Alternate function, **open-drain** (`GPIO_MODE_AF_OD`, AF4) | None (`GPIO_NOPULL`) | Very high | BME280 **SDA** |
| **PA9** | D8 | USART1 transmit | `USART1_TX` | Alternate function, push-pull (`GPIO_MODE_AF_PP`, AF7) | None | Very high | Wemos **RX** |
| **PA10** | D2 | USART1 receive | `USART1_RX` | Alternate function, push-pull (`GPIO_MODE_AF_PP`, AF7) | None | Very high | Wemos **TX** |
| **PC6** | Morpho CN10 | USART6 transmit | `USART6_TX` | Alternate function, push-pull (`GPIO_MODE_AF_PP`, AF8) | None | Very high | Converter **RX** |
| **PC7** | D9 | USART6 receive | `USART6_RX` | Alternate function, push-pull (`GPIO_MODE_AF_PP`, AF8) | None | Very high | Converter **TX** |

I²C lines are open-drain: devices only pull them low, and resistors pull them high. The internal pulls are off here because the BME280 breakout module has its own pull-up resistors on SCL and SDA.

### I²C Configuration (I2C1)

| Parameter | Value |
|---|---|
| Mode | I²C master |
| Clock speed | **100 kHz** (standard mode) |
| Addressing | 7-bit |
| BME280 address | **0x76** (`BME280_I2C_ADDR_PRIM`, SDO tied low); 0x77 if SDO is high |

### UART Configuration

| Parameter | USART1 (ESP8266) | USART6 (PC) |
|---|---|---|
| Mode | Asynchronous, TX + RX | Asynchronous, TX + RX |
| Baud rate | **115200** | **115200** |
| Word length | 8 bits | 8 bits |
| Parity | None | None |
| Stop bits | 1 | 1 |

**Baud-rate divider:** USART1 and USART6 are both on APB2 (PCLK2 = 84 MHz). `USARTDIV = 84,000,000 / (16 × 115,200) ≈ 45.57`. HAL writes 45 + 9/16 = 45.5625, which gives ≈ **115,226 baud** (+0.02 %).

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
     |                               |                 Wemos D1 Mini (ESP8266)
     |                               |                +---------------+
     |  PA9   (USART1_TX)       [D8] |--------------->| RX            |
     |  PA10  (USART1_RX)       [D2] |<---------------| TX            |
     |  GND                          |----------------| GND           |
     |  5V                           |--------------->| 5V            |
     |                               |                +---------------+
     |                               |
     |                               |                 USB-to-TTL            PC
     |                               |                +-----------+     +----------+
     |  PC6   (USART6_TX)            |--------------->| RX        |     |  PuTTY   |
     |  PC7   (USART6_RX)       [D9] |<---------------| TX        |<===>|  115200  |
     |  GND                          |----------------| GND       | USB |  8-N-1   |
     +-------------------------------+                +-----------+     +----------+
```

- **I²C bus:** SCL and SDA run straight to the BME280 module, which has the pull-up resistors to 3.3 V. SDO to GND selects address 0x76. The module's CSB pin is tied high on the module, which selects I²C instead of SPI.
- **Serial links:** on each UART, the STM32's TX goes to the other device's RX and vice versa.
- **Common ground:** the BME280, the Wemos, the USB-to-TTL converter and the Nucleo all share GND.
- The ST-LINK, its 8 MHz clock output and the reset circuit are on the Nucleo board and are not shown.

## Code Logic

The application is in [main.c](BME280Nucleo/Core/Src/main.c). The Bosch driver files [bme280.c](BME280Nucleo/Core/Src/bme280.c), [bme280.h](BME280Nucleo/Core/Inc/bme280.h) and [bme280_defs.h](BME280Nucleo/Core/Inc/bme280_defs.h) are taken unchanged from the official BoschSensortec/BME280_SensorAPI repository (v3.5.1). The driver is built with floating-point compensation (`BME280_DOUBLE_ENABLE`).

### Bosch API ↔ STM32 HAL Glue

The Bosch API never touches hardware directly. It calls three functions that the application provides through the `struct bme280_dev` device structure:

| Bosch API needs | Implemented as | HAL call |
|---|---|---|
| Read registers | `user_i2c_read(reg_addr, reg_data, len, intf_ptr)` | `HAL_I2C_Mem_Read(&hi2c1, addr << 1, reg_addr, I2C_MEMADD_SIZE_8BIT, …)` |
| Write registers | `user_i2c_write(reg_addr, reg_data, len, intf_ptr)` | `HAL_I2C_Mem_Write(&hi2c1, addr << 1, reg_addr, I2C_MEMADD_SIZE_8BIT, …)` |
| Delay in µs | `user_delay_us(period, intf_ptr)` | `HAL_Delay(period / 1000 + 1)`, rounded up to whole milliseconds |

`intf_ptr` points to the sensor's 7-bit address (0x76). HAL expects the address shifted left by one bit. Each wrapper returns `BME280_OK` or `BME280_E_COMM_FAIL`, so I²C errors reach the Bosch API.

### Sensor Set-up (runs once)

| Step | Bosch API call | What it does |
|---|---|---|
| 1 | `dev.read / write / delay_us / intf / intf_ptr = …` | Connects the driver to the I²C wrappers above |
| 2 | `bme280_init(&dev)` | Reads the **chip ID from register 0xD0** (must be 0x60), soft-resets the sensor and reads its factory **calibration (trim) data** |
| 3 | `bme280_get_sensor_settings(&settings, &dev)` | Reads the current settings, including the standby time, as a starting point |
| 4 | `settings.osr_t / osr_p / osr_h = BME280_OVERSAMPLING_2X`, `settings.filter = BME280_FILTER_COEFF_OFF` | 2× oversampling for temperature, pressure and humidity; IIR filter off |
| 5 | `bme280_set_sensor_settings(BME280_SEL_ALL_SETTINGS, &settings, &dev)` | Writes oversampling, filter and standby time to the sensor |
| 6 | `bme280_set_sensor_mode(BME280_POWERMODE_NORMAL, &dev)` | **Normal mode:** the sensor measures continuously by itself, waiting the standby time between measurements |

If any step does not return `BME280_OK`, the program stops in `Error_Handler()`.

### Measurement

Every 2 seconds the main loop calls `bme280_get_sensor_data(BME280_ALL, &comp_data, &dev)`. The API reads the raw temperature, pressure and humidity registers and applies Bosch's compensation formulas using the calibration data:

| Value | Source | Unit |
|---|---|---|
| `temperature` | `comp_data.temperature` | °C |
| `pressure` | `comp_data.pressure / 100.0f` | hPa (the API returns Pa) |
| `humidity` | `comp_data.humidity` | % RH |

### Wi-Fi and HTTP Server

The ESP8266 is driven with the same AT-command functions as Cat10 (`sendUART()`, `recieveUART()`, `sendATCommand()`, `initWifi()`, `setMode()`, `joinAP()`, `getIP()`). Then it is set up as a TCP server on port 80:

| AT command | Purpose |
|---|---|
| `AT+CIPMUX=1` | Allow multiple connections (link IDs 0–4) |
| `AT+CIPSERVER=1,80` | Start a TCP server on port 80 (HTTP) |
| `AT+CIPSEND=<link>,<length>` | Send `<length>` bytes to a client; the modem answers `>` and then `SEND OK` |
| `AT+CIPCLOSE=<link>` | Close the client connection after the response |

**Receiving with an interrupt:** once the server is running, USART1 receives one byte at a time with `HAL_UART_Receive_IT()`. `HAL_UART_RxCpltCallback()` builds each line and reacts to the modem's messages, so requests are captured while the main loop is busy:

| Modem message | Meaning | Action |
|---|---|---|
| `<id>,CONNECT` | A client opened link `<id>` | `connections[id].state = CONNECTED` |
| `+IPD,<id>,<len>:<data>` | Request data arrived on link `<id>` | Store the request text, `state = DATA` |
| `<id>,CLOSED` | Client closed link `<id>` | `state = CLOSED` |
| `WIFI DISCONNECT` | Wi-Fi was lost | `onWifiDisconnect()`: wait for `WIFI GOT IP`, then start the server again |

**Answering requests:** `handleResponse()` checks the five links. For each one holding a request, it extracts the path after `GET`, then passes it to `handleRouting()`. That function logs `<id>: GET <path>` on the PC terminal and replies:

| Path | Response body |
|---|---|
| `/` | The current temperature (°C) |
| anything else | `Page not found` |

`sendHTML()` builds the raw HTTP response, with **Content-Length** calculated from the actual length of the body using `strlen()`:

```
HTTP/1.1 200 OK
Content-Type: text/html
Content-Length: <strlen(body)>
Connection: close

<body>
```

It then sends the response with `AT+CIPSEND=<link>,<total length>`, waits for the `>` prompt, transmits the bytes, waits for `SEND OK`, and closes the link with `AT+CIPCLOSE`.

### Program Flow

```
Start → HAL_Init → SystemClock_Config (84 MHz) → GPIO, I2C1, USART1, USART6
  → configure the Bosch API (read/write/delay functions)
  → bme280_init: chip ID 0x60? ── no ──► Error_Handler
           │ yes
  → soft reset + calibration data → oversampling ×2, filter off → normal mode
  → ESP8266: ATE0, AT, AT+CWMODE=1, AT+CWJAP → print IP on the PC terminal
  → AT+CIPMUX=1, AT+CIPSERVER=1,80, start interrupt receive
  → LOOP every 2 s:
        answer any pending HTTP requests (GET / → temperature)
        bme280_get_sensor_data → temperature, pressure (hPa), humidity
```

### Expected Output

PC terminal (USART6):

```
Initializing Wifi...
Initialized Wifi
Connecting to Wifi...
Connected to Wifi Successfully
IP Address: xxx.xxx.xxx.xxx
0: GET /
```

Browser at `http://<IP Address>/`: the current temperature in °C.

## Task Files

| File | Purpose |
|---|---|
| [`BME280Nucleo/Core/Src/main.c`](BME280Nucleo/Core/Src/main.c) | Application: the Bosch API glue, sensor set-up and reading, ESP8266 AT layer, HTTP server, and peripheral set-up |
| [`BME280Nucleo/Core/Src/bme280.c`](BME280Nucleo/Core/Src/bme280.c) | Bosch Sensortec BME280 Sensor API (v3.5.1): set-up, configuration, compensation |
| [`BME280Nucleo/Core/Inc/bme280.h`](BME280Nucleo/Core/Inc/bme280.h), [`bme280_defs.h`](BME280Nucleo/Core/Inc/bme280_defs.h) | Bosch API declarations, register definitions and data structures |
| [`BME280Nucleo/Core/Src/stm32f4xx_hal_msp.c`](BME280Nucleo/Core/Src/stm32f4xx_hal_msp.c) | MSP setup: I2C1/USART1/USART6 pin modes and clocks, USART1 interrupt enable |
| [`BME280Nucleo/Core/Src/stm32f4xx_it.c`](BME280Nucleo/Core/Src/stm32f4xx_it.c) | Interrupt handlers: SysTick and `USART1_IRQHandler` |
| [`BME280Nucleo/Core/Startup/startup_stm32f401retx.s`](BME280Nucleo/Core/Startup/startup_stm32f401retx.s) | Startup code and vector table |
| [`BME280Nucleo/BME280Nucleo.ioc`](BME280Nucleo/BME280Nucleo.ioc) | STM32CubeMX configuration: pins, I2C1, USART1, USART6, NVIC and clock tree |
| [`BME280Nucleo/STM32F401RETX_FLASH.ld`](BME280Nucleo/STM32F401RETX_FLASH.ld), [`STM32F401RETX_RAM.ld`](BME280Nucleo/STM32F401RETX_RAM.ld) | Linker scripts (Flash and RAM) |
| [`BME280Nucleo/Drivers/`](BME280Nucleo/Drivers/) | STM32F4 HAL driver and CMSIS headers |

To build, open the `BME280Nucleo` folder in STM32CubeIDE (**File → Import → Existing Projects into Workspace**). Set your Wi-Fi network name and password in the `joinAP()` call in `main()`, then choose **Project → Build Project** and flash with **Run → Debug**. Open PuTTY on the converter's COM port to see the IP address, then browse to `http://<IP Address>/` from a device on the same network.

## Inference

1. **A vendor driver saves rewriting the datasheet.** The Bosch API already contains the register map, the chip-ID check, calibration-data parsing and the compensation formulas. The application only has to supply three hardware functions (read, write, delay), which keeps the sensor code portable across microcontrollers.
2. **Raw sensor values need calibration.** The BME280's raw ADC readings mean nothing alone. Each chip stores factory trim coefficients, which `bme280_init()` reads and `bme280_get_sensor_data()` uses to compute accurate temperature, pressure and humidity.
3. **I²C uses addressed register access over two shared wires.** Every transfer starts with the device address (0x76, shifted for HAL) and a register address. Open-drain lines with pull-ups let several devices share SCL and SDA.
4. **Sensor settings trade accuracy for time and power.** Higher oversampling and the IIR filter reduce noise, but lengthen each measurement. Normal mode lets the sensor measure continuously, so the STM32 only has to read the latest result.
5. **The ESP8266 can act as a web server driven by the STM32.** With `AT+CIPMUX=1` and `AT+CIPSERVER=1,80`, the modem accepts TCP connections and passes each request to the STM32 as `+IPD` data. The STM32 decides the response and sends it back with `AT+CIPSEND`.
6. **Interrupt-driven receive captures asynchronous events.** Client connections and requests arrive at any time. Receiving USART1 bytes in `HAL_UART_RxCpltCallback()` records them as they happen, while the main loop reads the sensor and answers requests.
7. **HTTP is just text with a precise length.** A valid response needs the status line, headers, a blank line, and a `Content-Length` equal to the exact body length (`strlen()`). That length, plus the headers, is also the byte count given to `AT+CIPSEND`.
8. **Separate UARTs keep the roles clear.** USART1 carries only modem traffic and USART6 only the human-readable log, so debugging output never corrupts the AT-command conversation.
