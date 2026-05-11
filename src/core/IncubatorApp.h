/******************************************************************
 Created with PROGRAMINO IDE for Arduino 
 Libraries   :
 Author      :
 Description :
******************************************************************/


#ifndef INCUBATOR_APP_H
#define INCUBATOR_APP_H

#include <Arduino.h>

#include "AppContext.h"

#include "../services/storage/StorageService.h"
#include "../services/sensor/SensorService.h"
#include "../services/heater/HeaterService.h"
#include "../services/fan/FanService.h"
#include "../services/tray/TrayService.h"
#include "../services/safety/SafetyService.h"
#include "../services/telemetry/TelemetryService.h"

class IncubatorApp
{
public:
    IncubatorApp();

    void begin();
    void loop();

private:
    AppContext context;

    StorageService storage;
    SensorService sensor;
    HeaterService heater;
    FanService fan;
    TrayService tray;
    SafetyService safety;
    TelemetryService telemetry;

    void printBootInfo();
};

#endif