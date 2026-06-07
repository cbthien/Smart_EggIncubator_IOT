/******************************************************************
 * Smart Egg Incubator - Sensor Service
 * SHT30 temperature and humidity sensor via I2C
 * Debug/demo version
 ******************************************************************/

#ifndef SENSOR_SERVICE_H
#define SENSOR_SERVICE_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_SHT31.h>

#include "../../core/AppContext.h"

class SensorService
{
public:
    SensorService();

    void begin(AppContext* context);
    void loop();

    bool isOk() const;

private:
    AppContext* ctx;
    Adafruit_SHT31 sht30;

    unsigned long lastReadMs;
    unsigned long lastRecoverMs;

    uint8_t currentAddress;
    uint8_t consecutiveFailCount;

    static const uint8_t SHT30_ADDR_PRIMARY = 0x44;
    static const uint8_t SHT30_ADDR_SECONDARY = 0x45;
    static const uint8_t MAX_FAIL_BEFORE_RECOVER = 3;
    static const unsigned long RECOVER_COOLDOWN_MS = 3000;

    void readSensor();

    bool initSensor();
    bool initSensorAt(uint8_t address);
    bool tryRead(float& temperature, float& humidity);

    void recoverSensor();
    void setSensorError(const String& code);
    void clearSensorError();
};

#endif
