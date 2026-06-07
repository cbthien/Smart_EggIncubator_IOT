/******************************************************************
 * Smart Egg Incubator - Main Application Implementation
 * Debug/demo version
 ******************************************************************/

#include "IncubatorApp.h"
#include "../config/Pins.h"
#include "../config/Config.h"
#include <ArduinoJson.h>

IncubatorApp* IncubatorApp::instance = nullptr;

IncubatorApp::IncubatorApp()
{
    instance = this;
}

void IncubatorApp::begin()
{
    Serial.begin(115200);
    delay(500);

    bootSafety();
    printBootInfo();

    storage.begin(&context);
    storage.loadSettings();

    sensor.begin(&context);

    heater.begin(&context);
    fan.begin(&context);
    tray.begin(&context);

    safety.begin(&context, &heater, &fan, &tray, &mqtt);

    wifi.begin(&context);

    if (wifi.isConnected())
    {
        setupOTA();
    }
    else
    {
        Serial.println("[App] WiFi not connected yet, OTA not started");
    }

    mqtt.begin(&context);
    mqtt.setCommandCallback(commandCallback);

    telemetry.begin(&context, &mqtt);

    Serial.println();
    Serial.println("================================");
    Serial.println(" System Ready - DEBUG LOG MODE");
    Serial.println("================================");
    Serial.println();
}

void IncubatorApp::loop()
{
    static unsigned long lastHeartbeatMs = 0;

    if (wifi.isConnected())
    {
        ArduinoOTA.handle();
    }

    sensor.loop();
    updateIncubationDay();
    safety.loop();

    if (safety.isSafe())
    {
        heater.loop();
    }
    else
    {
        heater.forceOff();
    }

    fan.loop();
    tray.loop();

    wifi.loop();
    mqtt.loop();
    telemetry.loop();

    if (millis() - lastHeartbeatMs >= 5000)
    {
        lastHeartbeatMs = millis();
        Serial.print("[App] Heartbeat uptime="); Serial.print(millis() / 1000);
        Serial.print("s sensor="); Serial.print(context.state.sensorOk ? "OK" : "FAIL");
        Serial.print(" safety="); Serial.print(context.state.safety);
        Serial.print(" error="); Serial.print(context.state.errorCode);
        Serial.print(" heater="); Serial.print(context.state.heaterLeftOn || context.state.heaterRightOn ? "ON" : "OFF");
        Serial.print(" trayTurning="); Serial.println(context.state.trayTurning ? "YES" : "NO");
    }

    delay(10);
}

void IncubatorApp::bootSafety()
{
    Serial.println("[Boot] Forcing all relays OFF...");

    for (int i = 0; i < NUM_RELAY_PINS; i++)
    {
        pinMode(ALL_RELAY_PINS[i], OUTPUT);
        digitalWrite(ALL_RELAY_PINS[i], RELAY_OFF);
        Serial.print("[Boot] Relay GPIO"); Serial.print(ALL_RELAY_PINS[i]); Serial.println(" = OFF");
    }

    Serial.println("[Boot] All relays OFF - safe state confirmed");
}

void IncubatorApp::printBootInfo()
{
    Serial.println();
    Serial.println("================================");
    Serial.println(" Smart Egg Incubator DEBUG");
    Serial.println(" ESP32-S3 IoT Controller");
    Serial.println("================================");
    Serial.print("[Boot] Free heap: "); Serial.print(ESP.getFreeHeap()); Serial.println(" bytes");
    Serial.print("[Boot] Chip MAC: "); Serial.println((uint32_t)ESP.getEfuseMac(), HEX);
}

