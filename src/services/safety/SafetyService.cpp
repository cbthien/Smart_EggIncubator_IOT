/******************************************************************
 * Smart Egg Incubator - Safety Service Implementation
 * Debug/demo version:
 *   - Logs why heater is disabled
 *   - Requires 3 consecutive sensor/fan fails before alarm
 *   - Over-temp still immediate
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
    sensorFailCount = 0;
    fanFailCount = 0;
}

void SafetyService::begin(AppContext* context, HeaterService* heater, FanService* fan, TrayService* tray, MqttService* mqtt)
{
    ctx = context;
    heaterService = heater;
    fanService = fan;
    trayService = tray;
    mqttService = mqtt;
    bootTimeMs = millis();
    sensorFailCount = 0;
    fanFailCount = 0;

    Serial.println("[Safety] Init OK");
    Serial.print("[Safety] Max temp: "); Serial.println(ctx->settings.maxTemp);
    Serial.println("[Safety] Sensor fail limit: 3 consecutive checks");
    Serial.println("[Safety] Fan fail limit: 3 consecutive checks after 15s boot grace");
}

void SafetyService::loop()
{
    if (ctx == nullptr) return;

    checkSensorFail();
    checkOverTemp();
    checkFanStall();

    if (ctx->state.safety != SAFETY_OK)
    {
        bool sensorOk = ctx->state.sensorOk;
        bool tempOk = ctx->state.temperature < ctx->settings.maxTemp;
        bool fansOk = ctx->state.fanLeftOk && ctx->state.fanRightOk;

        if (sensorOk && tempOk && fansOk && ctx->state.safety != SAFETY_MOTOR_TIMEOUT)
        {
            ctx->state.safety = SAFETY_OK;
            ctx->state.errorCode = "NONE";
            sensorFailCount = 0;
            fanFailCount = 0;
            Serial.println("[Safety] Conditions cleared - back to OK");
        }
    }
}

void SafetyService::checkSensorFail()
{
    if (ctx == nullptr) return;

    if (ctx->state.sensorOk)
    {
        if (sensorFailCount > 0) Serial.println("[Safety] Sensor recovered, fail count reset");
        sensorFailCount = 0;
        return;
    }

    sensorFailCount++;
    Serial.print("[Safety] Sensor not OK. failCount="); Serial.print(sensorFailCount);
    Serial.print("/"); Serial.println(SENSOR_FAIL_LIMIT);

    if (sensorFailCount >= SENSOR_FAIL_LIMIT)
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
        Serial.print("[Safety] Over temp detected: "); Serial.print(ctx->state.temperature);
        Serial.print(" >= "); Serial.println(ctx->settings.maxTemp);
        triggerSafety(SAFETY_OVER_TEMP, "OVER_TEMP");
    }
}

void SafetyService::checkFanStall()
{
    if (ctx == nullptr) return;

    if (!ctx->settings.fanStallCheckEnabled)
    {
        fanFailCount = 0;
        return;
    }

    if (millis() - bootTimeMs < FAN_GRACE_PERIOD_MS) return;

    if (!ctx->state.heaterLeftOn && !ctx->state.heaterRightOn)
    {
        fanFailCount = 0;
        return;
    }

    if (ctx->state.fanLeftOk && ctx->state.fanRightOk)
    {
        if (fanFailCount > 0) Serial.println("[Safety] Fan recovered, fail count reset");
        fanFailCount = 0;
        return;
    }

    fanFailCount++;
    Serial.print("[Safety] Fan not OK. L/R RPM="); Serial.print(ctx->state.fanLeftRpm);
    Serial.print("/"); Serial.print(ctx->state.fanRightRpm);
    Serial.print(" failCount="); Serial.print(fanFailCount);
    Serial.print("/"); Serial.println(FAN_FAIL_LIMIT);

    if (fanFailCount >= FAN_FAIL_LIMIT)
    {
        triggerSafety(SAFETY_FAN_STALL, "FAN_STALL");
    }
}

void SafetyService::triggerSafety(SystemSafety reason, const char* errorCode)
{
    if (ctx == nullptr) return;

    bool stateChanged = (ctx->state.safety != reason);
    ctx->state.safety = reason;
    ctx->state.errorCode = errorCode;

    if (heaterService != nullptr)
    {
        heaterService->forceOff();
    }

    if (stateChanged)
    {
        Serial.print("[Safety] ALARM: "); Serial.println(errorCode);
        Serial.println("[Safety] Heater forced OFF");
        publishAlarm(errorCode);
    }
}

void SafetyService::publishAlarm(const char* alarmType)
{
    if (ctx == nullptr) return;
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
