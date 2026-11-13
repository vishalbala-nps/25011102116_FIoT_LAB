# TASK 10C Connect Wemos to Wi-Fi

**Experiment:** [CONTINUOUS ASSESSMENT 10 – STM32–Wemos Wi-Fi Modem Interface Using AT Commands](../README.md)

---

### 1. Hardware Used

- **Development Board:** STM32 Blue Pill
- **Microcontroller IC:** STM32F103C8T6
- **UART Pins:** PA9/PA10 (USART1, Wemos), PA2/PA3 (USART2, PC)
- **Peripherals:** Wemos D1 Mini (ESP8266), USB-to-TTL converter, Wi-Fi access point
- **Power Supply:** 5 V to the Wemos, common GND

Circuit diagram: see the [experiment README](../README.md#circuit-diagram).

### 2. Core Logic / Pseudocode / Code Base

**Working Principle:**

The STM32 configures the ESP8266 as a Wi-Fi station, joins the access point and then queries the network status. The IP and MAC addresses are extracted from the `AT+CIFSR` reply and printed on the PC terminal.

**Pseudocode:**

1. Initialize the Wi-Fi module (Task 10A).
2. Send `AT+CWMODE=1` to configure station mode.
3. Print `Connecting to Wifi...` and send `AT+CWJAP="<ssid>","<password>"`.
4. Wait for the response (`WIFI CONNECTED`, `WIFI GOT IP`, `OK`).
5. If connected:
   - print `Connected to Wifi Successfully`;
   - send `AT+CIFSR`, extract the text after `STAIP,"` and print `IP Address: <ip>`;
   - send `AT+CIFSR`, extract the text after `STAMAC,"` and print `MAC Address: <mac>`.
6. Otherwise, print `Failed to connect to wifi`.

**Code Base:**

- `initWifi()` performs the boot delay, `__HAL_UART_CLEAR_OREFLAG()`, `ATE0` and `AT`.
- `joinAP()` builds and sends the `AT+CWJAP` command.
- `getIP()` and `getMAC()` send `AT+CIFSR`, locate the `STAIP` / `STAMAC` tag with `strstr()` and copy the quoted value.

**PC Terminal Output:**

```
Initializing Wifi...
Initialized Wifi
Connecting to Wifi...
Connected to Wifi Successfully
IP Address: xxx.xxx.xxx.xxx
MAC Address: xx:xx:xx:xx:xx:xx
```

### 3. Inference

The ESP8266 joins the Wi-Fi network as a station under the control of the STM32, and the assigned IP and MAC addresses are reported on the PC terminal. This shows that the ESP8266 can act as a Wi-Fi modem for a microcontroller that only needs a UART and AT commands.
