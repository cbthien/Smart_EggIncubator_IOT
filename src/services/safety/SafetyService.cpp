#include "SafetyService.h"

SafetyService::SafetyService()
{
    ctx = nullptr;
    heaterService = nullptr;
    trayService = nullptr;
}

void SafetyService::begin(AppContext* context, HeaterService* heater, TrayService* tray)
{
    ctx = context;
    heaterService = heater;
    trayService = tray;

    Serial.println("[Safety] Init OK");
}

void SafetyService::loop()
{
    if (ctx == nullptr) return;

    if (!ctx->state.sensorOk)
    {
        if (heaterService != nullptr)
        {
            heaterService->forceOff();
        }

        return;
    }

    if (ctx->state.temperature > ctx->settings.maxTemp)
    {
        if (heaterService != nullptr)
        {
            heaterService->forceOff();
        }

        ctx->state.errorCode = "OVER_TEMP";
    }

    if (ctx->state.leftLimit && ctx->state.rightLimit)
    {
        if (trayService != nullptr)
        {
            trayService->stopMotor();
        }

        ctx->state.trayPosition = TRAY_ERROR;
        ctx->state.errorCode = "BOTH_LIMIT_ACTIVE";
    }
}