/******************************************************************
 * Smart Egg Incubator - Configuration
 * All timing, thresholds, and network settings
 ******************************************************************/

#ifndef CONFIG_H
#define CONFIG_H

// ============================================================
// SENSOR
// ============================================================
#define SENSOR_READ_INTERVAL_MS     2000    // Read SHT30 every 2s
#define SENSOR_I2C_ADDRESS          0x44    // SHT30 default address

// ============================================================
// HEATER - Temperature Control
// ============================================================
#define DEFAULT_TARGET_TEMP         37.5f   // Target temperature (°C)
#define DEFAULT_HYSTERESIS          0.2f    // Hysteresis band (±°C)
#define DEFAULT_MAX_TEMP            39.5f   // Safety cutoff temperature (°C)

// ============================================================
// FAN - PWM Configuration
// ============================================================
#define FAN_PWM_FREQUENCY           25000   // 25kHz (standard 4-pin fan)
#define FAN_PWM_RESOLUTION          8       // 8-bit (0-255)
#define FAN_PWM_CHANNEL_LEFT        0       // LEDC channel for left fan
#define FAN_PWM_CHANNEL_RIGHT       1       // LEDC channel for right fan
#define DEFAULT_FAN_SPEED           255     // Default speed (0-255, 255=100%)
#define FAN_TACH_PULSES_PER_REV     2       // Typical: 2 pulses per revolution
#define FAN_RPM_READ_INTERVAL_MS    1000    // Read RPM every 1s
#define FAN_STALL_THRESHOLD_RPM     300     // Below this = fan stalled

// ============================================================
// TRAY - Motor Control
// ============================================================
#define DEFAULT_TURN_INTERVAL_MS    (2UL * 60UL * 60UL * 1000UL)  // 2 hours
#define DEFAULT_MOTOR_TIMEOUT_MS    45000   // 45s timeout (safety)

// ============================================================
// TELEMETRY
// ============================================================
#define TELEMETRY_INTERVAL_MS       30000   // Publish telemetry every 30s (sufficient for incubator)
#define SERIAL_TELEMETRY_ENABLED    true    // Print to Serial

// ============================================================
// MQTT
// ============================================================
#define MQTT_BROKER                 "broker.hivemq.com"
#define MQTT_PORT                   1883
#define MQTT_CLIENT_ID_PREFIX       "EggIncubator_"
#define MQTT_TOPIC_TELEMETRY        "egg_incubator/telemetry"
#define MQTT_TOPIC_STATUS           "egg_incubator/status"
#define MQTT_TOPIC_COMMAND          "egg_incubator/command"
#define MQTT_TOPIC_ALARM            "egg_incubator/alarm"
#define MQTT_RECONNECT_INTERVAL_MS  5000    // Retry every 5s

// ============================================================
// WIFI
// ============================================================
#define WIFI_AP_NAME                "EggIncubator_Setup"
#define WIFI_AP_PASSWORD            ""      // Open AP for setup
#define WIFI_CONNECT_TIMEOUT_S      120     // Timeout before closing AP (2 min)
#define WIFI_RECONNECT_INTERVAL_MS  10000   // Retry every 10s

// ============================================================
// OTA - Over The Air Update
// ============================================================
#define OTA_HOSTNAME                "EggIncubator"
#define OTA_PASSWORD                "egg12345"

// ============================================================
// STORAGE - NVS Keys
// ============================================================
#define NVS_NAMESPACE               "incubator"

// ============================================================
// INCUBATION
// ============================================================
#define MS_PER_DAY                  86400000UL  // 24 * 60 * 60 * 1000

#endif
