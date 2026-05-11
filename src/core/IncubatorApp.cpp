/******************************************************************
 Created with PROGRAMINO IDE for Arduino 
 Project     :
 Libraries   :
 Author      :
 Description :
******************************************************************/

#include "IncubatorApp.h"

IncubatorApp::IncubatorApp()
{
}

void IncubatorApp::begin()
{
    Serial.begin(115200);
    delay(1000);

    printBootInfo();

    storage.begin(&context);
    storage.loadSettings();

    sensor.begin(&context);
    heater.begin(&context);
    fan.begin(&context);
    tray.begin(&context);

    safety.begin(&context, &heater, &tray);
    telemetry.begin(&context);

    tray.homeIfNeeded();

    Serial.println("[App] Begin complete");
}

void IncubatorApp::loop()
{
    sensor.loop();

    safety.loop();

    heater.loop();
    fan.loop();
    tray.loop();

    telemetry.loop();

    delay(10);
}

void IncubatorApp::printBootInfo()
{
    Serial.println();
    Serial.println("================================");
    Serial.println(" Smart Egg Incubator");
    Serial.println(" ESP32-S3 IoT Controller");
    Serial.println("================================");

    Serial.println("System booting...");
}