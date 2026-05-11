#ifndef FAN_SERVICE_H
#define FAN_SERVICE_H

#include <Arduino.h>
#include "../../core/AppContext.h"

class FanService
{
public:
    FanService();

    void begin(AppContext* context);
    void loop();

    void setFan(bool state);
    void forceOn();
    void forceOff();

private:
    AppContext* ctx;
};

#endif