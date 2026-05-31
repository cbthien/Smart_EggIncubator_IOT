/******************************************************************
 * Smart Egg Incubator - Safety Service
 * Highest priority: overrides all other controls
 * Monitors: over-temp, sensor fail, fan stall, motor timeout
 * Publishes alarms via MQTT for webapp notification
 ******************************************************************/

#ifndef SAFETY_SERVICE_H
#define SAFETY_SERVICE_H

#include <Arduino.h>
#include "../../core/AppContext.h"
#include "../heater/HeaterService.h"
#include "../fan/FanService.h"
#include "../tray/TrayService.h"
#include "../mqtt/MqttService.h"

class SafetyService
{
public:
    SafetyService();

    void begin(AppContext* context, HeaterService* heater, FanService* fan, TrayService* tray, MqttService* mqtt);
    void loop();

    bool isSafe() const;

private:
    AppContext* ctx;
    HeaterService* heaterService;
    FanService* fanService;
    TrayService* trayService;
    MqttService* mqttService;

    unsigned long bootTimeMs;
    static const unsigned long FAN_GRACE_PERIOD_MS = 15000;  // 15s grace after boot

    void checkSensorFail();
    void checkOverTemp();
    void checkFanStall();
    void triggerSafety(SystemSafety reason, const char* errorCode);
    void publishAlarm(const char* alarmType);
};

#endif
