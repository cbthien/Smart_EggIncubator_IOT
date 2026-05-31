/******************************************************************
 * Smart Egg Incubator - Sensor Service
 * SHT30 temperature and humidity sensor via I2C
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

    void readSensor();
};

#endif
