#  Smart Egg Incubator IoT

IoT-based automatic egg incubator system powered by ESP32-S3 with MQTT remote monitoring, automatic tray turning, temperature & humidity control, and safety protection features.

---

#  Overview

Smart Egg Incubator IoT is a custom-built intelligent egg incubation system designed for:

* Automatic temperature regulation
* Humidity monitoring
* Automatic egg tray turning
* Remote monitoring via MQTT
* Real-time telemetry
* Safety protection system
* ESP32-S3 WiFi connectivity

The project combines embedded systems, IoT communication, and custom hardware control into a complete incubation automation platform.

---

#  Hardware Components

##  Main Controller

| Device   | Description                                   |
| -------- | --------------------------------------------- |
| ESP32-S3 | Main microcontroller with WiFi & MQTT support |

---

##  Sensors

| Device | Description                                  |
| ------ | -------------------------------------------- |
| SHT30  | High precision temperature & humidity sensor |

---

##  Heating System

| Device                 | Description                             |
| ---------------------- | --------------------------------------- |
| 12V Carbon Heater Wire | Generates heat inside incubator chamber |

---

##  Cooling & Air Circulation

| Device                         | Description                              |
| ------------------------------ | ---------------------------------------- |
| Cooler Master DF1202512B2UN x2 | 12V cooling fans for airflow circulation |

---

##  Egg Tray Turning System

| Device               | Description                              |
| -------------------- | ---------------------------------------- |
| TDY50 Motor          | Automatic tray turning motor             |
| KW12 Limit Switch x2 | Detects tray direction and stop position |

---

##  Custom PCB

Custom relay & driver board used for:

* Switching 12V heater
* Controlling 12V fans
* Controlling tray motor
* Isolating ESP32 GPIO from high current loads

ESP32 only sends logic-level control signals while the custom board handles the actual 12V power switching.

---

#  IoT Features

## MQTT Communication

Supports:

* Remote monitoring
* Remote control
* Telemetry upload
* Device status reporting
* Sensor data streaming

Possible integrations:

* Home Assistant
* Node-RED
* Mobile App
* Custom Dashboard

---

#  Software Architecture

Project structure:

```text
src/
├── config/
├── core/
├── packets/
├── services/
│   ├── fan/
│   ├── heater/
│   ├── mqtt/
│   ├── safety/
│   ├── sensor/
│   ├── storage/
│   ├── telemetry/
│   ├── tray/
│   └── wifi/
└── utils/
```

---

# 🔧 Main Services

| Service          | Responsibility              |
| ---------------- | --------------------------- |
| SensorService    | Read SHT30 data             |
| HeaterService    | Heater control              |
| FanService       | Fan control                 |
| TrayService      | Egg tray turning logic      |
| SafetyService    | Overheat & fault protection |
| WiFiService      | WiFi connection             |
| MqttService      | MQTT communication          |
| TelemetryService | Send telemetry data         |
| StorageService   | Persistent configuration    |

---

#  Safety Features

* Overheat protection
* Sensor fault detection
* Tray motor timeout protection
* Limit switch validation
* Automatic recovery logic

---

# 📡 Telemetry Example

```json
{
  "temperature": 37.8,
  "humidity": 58.4,
  "heater": true,
  "fans": true,
  "trayPosition": "LEFT",
  "wifi": true
}
```

---

#  Planned Features

* PID temperature control
* OLED/LCD local display
* Mobile application
* OTA firmware update
* Historical incubation data
* AI incubation optimization
* Automatic humidity control
* Web dashboard

---

#  Tech Stack

| Technology        | Usage                 |
| ----------------- | --------------------- |
| ESP32-S3          | Main controller       |
| Arduino Framework | Firmware development  |
| MQTT              | IoT communication     |
| C++               | Embedded software     |
| WiFi              | Remote connectivity   |
| Custom PCB        | Power & relay control |

---

#  Future Improvements

* Camera monitoring
* Hatch prediction system
* AI analytics
* Cloud synchronization
* Battery backup system

---

#  License

This project is open-source and available under the MIT License.

---

# 👨‍💻 Author

Developed by CaoThien

GitHub:
https://github.com/cbthien
