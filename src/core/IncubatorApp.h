/******************************************************************
 * Smart Egg Incubator - Main Application Orchestrator
 * Initializes and coordinates all services
 * Includes OTA update support
 ******************************************************************/

#ifndef INCUBATOR_APP_H
#define INCUBATOR_APP_H

#include <Arduino.h>
#include <ArduinoOTA.h>

#include "AppContext.h"

#include "../services/storage/StorageService.h"
#include "../services/sensor/SensorService.h"
#include "../services/heater/HeaterService.h"
#include "../services/fan/FanService.h"
#include "../services/tray/TrayService.h"
#include "../services/safety/SafetyService.h"
#include "../services/wifi/WiFiService.h"
#include "../services/mqtt/MqttService.h"
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
    WiFiService wifi;
    MqttService mqtt;
    TelemetryService telemetry;

    void bootSafety();
    void printBootInfo();
    void setupOTA();
    void updateIncubationDay();
    void handleCommand(const char* cmd, const char* payload);

    static IncubatorApp* instance;
    static void commandCallback(const char* cmd, const char* payload);
};

#endif
