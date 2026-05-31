/******************************************************************
 * Smart Egg Incubator - Storage Service
 * Persistent settings storage using ESP32 NVS (Preferences)
 ******************************************************************/

#ifndef STORAGE_SERVICE_H
#define STORAGE_SERVICE_H

#include <Arduino.h>
#include <Preferences.h>
#include "../../core/AppContext.h"

class StorageService
{
public:
    StorageService();

    void begin(AppContext* context);
    void loadSettings();
    void saveSettings();

private:
    AppContext* ctx;
    Preferences prefs;
};

#endif
