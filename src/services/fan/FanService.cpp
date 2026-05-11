#include "FanService.h"
#include "../../config/Pins.h"

FanService::FanService()
{
    ctx = nullptr;
}

void FanService::begin(AppContext* context)
{
    ctx = context;

    pinMode(PIN_FAN_CTRL, OUTPUT);
    setFan(false);

    Serial.println("[Fan] Init OK");
}

void FanService::loop()
{
    if (ctx == nullptr) return;

    if (ctx->settings.fanMode == FAN_ON)
    {
        setFan(true);
    }
    else if (ctx->settings.fanMode == FAN_OFF)
    {
        setFan(false);
    }
    else
    {
        setFan(true);
    }
}

void FanService::setFan(bool state)
{
    if (ctx != nullptr)
    {
        ctx->state.fanOn = state;
    }

    digitalWrite(PIN_FAN_CTRL, state ? HIGH : LOW);
}

void FanService::forceOn()
{
    setFan(true);
}

void FanService::forceOff()
{
    setFan(false);
}