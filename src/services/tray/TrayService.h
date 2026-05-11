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

    void detectPosition();
    void homeIfNeeded();
    void startTurn();
    void stopMotor();

private:
    AppContext* ctx;
    unsigned long motorStartMs;
    unsigned long lastTurnMs;

    bool leftLimitActive();
    bool rightLimitActive();

    void moveLeft();
    void moveRight();
    void updateLimitState();
    void checkMotorTimeout();
    void handleMoving();
    void scheduleAutoTurn();
};

#endif