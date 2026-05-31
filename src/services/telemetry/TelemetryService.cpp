/******************************************************************
 * Smart Egg Incubator - Telemetry Service Implementation
 * Builds JSON telemetry and publishes via MQTT + Serial
 ******************************************************************/

#include "TelemetryService.h"
#include "../../config/Config.h"
#include <ArduinoJson.h>

TelemetryService::TelemetryService()
{
    ctx = nullptr;
    mqttService = nullptr;
    lastPublishMs = 0;
}

void TelemetryService::begin(AppContext* context, MqttService* mqtt)
{
    ctx = context;
    mqttService = mqtt;

    Serial.println("[Telemetry] Init OK");
}

void TelemetryService::loop()
{
    if (ctx == nullptr) return;

    unsigned long now = millis();

    if (now - lastPublishMs < TELEMETRY_INTERVAL_MS)
    {
        return;
    }

    lastPublishMs = now;

    // Update uptime
    ctx->state.uptimeMs = now;

    publishTelemetry();

    if (SERIAL_TELEMETRY_ENABLED)
    {
        printSerial();
    }
}

void TelemetryService::publishTelemetry()
{
    if (mqttService == nullptr) return;
    if (!mqttService->isConnected()) return;

    String json = buildJson();
    mqttService->publishTelemetry(json.c_str());
}

String TelemetryService::buildJson()
{
    JsonDocument doc;

    // Sensor
    doc["temperature"] = ctx->state.temperature;
    doc["humidity"] = ctx->state.humidity;
    doc["sensor_ok"] = ctx->state.sensorOk;

    // Heater
    doc["heater_left"] = ctx->state.heaterLeftOn;
    doc["heater_right"] = ctx->state.heaterRightOn;

    // Fan
    doc["fan_left_rpm"] = ctx->state.fanLeftRpm;
    doc["fan_right_rpm"] = ctx->state.fanRightRpm;
    doc["fan_left_speed"] = ctx->state.fanLeftSpeed;
    doc["fan_right_speed"] = ctx->state.fanRightSpeed;
    doc["fan_left_ok"] = ctx->state.fanLeftOk;
    doc["fan_right_ok"] = ctx->state.fanRightOk;
    doc["fan_check_enabled"] = ctx->settings.fanStallCheckEnabled;

    // Tray
    doc["tray_position"] = trayToString(ctx->state.trayPosition);
    doc["tray_turning"] = ctx->state.trayTurning;
    doc["auto_turn_enabled"] = ctx->settings.autoTurnEnabled;

    // Incubation
    doc["incubation_day"] = ctx->state.incubationDay;
    doc["incubation_active"] = ctx->settings.incubationActive;

    // Connectivity
    doc["wifi"] = ctx->state.wifiConnected;
    doc["mqtt"] = ctx->state.mqttConnected;

    // Safety
    doc["safety"] = safetyToString(ctx->state.safety);
    doc["error"] = ctx->state.errorCode;

    // System
    doc["uptime_s"] = ctx->state.uptimeMs / 1000;
    doc["target_temp"] = ctx->settings.targetTemp;
    doc["fan_speed_set"] = ctx->settings.fanSpeed;

    String output;
    serializeJson(doc, output);
    return output;
}

void TelemetryService::printSerial()
{
    Serial.println("---------- TELEMETRY ----------");
    Serial.print("Temp: "); Serial.print(ctx->state.temperature); Serial.println(" C");
    Serial.print("Humidity: "); Serial.print(ctx->state.humidity); Serial.println(" %");
    Serial.print("Sensor: "); Serial.println(ctx->state.sensorOk ? "OK" : "FAIL");
    Serial.print("Heater L/R: ");
    Serial.print(ctx->state.heaterLeftOn ? "ON" : "OFF");
    Serial.print("/");
    Serial.println(ctx->state.heaterRightOn ? "ON" : "OFF");
    Serial.print("Fan L RPM: "); Serial.print(ctx->state.fanLeftRpm);
    Serial.print(" | R RPM: "); Serial.println(ctx->state.fanRightRpm);
    Serial.print("Fan check: "); Serial.println(ctx->settings.fanStallCheckEnabled ? "ON" : "OFF");
    Serial.print("Tray: "); Serial.print(trayToString(ctx->state.trayPosition));
    Serial.print(" | Auto-turn: "); Serial.println(ctx->settings.autoTurnEnabled ? "ON" : "OFF");
    Serial.print("Incubation day: "); Serial.print(ctx->state.incubationDay);
    Serial.print(" | Active: "); Serial.println(ctx->settings.incubationActive ? "YES" : "NO");
    Serial.print("Safety: "); Serial.println(safetyToString(ctx->state.safety));
    Serial.print("Error: "); Serial.println(ctx->state.errorCode);
    Serial.print("WiFi: "); Serial.print(ctx->state.wifiConnected ? "OK" : "NO");
    Serial.print(" | MQTT: "); Serial.println(ctx->state.mqttConnected ? "OK" : "NO");
    Serial.println("-------------------------------");
}

const char* TelemetryService::trayToString(TrayPosition position)
{
    switch (position)
    {
    case TRAY_LEFT:    return "LEFT";
    case TRAY_RIGHT:   return "RIGHT";
    case TRAY_MOVING:  return "MOVING";
    case TRAY_ERROR:   return "ERROR";
    default:           return "UNKNOWN";
    }
}

const char* TelemetryService::safetyToString(SystemSafety safety)
{
    switch (safety)
    {
    case SAFETY_OK:            return "OK";
    case SAFETY_OVER_TEMP:     return "OVER_TEMP";
    case SAFETY_SENSOR_FAIL:   return "SENSOR_FAIL";
    case SAFETY_FAN_STALL:     return "FAN_STALL";
    case SAFETY_MOTOR_TIMEOUT: return "MOTOR_TIMEOUT";
    case SAFETY_LIMIT_ERROR:   return "LIMIT_ERROR";
    default:                   return "UNKNOWN";
    }
}
