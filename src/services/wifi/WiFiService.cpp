/******************************************************************
 * Smart Egg Incubator - WiFi Service Implementation
 * Uses WiFiManager for captive portal provisioning
 * 
 * Logic:
 * - Boot: try saved WiFi credentials
 * - If no saved credentials OR connection fails → open AP portal
 * - AP portal blocks until user configures WiFi
 * - Once configured, credentials are saved automatically
 * - If WiFi lost during operation → auto-reconnect
 * - Command "reset_wifi" → clear credentials + reboot → AP opens
 ******************************************************************/

#include "WiFiService.h"
#include "../../config/Config.h"

WiFiService::WiFiService()
{
    ctx = nullptr;
    lastCheckMs = 0;
}

void WiFiService::begin(AppContext* context)
{
    ctx = context;

    Serial.println("[WiFi] Starting WiFiManager...");
    Serial.println("[WiFi] If no saved WiFi, AP 'EggIncubator_Setup' will open.");
    Serial.println("[WiFi] Connect your phone to that AP to configure WiFi.");

    // NOTE: Uncomment the line below to force AP mode (clear saved WiFi).
    // Use this when you need to change WiFi network.
    // After configuring new WiFi, comment it back and re-upload.
    // You can also send MQTT command {"cmd": "reset_wifi"} to reset remotely.
    
    wifiManager.resetSettings();  //(bỏ comment dòng này để set wifi mới)

    // Configure WiFiManager
    wifiManager.setConfigPortalTimeout(WIFI_CONNECT_TIMEOUT_S);  // 120s timeout
    wifiManager.setConnectTimeout(10);  // 10s to try connecting

    // This will:
    // 1. Try to connect with saved credentials (none after reset)
    // 2. If fails → open AP "EggIncubator_Setup" 
    // 3. Block here until user configures WiFi OR timeout
    bool connected = wifiManager.autoConnect(WIFI_AP_NAME);

    if (connected)
    {
        ctx->state.wifiConnected = true;
        Serial.print("[WiFi] Connected! IP: ");
        Serial.println(WiFi.localIP());
    }
    else
    {
        ctx->state.wifiConnected = false;
        Serial.println("[WiFi] AP portal timed out. Continuing without WiFi.");
        Serial.println("[WiFi] Send MQTT cmd 'reset_wifi' or reboot to try again.");
    }
}

void WiFiService::loop()
{
    if (ctx == nullptr) return;

    unsigned long now = millis();

    if (now - lastCheckMs >= WIFI_RECONNECT_INTERVAL_MS)
    {
        lastCheckMs = now;
        checkConnection();
    }
}

void WiFiService::checkConnection()
{
    bool connected = (WiFi.status() == WL_CONNECTED);

    if (ctx->state.wifiConnected && !connected)
    {
        Serial.println("[WiFi] Connection lost! Reconnecting...");
        WiFi.reconnect();
    }
    else if (!ctx->state.wifiConnected && connected)
    {
        Serial.print("[WiFi] Reconnected! IP: ");
        Serial.println(WiFi.localIP());
    }

    ctx->state.wifiConnected = connected;
}

bool WiFiService::isConnected() const
{
    return WiFi.status() == WL_CONNECTED;
}

String WiFiService::getIP() const
{
    if (WiFi.status() == WL_CONNECTED)
    {
        return WiFi.localIP().toString();
    }
    return "0.0.0.0";
}

void WiFiService::resetAndReboot()
{
    Serial.println("[WiFi] Resetting WiFi credentials...");
    wifiManager.resetSettings();
    Serial.println("[WiFi] Credentials cleared. Rebooting...");
    delay(500);
    ESP.restart();
}
