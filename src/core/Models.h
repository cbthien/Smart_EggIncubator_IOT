/******************************************************************
 * Smart Egg Incubator - Data Models
 * Enums and structs for system state and settings
 ******************************************************************/

#ifndef MODELS_H
#define MODELS_H

#include <Arduino.h>

// ============================================================
// ENUMS
// ============================================================

enum TrayPosition
{
    TRAY_UNKNOWN,
    TRAY_LEFT,
    TRAY_RIGHT,
    TRAY_MOVING,
    TRAY_ERROR
};

enum HeaterMode
{
    HEATER_AUTO,
    HEATER_MANUAL_ON,
    HEATER_MANUAL_OFF
};

enum SystemSafety
{
    SAFETY_OK,
    SAFETY_OVER_TEMP,
    SAFETY_SENSOR_FAIL,
    SAFETY_FAN_STALL,
    SAFETY_MOTOR_TIMEOUT,
    SAFETY_LIMIT_ERROR
};

// ============================================================
// SYSTEM STATE (runtime, read-only from outside)
// ============================================================

struct IncubatorState
{
    // Sensor
    float temperature = 0.0f;
    float humidity = 0.0f;
    bool sensorOk = false;

    // Heater
    bool heaterLeftOn = false;
    bool heaterRightOn = false;

    // Fan
    uint8_t fanLeftSpeed = 0;       // 0-255 PWM duty
    uint8_t fanRightSpeed = 0;
    uint16_t fanLeftRpm = 0;
    uint16_t fanRightRpm = 0;
    bool fanLeftOk = true;
    bool fanRightOk = true;

    // Tray
    TrayPosition trayPosition = TRAY_UNKNOWN;
    bool trayTurning = false;

    // Limit switches
    bool limitLeft = false;
    bool limitRight = false;

    // Connectivity
    bool wifiConnected = false;
    bool mqttConnected = false;

    // Safety
    SystemSafety safety = SAFETY_OK;
    String errorCode = "NONE";

    // Incubation
    uint16_t incubationDay = 0;     // Current day (0 = not started)

    // Uptime
    unsigned long uptimeMs = 0;
};

// ============================================================
// SYSTEM SETTINGS (configurable, stored in NVS)
// ============================================================

struct IncubatorSettings
{
    // Temperature
    float targetTemp = 37.5f;
    float hysteresis = 0.2f;
    float maxTemp = 39.5f;

    // Heater
    HeaterMode heaterMode = HEATER_AUTO;

    // Fan
    uint8_t fanSpeed = 255;             // Default 100%
    bool fanStallCheckEnabled = true;   // Can disable for testing without fans

    // Tray
    unsigned long turnIntervalMs = 2UL * 60UL * 60UL * 1000UL;  // 2 hours
    unsigned long motorTimeoutMs = 60000;   // 60s
    bool autoTurnEnabled = true;

    // Incubation
    unsigned long incubationStartMs = 0;    // millis() when incubation started (0 = not started)
    bool incubationActive = false;          // Is incubation running?
};

#endif
