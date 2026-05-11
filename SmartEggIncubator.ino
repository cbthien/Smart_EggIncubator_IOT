/******************************************************************
 Created with PROGRAMINO IDE for Arduino 
 Project     :SmartEggIncubator
 Libraries   :
 Author      :
 Description :
******************************************************************/


#include "src/core/IncubatorApp.h"

IncubatorApp app;

void setup()
{
    app.begin();
}

void loop()
{
    app.loop();
}