void IncubatorApp::setupOTA()
{
    ArduinoOTA.setHostname(OTA_HOSTNAME);
    ArduinoOTA.setPassword(OTA_PASSWORD);

    ArduinoOTA.onStart([]() { Serial.println("[OTA] Update starting..."); });
    ArduinoOTA.onEnd([]() { Serial.println("[OTA] Update complete!"); });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("[OTA] Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("[OTA] Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
        else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
        else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
        else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

    ArduinoOTA.begin();
    Serial.println("[OTA] Ready - hostname: " + String(OTA_HOSTNAME));
}

void IncubatorApp::updateIncubationDay()
{
    if (!context.settings.incubationActive)
    {
        context.state.incubationDay = 0;
        return;
    }

    if (context.settings.incubationStartMs == 0)
    {
        context.state.incubationDay = 0;
        return;
    }

    unsigned long elapsed = millis() - context.settings.incubationStartMs;
    uint16_t newDay = (uint16_t)(elapsed / MS_PER_DAY) + 1;

    if (newDay != context.state.incubationDay && newDay > context.state.incubationDay)
    {
        context.state.incubationDay = newDay;
        storage.saveSettings();
        Serial.print("[Incubation] Day changed to: "); Serial.println(newDay);
    }
    else
    {
        context.state.incubationDay = newDay;
    }
}

void IncubatorApp::commandCallback(const char* topic, const char* payload)
{
    if (instance != nullptr)
    {
        instance->handleCommand(topic, payload);
    }
}

void IncubatorApp::handleCommand(const char* topic, const char* payload)
{
    Serial.print("[Command] Topic: "); Serial.println(topic);
    Serial.print("[Command] Payload: "); Serial.println(payload);

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, payload);

    if (error)
    {
        Serial.print("[Command] JSON parse error: "); Serial.println(error.c_str());
        return;
    }

    const char* cmd = doc["cmd"];
    if (cmd == nullptr)
    {
        Serial.println("[Command] Missing 'cmd' field");
        return;
    }

    if (strcmp(cmd, "set_temp") == 0)
    {
        float value = doc["value"] | 37.5f;
        if (value >= 30.0f && value <= 40.0f)
        {
            context.settings.targetTemp = value;
            storage.saveSettings();
            Serial.print("[Command] Target temp set to: "); Serial.println(value);
        }
        else Serial.println("[Command] set_temp rejected: range must be 30-40");
    }
    else if (strcmp(cmd, "set_fan_speed") == 0)
    {
        int value = doc["value"] | 100;
        if (value >= 0 && value <= 100)
        {
            uint8_t speed = (uint8_t)((uint16_t)value * 255 / 100);
            context.settings.fanSpeed = speed;
            fan.setSpeed(speed);
            storage.saveSettings();
            Serial.print("[Command] Fan speed set to: "); Serial.print(value); Serial.println("%");
        }
        else Serial.println("[Command] set_fan_speed rejected: range must be 0-100");
    }
    else if (strcmp(cmd, "disable_fan_check") == 0)
    {
        context.settings.fanStallCheckEnabled = false;
        storage.saveSettings();
        Serial.println("[Command] Fan stall check DISABLED");
    }
    else if (strcmp(cmd, "enable_fan_check") == 0)
    {
        context.settings.fanStallCheckEnabled = true;
        storage.saveSettings();
        Serial.println("[Command] Fan stall check ENABLED");
    }
    else if (strcmp(cmd, "turn_tray") == 0)
    {
        tray.startTurn();
        Serial.println("[Command] Manual tray turn triggered");
    }
    else if (strcmp(cmd, "stop_auto_turn") == 0)
    {
        context.settings.autoTurnEnabled = false;
        storage.saveSettings();
        Serial.println("[Command] Auto-turn STOPPED");
    }
    else if (strcmp(cmd, "start_auto_turn") == 0)
    {
        context.settings.autoTurnEnabled = true;
        storage.saveSettings();
        Serial.println("[Command] Auto-turn STARTED");
    }
    else if (strcmp(cmd, "heater_off") == 0)
    {
        context.settings.heaterMode = HEATER_MANUAL_OFF;
        heater.forceOff();
        storage.saveSettings();
        Serial.println("[Command] Heater forced OFF");
    }
    else if (strcmp(cmd, "heater_auto") == 0)
    {
        context.settings.heaterMode = HEATER_AUTO;
        storage.saveSettings();
        Serial.println("[Command] Heater set to AUTO");
    }
    else if (strcmp(cmd, "start_incubation") == 0)
    {
        context.settings.incubationActive = true;
        context.settings.incubationStartMs = millis();
        context.state.incubationDay = 1;
        storage.saveSettings();
        Serial.println("[Command] Incubation STARTED - Day 1");
    }
    else if (strcmp(cmd, "reset_incubation") == 0)
    {
        context.settings.incubationActive = false;
        context.settings.incubationStartMs = 0;
        context.state.incubationDay = 0;
        storage.saveSettings();
        Serial.println("[Command] Incubation RESET");
    }
    else if (strcmp(cmd, "reset_wifi") == 0)
    {
        Serial.println("[Command] Resetting WiFi credentials...");
        wifi.resetAndReboot();
    }
    else if (strcmp(cmd, "reboot") == 0)
    {
        Serial.println("[Command] Rebooting...");
        delay(500);
        ESP.restart();
    }
    else
    {
        Serial.print("[Command] Unknown command: "); Serial.println(cmd);
    }
}
