/******************************************************************
 * Smart Egg Incubator - Sensor Service Implementation
 * SHT30 I2C temperature/humidity sensor
 ******************************************************************/

#include "SensorService.h"
#include "../../config/Pins.h"
#include "../../config/Config.h"

SensorService::SensorService()
{
    ctx = nullptr;
    lastReadMs = 0;
}

void SensorService::begin(AppContext* context)
{
    ctx = context;

    Wire.begin(PIN_SHT30_SDA, PIN_SHT30_SCL);

    Serial.println("[Sensor] Init SHT30...");

    if (!sht30.begin(SENSOR_I2C_ADDRESS))
    {
        ctx->state.sensorOk = false;
        ctx->state.errorCode = "SHT30_NOT_FOUND";
        Serial.println("[Sensor] ERROR: SHT30 NOT FOUND!");
        return;
    }

    ctx->state.sensorOk = true;
    ctx->state.errorCode = "NONE";

    Serial.println("[Sensor] SHT30 OK");
}

void SensorService::loop()
{
    if (ctx == nullptr) return;

    unsigned long now = millis();

    if (now - lastReadMs >= SENSOR_READ_INTERVAL_MS)
    {
        lastReadMs = now;
        readSensor();
    }
}

void SensorService::readSensor()
{
    float t = sht30.readTemperature();
    float h = sht30.readHumidity();

    if (isnan(t) || isnan(h))
    {
        ctx->state.sensorOk = false;
        ctx->state.errorCode = "SHT30_READ_ERROR";
        Serial.println("[Sensor] ERROR: Read failed (NaN)");
        return;
    }

    ctx->state.temperature = t;
    ctx->state.humidity = h;
    ctx->state.sensorOk = true;

    // Only clear error if it was sensor-related
    if (ctx->state.errorCode == "SHT30_READ_ERROR" || 
        ctx->state.errorCode == "SHT30_NOT_FOUND")
    {
        ctx->state.errorCode = "NONE";
    }
}

bool SensorService::isOk() const
{
    if (ctx == nullptr) return false;
    return ctx->state.sensorOk;
}
