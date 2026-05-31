/******************************************************************
 * Smart Egg Incubator - Pin Configuration
 * Board: ESP32-S3 Dev Module
 * Relay: 4CH 12V Opto-isolated, LOW Trigger
 ******************************************************************/

#ifndef PINS_H
#define PINS_H

// ============================================================
// RELAY LOGIC (LOW Trigger - Opto Isolated)
// ============================================================
#define RELAY_ON  LOW
#define RELAY_OFF HIGH

// ============================================================
// RELAY OUTPUTS
// ============================================================
#define PIN_RELAY_HEATER_LEFT   4   // Relay CH1 - Heater Left (12V)
#define PIN_RELAY_HEATER_RIGHT  5   // Relay CH2 - Heater Right (12V)
#define PIN_RELAY_MOTOR         6   // Relay CH3 - DTY50 Motor (220VAC)
#define PIN_RELAY_SPARE         7   // Relay CH4 - Reserved

// ============================================================
// FAN PWM OUTPUTS (Cooler Master DF1202512B2UN, 4-pin)
// ============================================================
#define PIN_FAN_LEFT_PWM        12  // Fan Left - PWM speed control
#define PIN_FAN_RIGHT_PWM       14  // Fan Right - PWM speed control

// ============================================================
// FAN TACH INPUTS (RPM feedback)
// ============================================================
#define PIN_FAN_LEFT_TACH       11  // Fan Left - Tach pulse input
#define PIN_FAN_RIGHT_TACH      13  // Fan Right - Tach pulse input

// ============================================================
// LIMIT SWITCHES (INPUT_PULLUP, active LOW)
// ============================================================
#define PIN_LIMIT_LEFT          17  // Left limit switch
#define PIN_LIMIT_RIGHT         18  // Right limit switch

// ============================================================
// I2C - SHT30 Sensor
// ============================================================
#define PIN_SHT30_SDA           8
#define PIN_SHT30_SCL           9

// ============================================================
// ALL RELAY PINS (for boot safety initialization)
// ============================================================
static const int ALL_RELAY_PINS[] = {
    PIN_RELAY_HEATER_LEFT,
    PIN_RELAY_HEATER_RIGHT,
    PIN_RELAY_MOTOR,
    PIN_RELAY_SPARE
};
static const int NUM_RELAY_PINS = 4;

#endif
