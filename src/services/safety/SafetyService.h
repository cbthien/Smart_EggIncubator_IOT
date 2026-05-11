#ifndef SAFETY_SERVICE_H
#define SAFETY_SERVICE_H

#include <Arduino.h>
#include "../../core/AppContext.h"
#include "../heater/HeaterService.h"
#include "../tray/TrayService.h"

class SafetyService
{
public:
    SafetyService();

    void begin(AppContext* context, HeaterService* heater, TrayService* tray);
    void loop();

private:
    AppContext* ctx;
    HeaterService* heaterService;
    TrayService* trayService;
};

#endif