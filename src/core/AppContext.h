/******************************************************************
 * Smart Egg Incubator - Application Context
 * Shared state container accessible by all services
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
