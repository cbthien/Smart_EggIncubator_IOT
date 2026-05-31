/******************************************************************
 * Smart Egg Incubator - WiFi Service
 * WiFiManager-based provisioning (AP mode for setup)
 * - First boot: opens AP portal (blocks until configured)
 * - WiFi lost: auto-reconnect
 * - Command "reset_wifi": clears credentials + opens AP again
 ******************************************************************/

#ifndef WIFI_SERVICE_H
#define WIFI_SERVICE_H

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiManager.h>

#include "../../core/AppContext.h"

class WiFiService
{
public:
    WiFiService();

    void begin(AppContext* context);
    void loop();

    bool isConnected() const;
    String getIP() const;

    // Reset WiFi credentials and reboot into AP mode
    void resetAndReboot();

private:
    AppContext* ctx;
    WiFiManager wifiManager;
    unsigned long lastCheckMs;

    void checkConnection();
};

#endif
