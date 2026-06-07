/******************************************************************
 * Smart Egg Incubator - Safety Service
 * Debug/demo version
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
    uint8_t sensorFailCount;
    uint8_t fanFailCount;

    static const unsigned long FAN_GRACE_PERIOD_MS = 15000;
    static const uint8_t SENSOR_FAIL_LIMIT = 3;
    static const uint8_t FAN_FAIL_LIMIT = 3;

    void checkSensorFail();
    void checkOverTemp();
    void checkFanStall();
    void triggerSafety(SystemSafety reason, const char* errorCode);
    void publishAlarm(const char* alarmType);
};

#endif
