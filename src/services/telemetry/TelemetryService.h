/******************************************************************
 * Smart Egg Incubator - Telemetry Service
 * Publishes system state via Serial and MQTT (JSON)
 ******************************************************************/

#ifndef TELEMETRY_SERVICE_H
#define TELEMETRY_SERVICE_H

#include <Arduino.h>
#include "../../core/AppContext.h"
#include "../mqtt/MqttService.h"

class TelemetryService
{
public:
    TelemetryService();

    void begin(AppContext* context, MqttService* mqtt);
    void loop();

private:
    AppContext* ctx;
    MqttService* mqttService;
    unsigned long lastPublishMs;

    void publishTelemetry();
    void printSerial();
    String buildJson();

    const char* trayToString(TrayPosition position);
    const char* safetyToString(SystemSafety safety);
};

#endif
