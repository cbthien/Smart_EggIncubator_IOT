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

    void setHeater(bool state);
    void forceOff();

private:
    AppContext* ctx;
};

#endif