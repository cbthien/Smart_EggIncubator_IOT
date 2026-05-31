/******************************************************************
 * Smart Egg Incubator - Safety Service Implementation
 * Highest priority safety system
 * Immediately shuts down heaters on any dangerous condition
 * Publishes alarm to MQTT for webapp notification
 ******************************************************************/

#include "SafetyService.h"
#include "../../config/Config.h"
#include <ArduinoJson.h>

SafetyService::SafetyService()
{
    ctx = nullptr;
    heaterService = nullptr;
    fanService = nullptr;
    trayService = nullptr;
    mqttService = nullptr;
    bootTimeMs = 0;
}

void SafetyService::begin(AppContext* context, HeaterService* heater, FanService* fan, TrayService* tray, MqttService* mqtt)
{
    ctx = context;
    heaterService = heater;
    fanService = fan;
    trayService = tray;
    mqttService = mqtt;
    bootTimeMs = millis();

    Serial.println("[Safety] Init OK - monitoring: temp, sensor, fan, motor");
    Serial.println("[Safety] Fan stall check grace period: 15s");
}

void SafetyService::loop()
{
    if (ctx == nullptr) return;

    // Reset safety status if conditions cleared
    if (ctx->state.safety != SAFETY_OK)
    {
        bool sensorOk = ctx->state.sensorOk;
        bool tempOk = ctx->state.temperature < ctx->settings.maxTemp;
        bool fansOk = ctx->state.fanLeftOk && ctx->state.fanRightOk;

        if (sensorOk && tempOk && fansOk && 
            ctx->state.safety != SAFETY_MOTOR_TIMEOUT)
        {
            ctx->state.safety = SAFETY_OK;
            ctx->state.errorCode = "NONE";
            Serial.println("[Safety] Conditions cleared - back to OK");
        }
    }

    // Check all safety conditions (priority order)
    checkSensorFail();
    checkOverTemp();
    checkFanStall();
}

void SafetyService::checkSensorFail()
{
    if (ctx == nullptr) return;

    if (!ctx->state.sensorOk)
    {
        triggerSafety(SAFETY_SENSOR_FAIL, "SENSOR_FAIL");
    }
}

void SafetyService::checkOverTemp()
{
    if (ctx == nullptr) return;
    if (!ctx->state.sensorOk) return;

    if (ctx->state.temperature >= ctx->settings.maxTemp)
    {
        triggerSafety(SAFETY_OVER_TEMP, "OVER_TEMP");
    }
}

void SafetyService::checkFanStall()
{
    if (ctx == nullptr) return;

    // Skip fan check if disabled (for testing without fans)
    if (!ctx->settings.fanStallCheckEnabled) return;

    // Grace period after boot (fans need time to spin up)
    if (millis() - bootTimeMs < FAN_GRACE_PERIOD_MS) return;

    // Only check if fans should be running and heater is on
    if (!ctx->state.heaterLeftOn && !ctx->state.heaterRightOn) return;

    if (!ctx->state.fanLeftOk || !ctx->state.fanRightOk)
    {
        triggerSafety(SAFETY_FAN_STALL, "FAN_STALL");
    }
}

void SafetyService::triggerSafety(SystemSafety reason, const char* errorCode)
{
    if (ctx == nullptr) return;

    // Only log and publish if state actually changed (prevent spam)
    bool stateChanged = (ctx->state.safety != reason);

    // Set safety state
    ctx->state.safety = reason;
    ctx->state.errorCode = errorCode;

    // IMMEDIATE ACTION: Force heaters OFF
    if (heaterService != nullptr)
    {
        heaterService->forceOff();
    }

    // Log and publish only on state change
    if (stateChanged)
    {
        Serial.print("[Safety] ALARM: ");
        Serial.println(errorCode);

        // Publish alarm via MQTT for webapp
        publishAlarm(errorCode);
    }
}

void SafetyService::publishAlarm(const char* alarmType)
{
    if (mqttService == nullptr) return;
    if (!mqttService->isConnected()) return;

    JsonDocument doc;
    doc["alarm"] = alarmType;
    doc["temperature"] = ctx->state.temperature;
    doc["humidity"] = ctx->state.humidity;
    doc["uptime_s"] = millis() / 1000;

    String json;
    serializeJson(doc, json);

    mqttService->publishAlarm(json.c_str());
}

bool SafetyService::isSafe() const
{
    if (ctx == nullptr) return false;
    return ctx->state.safety == SAFETY_OK;
}
