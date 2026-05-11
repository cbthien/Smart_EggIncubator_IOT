/******************************************************************
 Created with PROGRAMINO IDE for Arduino 
 Libraries   :
 Author      :
 Description :
******************************************************************/


#ifndef APP_CONTEXT_H
#define APP_CONTEXT_H

#include "Models.h"

struct AppContext
{
    IncubatorState state;
    IncubatorSettings settings;
};

#endif