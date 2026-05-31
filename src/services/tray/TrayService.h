/******************************************************************
 * Smart Egg Incubator - Tray Service
 * DTY50 220VAC motor control via 1 relay + 2 limit switches
 * Features: debounce, auto-home on boot, timeout protection
 ******************************************************************/

#ifndef TRAY_SERVICE_H
#define TRAY_SERVICE_H

#include <Arduino.h>
#include "../../core/AppContext.h"

class TrayService
{
public:
    TrayService();

    void begin(AppContext* context);
    void loop();

    void startTurn();
    void stopMotor();
    void detectPosition();
    void homeIfNeeded();

private:
    AppContext* ctx;
    unsigned long motorStartMs;
    unsigned long lastTurnMs;
    bool waitingForFirstTurn;

    // Debounce
    static const unsigned long DEBOUNCE_MS = 50;
    unsigned long leftDebounceMs;
    unsigned long rightDebounceMs;
    bool leftStable;
    bool rightStable;

    bool readLeftLimit();
    bool readRightLimit();
    void updateLimitState();
    void handleMoving();
    void checkMotorTimeout();
    void scheduleAutoTurn();
};

#endif
