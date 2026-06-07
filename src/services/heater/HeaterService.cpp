/******************************************************************
 * Smart Egg Incubator - Heater Service Implementation
 * Dual heater control with hysteresis + anti-chatter protection
 * Minimum ON time: 30s, Minimum OFF time: 10s
 ******************************************************************/

#include "HeaterService.h"
#include "../../config/Pins.h"
#include "../../config/Config.h"

HeaterService::HeaterService()
{
    ctx = nullptr;
    lastStateChangeMs = 0;
    currentState = false;
}

void HeaterService::begin(AppContext* context)
{
    ctx = context;

    // Configure relay pins as output
    pinMode(PIN_RELAY_HEATER_LEFT, OUTPUT);
    pinMode(PIN_RELAY_HEATER_RIGHT, OUTPUT);

    // Force heaters OFF at boot (safety)
    forceOff();

    Serial.println("[Heater] Init OK - 2 channels, LOW trigger, anti-chatter");
    Serial.print("[Heater] Relay L/R GPIO: "); Serial.print(PIN_RELAY_HEATER_LEFT); Serial.print(" / "); Serial.println(PIN_RELAY_HEATER_RIGHT);
    Serial.print("[Heater] Target/Hys/Max: "); Serial.print(ctx->settings.targetTemp); Serial.print(" / "); Serial.print(ctx->settings.hysteresis); Serial.print(" / "); Serial.println(ctx->settings.maxTemp);
}

void HeaterService::loop()
{
    if (ctx == nullptr) return;

    // If sensor is not OK, heaters must be OFF (safety handles this too)
    if (!ctx->state.sensorOk)
    {
        static unsigned long lastSensorFailLogMs = 0;
        if (millis() - lastSensorFailLogMs >= 3000)
        {
            lastSensorFailLogMs = millis();
            Serial.println("[Heater] Blocked: sensor not OK");
        }
        forceOff();
        return;
    }

    // Manual mode: don't auto-control
    if (ctx->settings.heaterMode == HEATER_MANUAL_ON)
    {
        Serial.println("[Heater] Manual ON requested");
        setHeaters(true);
        return;
    }
    else if (ctx->settings.heaterMode == HEATER_MANUAL_OFF)
    {
        Serial.println("[Heater] Manual OFF requested");
        setHeaters(false);
        return;
    }

    // AUTO mode: hysteresis control with anti-chatter
    float temp = ctx->state.temperature;
    float target = ctx->settings.targetTemp;
    float hys = ctx->settings.hysteresis;

    bool wantOn = false;
    bool wantOff = false;

    if (temp < target - hys)
    {
        wantOn = true;
    }
    else if (temp > target + hys)
    {
        wantOff = true;
    }

    // Anti-chatter: enforce minimum ON/OFF time
    unsigned long elapsed = millis() - lastStateChangeMs;

    if (wantOn && !currentState)
    {
        // Want to turn ON, currently OFF
        if (elapsed >= MIN_OFF_TIME_MS)
        {
            Serial.print("[Heater] AUTO ON: temp="); Serial.print(temp); Serial.print(" target="); Serial.println(target);
            setHeaters(true);
        }
    }
    else if (wantOff && currentState)
    {
        // Want to turn OFF, currently ON
        if (elapsed >= MIN_ON_TIME_MS)
        {
            Serial.print("[Heater] AUTO OFF: temp="); Serial.print(temp); Serial.print(" target="); Serial.println(target);
            setHeaters(false);
        }
    }
}

void HeaterService::setHeaters(bool state)
{
    // Only update if state actually changes
    if (state != currentState)
    {
        currentState = state;
        lastStateChangeMs = millis();
    }

    if (ctx != nullptr)
    {
        ctx->state.heaterLeftOn = state;
        ctx->state.heaterRightOn = state;
    }

    digitalWrite(PIN_RELAY_HEATER_LEFT, state ? RELAY_ON : RELAY_OFF);
    digitalWrite(PIN_RELAY_HEATER_RIGHT, state ? RELAY_ON : RELAY_OFF);

    static bool lastLoggedState = false;
    static bool firstLog = true;
    if (firstLog || lastLoggedState != state)
    {
        firstLog = false;
        lastLoggedState = state;
        Serial.print("[Heater] Relay state changed: ");
        Serial.println(state ? "ON" : "OFF");
    }
}

void HeaterService::forceOff()
{
    // forceOff bypasses anti-chatter (safety override)
    currentState = false;
    lastStateChangeMs = millis();

    if (ctx != nullptr)
    {
        ctx->state.heaterLeftOn = false;
        ctx->state.heaterRightOn = false;
    }

    digitalWrite(PIN_RELAY_HEATER_LEFT, RELAY_OFF);
    digitalWrite(PIN_RELAY_HEATER_RIGHT, RELAY_OFF);
}

bool HeaterService::isOn() const
{
    if (ctx == nullptr) return false;
    return ctx->state.heaterLeftOn || ctx->state.heaterRightOn;
}
