#include "StorageService.h"

StorageService::StorageService()
{
    ctx = nullptr;
}

void StorageService::begin(AppContext* context)
{
    ctx = context;

    prefs.begin("incubator", false);

    Serial.println("[Storage] Init OK");
}

void StorageService::loadSettings()
{
    if (ctx == nullptr) return;

    ctx->settings.targetTemp = prefs.getFloat("targetTemp", 37.5);
    ctx->settings.hysteresis = prefs.getFloat("hysteresis", 0.2);
    ctx->settings.maxTemp = prefs.getFloat("maxTemp", 39.5);

    ctx->settings.turnIntervalMs = prefs.getULong("turnMs", 2UL * 60UL * 60UL * 1000UL);
    ctx->settings.motorTimeoutMs = prefs.getULong("motorTimeout", 30000);

    ctx->settings.autoTurnEnabled = prefs.getBool("autoTurn", true);

    Serial.println("[Storage] Settings loaded");
}

void StorageService::saveSettings()
{
    if (ctx == nullptr) return;

    prefs.putFloat("targetTemp", ctx->settings.targetTemp);
    prefs.putFloat("hysteresis", ctx->settings.hysteresis);
    prefs.putFloat("maxTemp", ctx->settings.maxTemp);

    prefs.putULong("turnMs", ctx->settings.turnIntervalMs);
    prefs.putULong("motorTimeout", ctx->settings.motorTimeoutMs);

    prefs.putBool("autoTurn", ctx->settings.autoTurnEnabled);

    Serial.println("[Storage] Settings saved");
}