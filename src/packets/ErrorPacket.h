/******************************************************************
 * Smart Egg Incubator - Error/Alarm Packet
 * Published when safety events occur
 * 
 * Topic: egg_incubator/alarm
 * 
 * Error codes:
 *   NONE              - No error
 *   OVER_TEMP         - Temperature >= maxTemp (39.5°C)
 *   SENSOR_FAIL       - SHT30 read error or not found
 *   SHT30_NOT_FOUND   - Sensor not detected on I2C
 *   SHT30_READ_ERROR  - Sensor read returned NaN
 *   FAN_STALL         - Fan RPM below threshold while running
 *   MOTOR_TIMEOUT     - Tray motor exceeded timeout
 *   BOTH_LIMIT_ACTIVE - Both limit switches active (wiring error)
 ******************************************************************/

#ifndef ERROR_PACKET_H
#define ERROR_PACKET_H

// Alarms are published by SafetyService via MqttService
// This header serves as documentation only.

#endif
