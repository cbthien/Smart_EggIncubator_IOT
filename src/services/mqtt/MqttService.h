/******************************************************************
 * Smart Egg Incubator - MQTT Service
 * PubSubClient-based MQTT communication
 * Publish telemetry, subscribe to commands
 ******************************************************************/

#ifndef MQTT_SERVICE_H
#define MQTT_SERVICE_H

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

#include "../../core/AppContext.h"

// Forward declarations for command handling
class MqttService;

// Callback type for command processing
typedef void (*CommandCallback)(const char* cmd, const char* payload);

class MqttService
{
public:
    MqttService();

    void begin(AppContext* context);
    void loop();

    void publish(const char* topic, const char* payload);
    void publishTelemetry(const char* json);
    void publishAlarm(const char* message);

    bool isConnected();

    void setCommandCallback(CommandCallback cb);

private:
    AppContext* ctx;
    WiFiClient wifiClient;
    PubSubClient mqttClient;

    unsigned long lastReconnectMs;
    String clientId;
    CommandCallback cmdCallback;

    void connect();
    void subscribe();

    static void mqttCallback(char* topic, byte* payload, unsigned int length);
    static MqttService* instance;  // For static callback access
};

#endif
