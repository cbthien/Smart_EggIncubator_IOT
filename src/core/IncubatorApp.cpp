/******************************************************************
 * Smart Egg Incubator - Main Application Implementation
 * Boot sequence, service coordination, command handling
 * OTA update, incubation day counter
 ******************************************************************/

#include "IncubatorApp.h"
#include "../config/Pins.h"
#include "../config/Config.h"
#include <ArduinoJson.h>

// Static instance for command callback
IncubatorApp* IncubatorApp::instance = nullptr;

IncubatorApp::IncubatorApp()
{
    instance = this;
}

void IncubatorApp::begin()
{
    // Step 1: Boot safety - force all relays OFF immediately
    bootSafety();

    printBootInfo();

    // Step 2: Load settings from NVS
    storage.begin(&context);
    storage.loadSettings();

    // Step 3: Initialize sensor
    sensor.begin(&context);

    // Step 4: Initialize actuators
    heater.begin(&context);
    fan.begin(&context);
    tray.begin(&context);

    // Step 5: Initialize safety (needs references to actuators + mqtt)
    // Note: mqtt not connected yet, but pointer is valid
    safety.begin(&context, &heater, &fan, &tray, &mqtt);

    // Step 6: Connect WiFi (may block for AP portal)
    wifi.begin(&context);

    // Step 7: Setup OTA (requires WiFi)
    if (wifi.isConnected())
    {
        setupOTA();
    }

    // Step 8: Connect MQTT
    mqtt.begin(&context);
    mqtt.setCommandCallback(commandCallback);

    // Step 9: Initialize telemetry
    telemetry.begin(&context, &mqtt);

    Serial.println();
    Serial.println("================================");
    Serial.println(" System Ready!");
    Serial.println("================================");
    Serial.println();
}

void IncubatorApp::loop()
{
    // OTA handle (check for firmware updates)
    ArduinoOTA.handle();

    // Read sensors
    sensor.loop();

    // Update incubation day counter
    updateIncubationDay();

    // Safety check (highest priority - runs before actuators)
    safety.loop();

    // Actuator control (only if safety allows)
    heater.loop();
    fan.loop();
    tray.loop();

    // Network
    wifi.loop();
    mqtt.loop();

    // Telemetry (Serial + MQTT)
    telemetry.loop();

    // Small delay to prevent watchdog issues
    delay(10);
}

void IncubatorApp::bootSafety()
{
    Serial.println("[Boot] Forcing all relays OFF...");

    // Configure all relay pins as OUTPUT and force OFF
    for (int i = 0; i < NUM_RELAY_PINS; i++)
    {
        pinMode(ALL_RELAY_PINS[i], OUTPUT);
        digitalWrite(ALL_RELAY_PINS[i], RELAY_OFF);
    }

    Serial.println("[Boot] All relays OFF - safe state confirmed");
}

void IncubatorApp::printBootInfo()
{
    Serial.println();
    Serial.println("================================");
    Serial.println(" Smart Egg Incubator v2.1");
    Serial.println(" ESP32-S3 IoT Controller");
    Serial.println(" Visual Micro + Arduino");
    Serial.println("================================");
    Serial.println();
    Serial.println("[Boot] System starting...");
    Serial.print("[Boot] Free heap: ");
    Serial.print(ESP.getFreeHeap());
    Serial.println(" bytes");
}

void IncubatorApp::setupOTA()
{
    ArduinoOTA.setHostname(OTA_HOSTNAME);
    ArduinoOTA.setPassword(OTA_PASSWORD);

    ArduinoOTA.onStart([]() {
        Serial.println("[OTA] Update starting...");
    });

    ArduinoOTA.onEnd([]() {
        Serial.println("[OTA] Update complete!");
    });

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
    uint16_t newDay = (uint16_t)(elapsed / MS_PER_DAY) + 1;  // Day 1 = first day

    // Save to NVS when day changes (survives reboot)
    if (newDay != context.state.incubationDay && newDay > context.state.incubationDay)
    {
        context.state.incubationDay = newDay;
        storage.saveSettings();
        Serial.print("[Incubation] Day changed to: ");
        Serial.println(newDay);
    }
    else
    {
        context.state.incubationDay = newDay;
    }
}

// Static callback bridge
void IncubatorApp::commandCallback(const char* topic, const char* payload)
{
    if (instance != nullptr)
    {
        instance->handleCommand(topic, payload);
    }
}

void IncubatorApp::handleCommand(const char* topic, const char* payload)
{
    Serial.print("[Command] Processing: ");
    Serial.println(payload);

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, payload);

    if (error)
    {
        Serial.print("[Command] JSON parse error: ");
        Serial.println(error.c_str());
        return;
    }

    const char* cmd = doc["cmd"];
    if (cmd == nullptr)
    {
        Serial.println("[Command] Missing 'cmd' field");
        return;
    }

    // === Temperature commands ===
    if (strcmp(cmd, "set_temp") == 0)
    {
        float value = doc["value"] | 37.5f;
        if (value >= 30.0f && value <= 40.0f)
        {
            context.settings.targetTemp = value;
            storage.saveSettings();
            Serial.print("[Command] Target temp set to: ");
            Serial.println(value);
        }
    }
    // === Fan commands ===
    else if (strcmp(cmd, "set_fan_speed") == 0)
    {
        int value = doc["value"] | 100;
        if (value >= 0 && value <= 100)
        {
            uint8_t speed = (uint8_t)((uint16_t)value * 255 / 100);
            context.settings.fanSpeed = speed;
            fan.setSpeed(speed);
            storage.saveSettings();
            Serial.print("[Command] Fan speed set to: ");
            Serial.print(value);
            Serial.println("%");
        }
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
    // === Tray commands ===
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
    // === Heater commands ===
    else if (strcmp(cmd, "heater_off") == 0)
    {
        context.settings.heaterMode = HEATER_MANUAL_OFF;
        heater.forceOff();
        Serial.println("[Command] Heater forced OFF");
    }
    else if (strcmp(cmd, "heater_auto") == 0)
    {
        context.settings.heaterMode = HEATER_AUTO;
        Serial.println("[Command] Heater set to AUTO");
    }
    // === Incubation commands ===
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
    // === System commands ===
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
        Serial.print("[Command] Unknown command: ");
        Serial.println(cmd);
    }
}
