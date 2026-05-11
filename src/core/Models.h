/******************************************************************
 Created with PROGRAMINO IDE for Arduino 
 Libraries   :
 Author      :
 Description :
******************************************************************/


#ifndef MODELS_H
#define MODELS_H

#include <Arduino.h>

enum TrayPosition
{
    TRAY_UNKNOWN,
    TRAY_LEFT,
    TRAY_RIGHT,
    TRAY_MOVING_LEFT,
    TRAY_MOVING_RIGHT,
    TRAY_ERROR
};

enum HeaterMode
{
    HEATER_AUTO,
    HEATER_MANUAL
};

enum FanMode
{
    FAN_AUTO,
    FAN_ON,
    FAN_OFF
};

struct IncubatorState
{
    float temperature = 0;
    float humidity = 0;

    bool sensorOk = false;
    bool heaterOn = false;
    bool fanOn = false;

    bool leftLimit = false;
    bool rightLimit = false;
    bool turning = false;

    bool wifiConnected = false;
    bool mqttConnected = false;

    TrayPosition trayPosition = TRAY_UNKNOWN;

    String errorCode = "NONE";
};

struct IncubatorSettings
{
    float targetTemp = 37.5;
    float hysteresis = 0.2;
    float maxTemp = 39.5;

    unsigned long turnIntervalMs = 2UL * 60UL * 60UL * 1000UL;
    unsigned long motorTimeoutMs = 30000;

    HeaterMode heaterMode = HEATER_AUTO;
    FanMode fanMode = FAN_AUTO;

    bool autoTurnEnabled = true;
};

#endif