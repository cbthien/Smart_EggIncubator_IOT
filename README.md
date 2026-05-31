# Smart Egg Incubator IoT

IoT-based automatic egg incubator system powered by **ESP32-S3** with MQTT remote monitoring, automatic tray turning, temperature & humidity control, PWM fan speed control, and multi-layer safety protection.

---

## Overview

Smart Egg Incubator IoT is a production-ready intelligent egg incubation system featuring:

- Automatic temperature regulation with hysteresis control
- Real-time humidity monitoring
- Automatic egg tray turning (configurable interval)
- Remote monitoring & control via MQTT
- Real-time telemetry (JSON over MQTT)
- Multi-layer safety protection system
- WiFi provisioning via captive portal (no hardcoded credentials)
- OTA firmware update (upload via WiFi)
- Incubation day counter
- Fan RPM monitoring with stall detection

---

## Hardware Components

### Main Controller

| Device | Description |
|--------|-------------|
| ESP32-S3 Dev Module | Main MCU with WiFi, dual-core, 240MHz |

### Power Supply

| Device | Description |
|--------|-------------|
| 12V 15A 180W PSU | Main power for heaters, fans, motor |
| XL4016 Buck Converter | 12V → 5V for ESP32 logic |

### Sensors

| Device | Description |
|--------|-------------|
| SHT30 | High precision I2C temperature & humidity sensor |

### Heating System

| Device | Qty | Description |
|--------|-----|-------------|
| 12V Carbon Heater Wire | 2 | Left + Right heating zones |

### Cooling & Air Circulation

| Device | Qty | Description |
|--------|-----|-------------|
| Cooler Master DF1202512B2UN | 2 | 12V 4-pin PWM fans with Tach feedback |

### Egg Tray Turning System

| Device | Qty | Description |
|--------|-----|-------------|
| DTY50 220VAC Motor | 1 | Continuous rotation motor for tray turning |
| KW12 Limit Switch | 2 | Detects left/right tray position |

### Relay Module

| Device | Description |
|--------|-------------|
| 4CH Relay Module 12V | Opto-isolated, LOW trigger |

### Relay Channel Assignment

| Channel | Function | Load |
|---------|----------|------|
| CH1 (GPIO 4) | Heater Left | 12V DC |
| CH2 (GPIO 5) | Heater Right | 12V DC |
| CH3 (GPIO 6) | DTY50 Motor | 220V AC |
| CH4 (GPIO 7) | Reserved | — |

---

## GPIO Pin Map

| GPIO | Function | Type |
|------|----------|------|
| 4 | Relay CH1 - Heater Left | OUTPUT |
| 5 | Relay CH2 - Heater Right | OUTPUT |
| 6 | Relay CH3 - DTY50 Motor | OUTPUT |
| 7 | Relay CH4 - Reserved | OUTPUT |
| 8 | SHT30 SDA | I2C |
| 9 | SHT30 SCL | I2C |
| 11 | Fan Left Tach (RPM) | INPUT (interrupt) |
| 12 | Fan Left PWM | OUTPUT (25kHz) |
| 13 | Fan Right Tach (RPM) | INPUT (interrupt) |
| 14 | Fan Right PWM | OUTPUT (25kHz) |
| 17 | Left Limit Switch | INPUT_PULLUP |
| 18 | Right Limit Switch | INPUT_PULLUP |

---

## IoT Communication

### MQTT Topics

| Topic | Direction | Description |
|-------|-----------|-------------|
| `egg_incubator/telemetry` | ESP32 → Cloud | System state every 30s |
| `egg_incubator/command` | Cloud → ESP32 | Remote commands |
| `egg_incubator/alarm` | ESP32 → Cloud | Safety alarms |
| `egg_incubator/status` | ESP32 → Cloud | Online/offline status |

### MQTT Broker

Default: `broker.hivemq.com:1883` (free, public, no auth)

---

## How to Connect MQTT

### Option 1: MQTTX Desktop App (Recommended)

1. Download: https://mqttx.app/
2. Create new connection:
   - **Host:** `broker.hivemq.com`
   - **Port:** `1883`
   - **MQTT Version:** `3.1.1`
   - Username/Password: leave empty
3. Click **Connect**
4. Subscribe to `egg_incubator/telemetry` to see data
5. Publish to `egg_incubator/command` to send commands

### Option 2: HiveMQ Web Client (No install)

1. Open: http://www.hivemq.com/demos/websocket-client/
2. Click **Connect** (default settings are correct)
3. Add subscription: `egg_incubator/telemetry`
4. To send command: publish to `egg_incubator/command`

### Option 3: Any MQTT Client

Any MQTT client that supports TCP on port 1883 will work:
- MQTT Explorer
- Home Assistant
- Node-RED
- Custom mobile app

---

## WiFi Setup (First Time)

The device uses **WiFiManager** for WiFi provisioning. No hardcoded SSID/password.

### Steps:

1. **Power on ESP32** — first boot or after WiFi reset
2. ESP32 creates a WiFi access point named: **`EggIncubator_Setup`**
3. On your phone/laptop:
   - Go to WiFi settings
   - Connect to **`EggIncubator_Setup`** (open network, no password)
4. A captive portal page will open automatically
   - If it doesn't open: open browser → go to **`192.168.4.1`**
5. Click **"Configure WiFi"**
6. Select your home WiFi network from the list
7. Enter WiFi password → click **Save**
8. ESP32 will restart and connect to your WiFi
9. Check Serial Monitor for the assigned IP address

### Changing WiFi Later:

Send MQTT command:
```json
{"cmd": "reset_wifi"}
```
ESP32 will clear saved WiFi and reboot into AP mode again.

### AP Timeout:

