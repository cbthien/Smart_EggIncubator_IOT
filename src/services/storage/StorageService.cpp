/******************************************************************
 * Smart Egg Incubator - Storage Service Implementation
 * Load/save settings from ESP32 NVS
 ******************************************************************/

#include "StorageService.h"
#include "../../config/Config.h"

StorageService::StorageService()
{
    ctx = nullptr;
}

void StorageService::begin(AppContext* context)
{
    ctx = context;

    prefs.begin(NVS_NAMESPACE, false);

    Serial.println("[Storage] Init OK");
}

void StorageService::loadSettings()
{
    if (ctx == nullptr) return;

    ctx->settings.targetTemp = prefs.getFloat("targetTemp", DEFAULT_TARGET_TEMP);
    ctx->settings.hysteresis = prefs.getFloat("hysteresis", DEFAULT_HYSTERESIS);
    ctx->settings.maxTemp = prefs.getFloat("maxTemp", DEFAULT_MAX_TEMP);

    ctx->settings.turnIntervalMs = prefs.getULong("turnMs", DEFAULT_TURN_INTERVAL_MS);
    ctx->settings.motorTimeoutMs = prefs.getULong("motorTimeout", DEFAULT_MOTOR_TIMEOUT_MS);

    ctx->settings.autoTurnEnabled = prefs.getBool("autoTurn", true);
    ctx->settings.fanSpeed = prefs.getUChar("fanSpeed", DEFAULT_FAN_SPEED);
    ctx->settings.fanStallCheckEnabled = prefs.getBool("fanCheck", true);

    // Incubation
    ctx->settings.incubationActive = prefs.getBool("incubActive", false);
    ctx->settings.incubationStartMs = prefs.getULong("incubStart", 0);

    Serial.println("[Storage] Settings loaded:");
    Serial.print("  Target temp: "); Serial.println(ctx->settings.targetTemp);
    Serial.print("  Hysteresis: "); Serial.println(ctx->settings.hysteresis);
    Serial.print("  Max temp: "); Serial.println(ctx->settings.maxTemp);
    Serial.print("  Turn interval: "); Serial.print(ctx->settings.turnIntervalMs / 60000); Serial.println(" min");
    Serial.print("  Motor timeout: "); Serial.print(ctx->settings.motorTimeoutMs / 1000); Serial.println(" s");
    Serial.print("  Fan speed: "); Serial.println(ctx->settings.fanSpeed);
    Serial.print("  Fan stall check: "); Serial.println(ctx->settings.fanStallCheckEnabled ? "ON" : "OFF");
    Serial.print("  Auto turn: "); Serial.println(ctx->settings.autoTurnEnabled ? "ON" : "OFF");
    Serial.print("  Incubation active: "); Serial.println(ctx->settings.incubationActive ? "YES" : "NO");
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
    prefs.putUChar("fanSpeed", ctx->settings.fanSpeed);
    prefs.putBool("fanCheck", ctx->settings.fanStallCheckEnabled);

    // Incubation
    prefs.putBool("incubActive", ctx->settings.incubationActive);
    prefs.putULong("incubStart", ctx->settings.incubationStartMs);

    Serial.println("[Storage] Settings saved");
}
