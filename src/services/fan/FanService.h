/******************************************************************
 * Smart Egg Incubator - Fan Service
 * Controls 2x 4-pin PWM fans with RPM monitoring via Tach
 ******************************************************************/

#ifndef FAN_SERVICE_H
#define FAN_SERVICE_H

#include <Arduino.h>
#include "../../core/AppContext.h"

class FanService
{
public:
    FanService();

    void begin(AppContext* context);
    void loop();

    void setSpeed(uint8_t speed);       // 0-255
    void setSpeedPercent(uint8_t pct);   // 0-100%
    void forceOff();

    uint16_t getLeftRpm() const;
    uint16_t getRightRpm() const;

    // ISR callbacks (must be static)
    static void IRAM_ATTR onLeftTachPulse();
    static void IRAM_ATTR onRightTachPulse();

private:
    AppContext* ctx;
    unsigned long lastRpmReadMs;

    void setupPwm();
    void setupTach();
    void readRpm();
    void checkFanHealth();

    // Tach pulse counters (volatile for ISR)
    static volatile unsigned long leftPulseCount;
    static volatile unsigned long rightPulseCount;
};

#endif
