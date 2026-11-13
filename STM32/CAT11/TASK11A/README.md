# TASK 11A Configure ESP8266 as a TCP Server

**Experiment:** [CONTINUOUS ASSESSMENT 11 – HTTP Server - STM32–Wemos Wi-Fi Modem Using AT Commands](../README.md)

---

### 1. Hardware Used

- **Development Board:** STM32 Blue Pill
- **Microcontroller IC:** STM32F103C8T6 (ARM Cortex-M3, 72 MHz)
- **UART1 Pins (Wemos):** PA9 (USART1_TX) → Wemos RX, PA10 (USART1_RX) ← Wemos TX
- **UART2 Pins (PC):** PA2 (USART2_TX) → converter RX, PA3 (USART2_RX) ← converter TX
- **Peripherals:** Wemos D1 Mini (ESP8266) running AT firmware, USB-to-TTL (3.3 V) converter to PuTTY, Wi-Fi access point
- **Power Supply:** Wemos powered from 5 V; STM32, Wemos and converter share a common GND

Circuit diagram: see the [experiment README](../README.md#circuit-diagram).

### 2. Core Logic / Pseudocode / Code Base

**Working Principle:**

The STM32 first configures the ESP8266 as a Wi-Fi station and connects it to the network, then configures it as a TCP server with the three commands of Task 11A: `AT` (check the modem), `AT+CIPMUX=1` (allow multiple connections) and `AT+CIPSERVER=1,80` (listen on port 80). Every command and the ESP8266 response are shown on the PC terminal.

**Pseudocode:**

1. Initialize HAL, the system clock, USART1 (Wemos) and USART2 (PC).
2. Wait for the ESP8266 to boot, turn echo off (`ATE0`) and check it with `AT`.
3. Set station mode (`AT+CWMODE=1`) and join the access point (`AT+CWJAP="<ssid>","<password>"`).
4. Read the IP and MAC address with `AT+CIFSR` and print them.
5. Send `AT`, `AT+CIPMUX=1` and `AT+CIPSERVER=1,80`, printing each command as `>> <command>` and each response line as `<< <line>`.
6. Report whether the TCP server started on port 80.

**Code Base:**

- `initWifi()`, `setMode()`, `joinAP()`, `getIP()` and `getMAC()` connect the Wemos to Wi-Fi and read its addresses (from the ESP8266 functions of the BME280 HTTP experiment).
- `sendATCommand()` sends a command and collects the response until `OK`, `ERROR` or `FAIL`.
- `showATCommand()` prints the command and its response on the PC terminal.

**PC Terminal Output:**

```
Initializing Wifi...
Initialized Wifi
Connecting to Wifi...
Connected to Wifi Successfully
IP Address: xxx.xxx.xxx.xxx
MAC Address: xx:xx:xx:xx:xx:xx
>> AT
<< OK
>> AT+CIPMUX=1
<< OK
>> AT+CIPSERVER=1,80
<< OK
TCP server started on port 80
```

### 3. Inference

The ESP8266 joins the Wi-Fi network as a station and, with `AT+CIPMUX=1` and `AT+CIPSERVER=1,80`, accepts TCP connections on port 80. Configuring the Wi-Fi modem as a server needs only AT commands sent over the UART.