If no one connects within **2 minutes**, the AP closes and ESP32 continues without WiFi. Reset the device to try again.

---

## MQTT Commands

Send JSON to topic: `egg_incubator/command`

### Temperature Control

```json
{"cmd": "set_temp", "value": 37.8}
```
Set target temperature (30.0 - 40.0°C)

### Heater Control

```json
{"cmd": "heater_off"}
{"cmd": "heater_auto"}
```

### Fan Control

```json
{"cmd": "set_fan_speed", "value": 75}
```
Set fan speed (0-100%)

```json
{"cmd": "disable_fan_check"}
{"cmd": "enable_fan_check"}
```
Disable/enable fan stall detection (useful for testing without fans)

### Tray Control

```json
{"cmd": "turn_tray"}
{"cmd": "stop_auto_turn"}
{"cmd": "start_auto_turn"}
```

### Incubation Counter

```json
{"cmd": "start_incubation"}
{"cmd": "reset_incubation"}
```

### System

```json
{"cmd": "reset_wifi"}
{"cmd": "reboot"}
```

---

## Telemetry JSON Format

Published every 30 seconds to `egg_incubator/telemetry`:

```json
{
  "temperature": 37.5,
  "humidity": 60.2,
  "sensor_ok": true,
  "heater_left": true,
  "heater_right": true,
  "fan_left_rpm": 1200,
  "fan_right_rpm": 1180,
  "fan_left_speed": 255,
  "fan_right_speed": 255,
  "fan_left_ok": true,
  "fan_right_ok": true,
  "fan_check_enabled": true,
  "tray_position": "LEFT",
  "tray_turning": false,
  "auto_turn_enabled": true,
  "incubation_day": 5,
  "incubation_active": true,
  "wifi": true,
  "mqtt": true,
  "safety": "OK",
  "error": "NONE",
  "uptime_s": 3600,
  "target_temp": 37.5,
  "fan_speed_set": 255
}
```

---

## Alarm JSON Format

Published to `egg_incubator/alarm` when safety event occurs:

```json
{
  "alarm": "OVER_TEMP",
  "temperature": 39.8,
  "humidity": 55.0,
  "uptime_s": 12345
}
```

### Alarm Types:

| Alarm | Description | Action |
|-------|-------------|--------|
| `SENSOR_FAIL` | SHT30 read error or not found | Heater OFF |
| `OVER_TEMP` | Temperature ≥ 39.5°C | Heater OFF |
| `FAN_STALL` | Fan RPM below threshold | Heater OFF |
| `MOTOR_TIMEOUT` | Tray motor exceeded 60s | Motor OFF |
| `BOTH_LIMIT_ACTIVE` | Both limit switches active | Motor OFF |

---

## Safety System

Safety has **highest priority** and overrides all other controls:

1. **Over-temperature** (≥ 39.5°C) → Heater OFF immediately
2. **Sensor failure** (NaN, not found) → Heater OFF immediately
3. **Fan stall** (RPM < 100 while heater ON) → Heater OFF
4. **Motor timeout** (> 60s without reaching limit) → Motor OFF
5. **Dual limit error** (both switches active) → Motor OFF

Safety publishes alarm to MQTT for webapp notification.

---

## OTA Firmware Update

Update firmware over WiFi without USB cable:

1. ESP32 must be connected to same WiFi as your computer
2. In Arduino IDE / Visual Studio: select network port "EggIncubator"
3. Upload as normal
4. Password: `egg12345`

---

## Software Architecture

```
SmartEggIncubator/
├── SmartEggIncubator.ino          # Entry point
├── src/
│   ├── config/
│   │   ├── Config.h               # All configuration constants
│   │   └── Pins.h                 # GPIO pin definitions
│   ├── core/
│   │   ├── AppContext.h           # Shared state container
│   │   ├── IncubatorApp.h/.cpp    # Main orchestrator
│   │   └── Models.h              # Data structures & enums
│   ├── services/
│   │   ├── fan/                   # PWM fan control + RPM monitoring
│   │   ├── heater/               # Dual heater hysteresis control
│   │   ├── mqtt/                  # MQTT client (PubSubClient)
│   │   ├── safety/               # Multi-layer safety system
│   │   ├── sensor/               # SHT30 I2C sensor
│   │   ├── storage/              # NVS persistent settings
│   │   ├── telemetry/            # JSON telemetry builder
│   │   ├── tray/                 # Motor + limit switch control
│   │   └── wifi/                 # WiFiManager provisioning
│   ├── packets/                   # JSON format documentation
│   └── utils/                     # Logger, TimeUtils, JsonUtils
```

---

## Development Environment

| Tool | Version |
|------|---------|
| IDE | Visual Studio 2022 |
| Plugin | Visual Micro |
| Framework | Arduino |
| Board | ESP32-S3 Dev Module |
| ESP32 Core | 3.3.8 |

### Required Libraries

| Library | Version | Purpose |
|---------|---------|---------|
| Adafruit SHT31 | 2.2.2 | Temperature/humidity sensor |
| ArduinoJson | 7.4.3 | JSON serialization |
| PubSubClient | 2.8.0 | MQTT client |
| WiFiManager | 2.0.17 | WiFi provisioning |

---

## Quick Start

1. Clone repository
2. Open in Visual Studio with Visual Micro
3. Install required libraries
4. Select board: ESP32S3 Dev Module
5. Build & Upload
6. Connect phone to "EggIncubator_Setup" WiFi
7. Configure WiFi via 192.168.4.1
8. Open MQTTX → connect to broker.hivemq.com
9. Subscribe to `egg_incubator/telemetry`
10. Send commands to `egg_incubator/command`

---

## License

This project is open-source and available under the MIT License.

---

## Author

Developed by **Cao Thien**

GitHub: https://github.com/cbthien
