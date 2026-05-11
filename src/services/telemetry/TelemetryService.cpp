#include "TelemetryService.h"
#include "../../config/Config.h"

TelemetryService::TelemetryService()
{
    ctx = nullptr;
    lastPrintMs = 0;
}

void TelemetryService::begin(AppContext* context)
{
    ctx = context;

    Serial.println("[Telemetry] Init OK");
}

void TelemetryService::loop()
{
    if (ctx == nullptr) return;

    unsigned long now = millis();

    if (now - lastPrintMs < TELEMETRY_INTERVAL_MS)
    {
        return;
    }

    lastPrintMs = now;

    Serial.println("---------- TELEMETRY ----------");

    Serial.print("Temp: ");
    Serial.println(ctx->state.temperature);

    Serial.print("Humidity: ");
    Serial.println(ctx->state.humidity);

    Serial.print("Sensor OK: ");
    Serial.println(ctx->state.sensorOk ? "true" : "false");

    Serial.print("Heater: ");
    Serial.println(ctx->state.heaterOn ? "ON" : "OFF");

    Serial.print("Fan: ");
    Serial.println(ctx->state.fanOn ? "ON" : "OFF");

    Serial.print("Tray: ");
    Serial.println(trayToString(ctx->state.trayPosition));

    Serial.print("Left Limit: ");
    Serial.println(ctx->state.leftLimit ? "true" : "false");

    Serial.print("Right Limit: ");
    Serial.println(ctx->state.rightLimit ? "true" : "false");

    Serial.print("Turning: ");
    Serial.println(ctx->state.turning ? "true" : "false");

    Serial.print("Error: ");
    Serial.println(ctx->state.errorCode);

    Serial.println("-------------------------------");
}

const char* TelemetryService::trayToString(TrayPosition position)
{
    switch (position)
    {
    case TRAY_LEFT: return "LEFT";
    case TRAY_RIGHT: return "RIGHT";
    case TRAY_MOVING_LEFT: return "MOVING_LEFT";
    case TRAY_MOVING_RIGHT: return "MOVING_RIGHT";
    case TRAY_ERROR: return "ERROR";
    default: return "UNKNOWN";
    }
}