#include "HeaterService.h"
#include "../../config/Pins.h"

HeaterService::HeaterService()
{
    ctx = nullptr;
}

void HeaterService::begin(AppContext* context)
{
    ctx = context;

    pinMode(PIN_HEATER_CTRL, OUTPUT);
    setHeater(false);

    Serial.println("[Heater] Init OK");
}

void HeaterService::loop()
{
    if (ctx == nullptr) return;

    if (!ctx->state.sensorOk)
    {
        forceOff();
        return;
    }

    if (ctx->settings.heaterMode == HEATER_MANUAL)
    {
        return;
    }

    float temp = ctx->state.temperature;
    float target = ctx->settings.targetTemp;
    float hys = ctx->settings.hysteresis;

    if (temp < target - hys)
    {
        setHeater(true);
    }
    else if (temp > target + hys)
    {
        setHeater(false);
    }
}

void HeaterService::setHeater(bool state)
{
    if (ctx != nullptr)
    {
        ctx->state.heaterOn = state;
    }

    digitalWrite(PIN_HEATER_CTRL, state ? HIGH : LOW);
}

void HeaterService::forceOff()
{
    setHeater(false);
}