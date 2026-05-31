/******************************************************************
 * Smart Egg Incubator - MQTT Service Implementation
 * Connects to HiveMQ broker, publishes telemetry, receives commands
 ******************************************************************/

#include "MqttService.h"
#include "../../config/Config.h"

// Static instance pointer for callback
MqttService* MqttService::instance = nullptr;

MqttService::MqttService() : mqttClient(wifiClient)
{
    ctx = nullptr;
    lastReconnectMs = 0;
    cmdCallback = nullptr;
    instance = this;
}

void MqttService::begin(AppContext* context)
{
    ctx = context;

    // Generate unique client ID
    clientId = String(MQTT_CLIENT_ID_PREFIX) + String((uint32_t)ESP.getEfuseMac(), HEX);

    // Configure MQTT
    mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
    mqttClient.setCallback(mqttCallback);
    mqttClient.setBufferSize(512);

    Serial.print("[MQTT] Broker: ");
    Serial.print(MQTT_BROKER);
    Serial.print(":");
    Serial.println(MQTT_PORT);
    Serial.print("[MQTT] Client ID: ");
    Serial.println(clientId);

    // Try initial connection if WiFi is available
    if (WiFi.status() == WL_CONNECTED)
    {
        connect();
    }
}

void MqttService::loop()
{
    if (ctx == nullptr) return;

    // Don't try MQTT if WiFi is down
    if (WiFi.status() != WL_CONNECTED)
    {
        ctx->state.mqttConnected = false;
        return;
    }

    if (!mqttClient.connected())
    {
        ctx->state.mqttConnected = false;

        unsigned long now = millis();
        if (now - lastReconnectMs >= MQTT_RECONNECT_INTERVAL_MS)
        {
            lastReconnectMs = now;
            connect();
        }
    }
    else
    {
        ctx->state.mqttConnected = true;
        mqttClient.loop();
    }
}

void MqttService::connect()
{
    Serial.println("[MQTT] Connecting...");

    if (mqttClient.connect(clientId.c_str()))
    {
        Serial.println("[MQTT] Connected!");
        ctx->state.mqttConnected = true;
        subscribe();

        // Publish online status
        publish(MQTT_TOPIC_STATUS, "{\"status\":\"online\"}");
    }
    else
    {
        Serial.print("[MQTT] Failed, rc=");
        Serial.println(mqttClient.state());
    }
}

void MqttService::subscribe()
{
    mqttClient.subscribe(MQTT_TOPIC_COMMAND);
    Serial.print("[MQTT] Subscribed to: ");
    Serial.println(MQTT_TOPIC_COMMAND);
}

void MqttService::publish(const char* topic, const char* payload)
{
    if (!mqttClient.connected()) return;
    mqttClient.publish(topic, payload);
}

void MqttService::publishTelemetry(const char* json)
{
    publish(MQTT_TOPIC_TELEMETRY, json);
}

void MqttService::publishAlarm(const char* message)
{
    publish(MQTT_TOPIC_ALARM, message);
}

bool MqttService::isConnected()
{
    return mqttClient.connected();
}

void MqttService::setCommandCallback(CommandCallback cb)
{
    cmdCallback = cb;
}

void MqttService::mqttCallback(char* topic, byte* payload, unsigned int length)
{
    if (instance == nullptr) return;

    // Null-terminate the payload
    char message[256];
    unsigned int copyLen = (length < 255) ? length : 255;
    memcpy(message, payload, copyLen);
    message[copyLen] = '\0';

    Serial.print("[MQTT] Received on ");
    Serial.print(topic);
    Serial.print(": ");
    Serial.println(message);

    // Forward to command callback if set
    if (instance->cmdCallback != nullptr)
    {
        instance->cmdCallback(topic, message);
    }
}
