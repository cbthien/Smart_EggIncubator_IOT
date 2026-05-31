/******************************************************************
 * Smart Egg Incubator - Main Entry Point
 * ESP32-S3 IoT Controller
 * Visual Micro + Arduino Framework
 * 
 * Hardware:
 *   - ESP32-S3 Dev Module
 *   - 4CH Relay (LOW trigger): 2x Heater, 1x DTY50 Motor
 *   - 2x Cooler Master 4-pin Fan (PWM + Tach)
 *   - SHT30 Temperature/Humidity Sensor (I2C)
 *   - 2x Limit Switches
 *   - DTY50 220VAC Egg Turning Motor
 ******************************************************************/

#include "src/core/IncubatorApp.h"

// Function prototypes
void setup();
void loop();

IncubatorApp app;

void setup()
{
    Serial.begin(115200);
    delay(2000);  // Wait for serial monitor + ESP32 boot

    app.begin();
}

void loop()
{
    app.loop();
}
