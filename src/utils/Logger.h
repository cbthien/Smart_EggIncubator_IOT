/******************************************************************
 * Smart Egg Incubator - Logger Utility
 * Simple serial logger with module prefix
 * 
 * Usage:
 *   LOG_INFO("Heater", "Temperature reached target");
 *   LOG_WARN("Fan", "RPM below threshold");
 *   LOG_ERROR("Sensor", "SHT30 read failed");
 ******************************************************************/

#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>

#define LOG_INFO(module, msg)  do { Serial.print("["); Serial.print(module); Serial.print("] "); Serial.println(msg); } while(0)
#define LOG_WARN(module, msg)  do { Serial.print("["); Serial.print(module); Serial.print("] WARN: "); Serial.println(msg); } while(0)
#define LOG_ERROR(module, msg) do { Serial.print("["); Serial.print(module); Serial.print("] ERROR: "); Serial.println(msg); } while(0)

#define LOG_INFO_VAL(module, msg, val) do { Serial.print("["); Serial.print(module); Serial.print("] "); Serial.print(msg); Serial.println(val); } while(0)

#endif
