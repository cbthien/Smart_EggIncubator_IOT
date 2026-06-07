/******************************************************************
 * Smart Egg Incubator - Fan Service Implementation
 * PWM speed control + Tach RPM monitoring for 2x 4-pin fans
 ******************************************************************/

#include "FanService.h"
#include "../../config/Pins.h"
#include "../../config/Config.h"

// Static volatile counters for ISR
volatile unsigned long FanService::leftPulseCount = 0;
volatile unsigned long FanService::rightPulseCount = 0;

FanService::FanService()
{
    ctx = nullptr;
    lastRpmReadMs = 0;
}

void FanService::begin(AppContext* context)
{
    ctx = context;

    setupPwm();
    setupTach();

    // Start fans at configured speed (default 100%)
    setSpeed(ctx->settings.fanSpeed);

    Serial.println("[Fan] Init OK - 2x PWM + Tach");
    Serial.print("[Fan] PWM L/R GPIO: "); Serial.print(PIN_FAN_LEFT_PWM); Serial.print(" / "); Serial.println(PIN_FAN_RIGHT_PWM);
    Serial.print("[Fan] Tach L/R GPIO: "); Serial.print(PIN_FAN_LEFT_TACH); Serial.print(" / "); Serial.println(PIN_FAN_RIGHT_TACH);
    Serial.print("[Fan] Initial speed duty: "); Serial.println(ctx->settings.fanSpeed);
}

void FanService::loop()
{
    if (ctx == nullptr) return;

    unsigned long now = millis();

    if (now - lastRpmReadMs >= FAN_RPM_READ_INTERVAL_MS)
    {
        lastRpmReadMs = now;
        readRpm();
        checkFanHealth();
    }
}

void FanService::setupPwm()
{
    // Configure LEDC PWM for fan control (ESP32-S3)
    ledcAttach(PIN_FAN_LEFT_PWM, FAN_PWM_FREQUENCY, FAN_PWM_RESOLUTION);
    ledcAttach(PIN_FAN_RIGHT_PWM, FAN_PWM_FREQUENCY, FAN_PWM_RESOLUTION);

    Serial.println("[Fan] PWM configured: 25kHz, 8-bit");
}

void FanService::setupTach()
{
    // Configure Tach pins as input with pullup
    pinMode(PIN_FAN_LEFT_TACH, INPUT_PULLUP);
    pinMode(PIN_FAN_RIGHT_TACH, INPUT_PULLUP);

    // Attach interrupts for pulse counting
    attachInterrupt(digitalPinToInterrupt(PIN_FAN_LEFT_TACH), onLeftTachPulse, FALLING);
    attachInterrupt(digitalPinToInterrupt(PIN_FAN_RIGHT_TACH), onRightTachPulse, FALLING);

    Serial.println("[Fan] Tach interrupts attached");
}

void FanService::setSpeed(uint8_t speed)
{
    if (ctx != nullptr)
    {
        ctx->state.fanLeftSpeed = speed;
        ctx->state.fanRightSpeed = speed;
    }

    ledcWrite(PIN_FAN_LEFT_PWM, speed);
    ledcWrite(PIN_FAN_RIGHT_PWM, speed);

    static uint8_t lastLoggedSpeed = 255;
    static bool firstSpeedLog = true;
    if (firstSpeedLog || lastLoggedSpeed != speed)
    {
        firstSpeedLog = false;
        lastLoggedSpeed = speed;
        Serial.print("[Fan] Speed duty set: "); Serial.println(speed);
    }
}

void FanService::setSpeedPercent(uint8_t pct)
{
    if (pct > 100) pct = 100;
    uint8_t duty = (uint8_t)((uint16_t)pct * 255 / 100);
    setSpeed(duty);
}

void FanService::forceOff()
{
    setSpeed(0);
}

void FanService::readRpm()
{
    // Disable interrupts briefly to read counters
    noInterrupts();
    unsigned long leftCount = leftPulseCount;
    unsigned long rightCount = rightPulseCount;
    leftPulseCount = 0;
    rightPulseCount = 0;
    interrupts();

    // Calculate RPM: (pulses / pulses_per_rev) * (60000 / interval_ms)
    uint16_t leftRpm = (uint16_t)((leftCount * 60000UL) / 
                       (FAN_TACH_PULSES_PER_REV * FAN_RPM_READ_INTERVAL_MS));
    uint16_t rightRpm = (uint16_t)((rightCount * 60000UL) / 
                        (FAN_TACH_PULSES_PER_REV * FAN_RPM_READ_INTERVAL_MS));

    if (ctx != nullptr)
    {
        ctx->state.fanLeftRpm = leftRpm;
        ctx->state.fanRightRpm = rightRpm;

        Serial.print("[Fan] RPM L/R = ");
        Serial.print(leftRpm);
        Serial.print(" / ");
        Serial.println(rightRpm);
    }
}

void FanService::checkFanHealth()
{
    if (ctx == nullptr) return;

    // Only check if fans should be running
    if (ctx->state.fanLeftSpeed > 0)
    {
        ctx->state.fanLeftOk = (ctx->state.fanLeftRpm >= FAN_STALL_THRESHOLD_RPM);
    }
    else
    {
        ctx->state.fanLeftOk = true;  // Not expected to spin
    }

    if (ctx->state.fanRightSpeed > 0)
    {
        ctx->state.fanRightOk = (ctx->state.fanRightRpm >= FAN_STALL_THRESHOLD_RPM);
    }
    else
    {
        ctx->state.fanRightOk = true;
    }

    static bool lastLeftOk = true;
    static bool lastRightOk = true;
    if (ctx->state.fanLeftOk != lastLeftOk || ctx->state.fanRightOk != lastRightOk)
    {
        lastLeftOk = ctx->state.fanLeftOk;
        lastRightOk = ctx->state.fanRightOk;
        Serial.print("[Fan] Health changed L/R = ");
        Serial.print(ctx->state.fanLeftOk ? "OK" : "FAIL");
        Serial.print(" / ");
        Serial.println(ctx->state.fanRightOk ? "OK" : "FAIL");
        if (!ctx->state.fanLeftOk || !ctx->state.fanRightOk)
        {
            Serial.println("[Fan] If RPM=0, check tach wire, common GND, and tach pull-up to 3.3V only");
        }
    }
}

uint16_t FanService::getLeftRpm() const
{
    if (ctx == nullptr) return 0;
    return ctx->state.fanLeftRpm;
}

uint16_t FanService::getRightRpm() const
{
    if (ctx == nullptr) return 0;
    return ctx->state.fanRightRpm;
}

// ISR handlers
void IRAM_ATTR FanService::onLeftTachPulse()
{
    leftPulseCount++;
}

void IRAM_ATTR FanService::onRightTachPulse()
{
    rightPulseCount++;
}
