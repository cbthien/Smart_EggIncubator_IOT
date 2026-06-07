/******************************************************************
 * Smart Egg Incubator - Heater Service
 * Controls 2 heaters (left/right) via relay with hysteresis
 * Anti-chatter: minimum ON/OFF time to protect relay
 ******************************************************************/

#ifndef HEATER_SERVICE_H
#define HEATER_SERVICE_H

#include <Arduino.h>
#include "../../core/AppContext.h"

class HeaterService
{
public:
    HeaterService();

    void begin(AppContext* context);
    void loop();

    void setHeaters(bool state);
    void forceOff();

    bool isOn() const;

private:
    AppContext* ctx;
    unsigned long lastStateChangeMs;
    bool currentState;

    static const unsigned long MIN_ON_TIME_MS = 10000;   // 30s minimum ON
    static const unsigned long MIN_OFF_TIME_MS = 5000;  // 10s minimum OFF
};

#endif
