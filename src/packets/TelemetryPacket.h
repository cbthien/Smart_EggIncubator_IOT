/******************************************************************
 * Smart Egg Incubator - Telemetry Packet
 * JSON telemetry structure definition
 * 
 * Telemetry is built directly in TelemetryService using ArduinoJson.
 * This file documents the expected JSON format.
 * 
 * Published to: egg_incubator/telemetry
 * 
 * Format:
 * {
 *   "temperature": 37.5,
 *   "humidity": 60.2,
 *   "sensor_ok": true,
 *   "heater_left": true,
 *   "heater_right": true,
 *   "fan_left_rpm": 1200,
 *   "fan_right_rpm": 1180,
 *   "fan_left_speed": 255,
 *   "fan_right_speed": 255,
 *   "fan_left_ok": true,
 *   "fan_right_ok": true,
 *   "tray_position": "LEFT",
 *   "tray_turning": false,
 *   "wifi": true,
 *   "mqtt": true,
 *   "safety": "OK",
 *   "error": "NONE",
 *   "uptime_s": 3600,
 *   "target_temp": 37.5,
 *   "fan_speed_set": 255
 * }
 ******************************************************************/

#ifndef TELEMETRY_PACKET_H
#define TELEMETRY_PACKET_H

// Telemetry is handled by TelemetryService::buildJson()
// This header serves as documentation only.

#endif
