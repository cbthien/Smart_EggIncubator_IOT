#ifndef TELEMETRY_SERVICE_H
#define TELEMETRY_SERVICE_H

#include <Arduino.h>
#include "../../core/AppContext.h"

class TelemetryService
{
public:
    TelemetryService();

    void begin(AppContext* context);
    void loop();

private:
    AppContext* ctx;
    unsigned long lastPrintMs;

    const char* trayToString(TrayPosition position);
};

#endif