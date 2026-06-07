/******************************************************************
 * Smart Egg Incubator - Main Entry Point
 * ESP32-S3 IoT Controller
 * Visual Micro + Arduino Framework
 ******************************************************************/

#include "src/core/IncubatorApp.h"

// Tạo đối tượng ứng dụng chính
IncubatorApp app;

void setup()
{
    Serial.begin(115200);
    delay(2000);

    Serial.println();
    Serial.println("Starting Smart Egg Incubator...");

    app.begin();

    Serial.println("Smart Egg Incubator started.");
}

void loop()
{
    app.loop();
}