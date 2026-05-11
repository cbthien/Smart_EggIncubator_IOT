#include "TrayService.h"
#include "../../config/Pins.h"

TrayService::TrayService()
{
    ctx = nullptr;
    motorStartMs = 0;
    lastTurnMs = 0;
}

void TrayService::begin(AppContext* context)
{
    ctx = context;

    pinMode(PIN_MOTOR_LEFT_CTRL, OUTPUT);
    pinMode(PIN_MOTOR_RIGHT_CTRL, OUTPUT);

    pinMode(PIN_LEFT_LIMIT, INPUT_PULLUP);
    pinMode(PIN_RIGHT_LIMIT, INPUT_PULLUP);

    stopMotor();
    detectPosition();

    Serial.println("[Tray] Init OK");
}

void TrayService::loop()
{
    if (ctx == nullptr) return;

    updateLimitState();

    if (ctx->state.leftLimit && ctx->state.rightLimit)
    {
        stopMotor();
        ctx->state.trayPosition = TRAY_ERROR;
        ctx->state.errorCode = "BOTH_LIMIT_ACTIVE";
        return;
    }

    handleMoving();
    checkMotorTimeout();
    scheduleAutoTurn();
}

void TrayService::detectPosition()
{
    if (ctx == nullptr) return;

    updateLimitState();

    if (ctx->state.leftLimit && ctx->state.rightLimit)
    {
        ctx->state.trayPosition = TRAY_ERROR;
        ctx->state.errorCode = "BOTH_LIMIT_ACTIVE";
    }
    else if (ctx->state.leftLimit)
    {
        ctx->state.trayPosition = TRAY_LEFT;
    }
    else if (ctx->state.rightLimit)
    {
        ctx->state.trayPosition = TRAY_RIGHT;
    }
    else
    {
        ctx->state.trayPosition = TRAY_UNKNOWN;
    }
}

void TrayService::homeIfNeeded()
{
    if (ctx == nullptr) return;

    if (ctx->state.trayPosition == TRAY_UNKNOWN)
    {
        ctx->state.trayPosition = TRAY_MOVING_LEFT;
        ctx->state.turning = true;
        motorStartMs = millis();
        moveLeft();

        Serial.println("[Tray] Homing to LEFT");
    }
}

void TrayService::startTurn()
{
    if (ctx == nullptr) return;
    if (ctx->state.turning) return;
    if (ctx->state.trayPosition == TRAY_ERROR) return;

    if (ctx->state.trayPosition == TRAY_LEFT)
    {
        ctx->state.trayPosition = TRAY_MOVING_RIGHT;
        ctx->state.turning = true;
        motorStartMs = millis();
        moveRight();
    }
    else if (ctx->state.trayPosition == TRAY_RIGHT)
    {
        ctx->state.trayPosition = TRAY_MOVING_LEFT;
        ctx->state.turning = true;
        motorStartMs = millis();
        moveLeft();
    }
    else
    {
        homeIfNeeded();
    }
}

void TrayService::moveLeft()
{
    digitalWrite(PIN_MOTOR_RIGHT_CTRL, LOW);
    digitalWrite(PIN_MOTOR_LEFT_CTRL, HIGH);
}

void TrayService::moveRight()
{
    digitalWrite(PIN_MOTOR_LEFT_CTRL, LOW);
    digitalWrite(PIN_MOTOR_RIGHT_CTRL, HIGH);
}

void TrayService::stopMotor()
{
    digitalWrite(PIN_MOTOR_LEFT_CTRL, LOW);
    digitalWrite(PIN_MOTOR_RIGHT_CTRL, LOW);

    if (ctx != nullptr)
    {
        ctx->state.turning = false;
    }
}

bool TrayService::leftLimitActive()
{
    return digitalRead(PIN_LEFT_LIMIT) == LOW;
}

bool TrayService::rightLimitActive()
{
    return digitalRead(PIN_RIGHT_LIMIT) == LOW;
}

void TrayService::updateLimitState()
{
    if (ctx == nullptr) return;

    ctx->state.leftLimit = leftLimitActive();
    ctx->state.rightLimit = rightLimitActive();
}

void TrayService::handleMoving()
{
    if (ctx == nullptr) return;
    if (!ctx->state.turning) return;

    if (ctx->state.trayPosition == TRAY_MOVING_LEFT && ctx->state.leftLimit)
    {
        stopMotor();
        ctx->state.trayPosition = TRAY_LEFT;
        ctx->state.errorCode = "NONE";
        lastTurnMs = millis();

        Serial.println("[Tray] Reached LEFT");
    }

    if (ctx->state.trayPosition == TRAY_MOVING_RIGHT && ctx->state.rightLimit)
    {
        stopMotor();
        ctx->state.trayPosition = TRAY_RIGHT;
        ctx->state.errorCode = "NONE";
        lastTurnMs = millis();

        Serial.println("[Tray] Reached RIGHT");
    }
}

void TrayService::checkMotorTimeout()
{
    if (ctx == nullptr) return;
    if (!ctx->state.turning) return;

    if (millis() - motorStartMs > ctx->settings.motorTimeoutMs)
    {
        stopMotor();
        ctx->state.trayPosition = TRAY_ERROR;
        ctx->state.errorCode = "MOTOR_TIMEOUT";

        Serial.println("[Tray] Motor timeout");
    }
}

void TrayService::scheduleAutoTurn()
{
    if (ctx == nullptr) return;
    if (!ctx->settings.autoTurnEnabled) return;
    if (ctx->state.turning) return;
    if (ctx->state.trayPosition == TRAY_ERROR) return;

    if (lastTurnMs == 0)
    {
        lastTurnMs = millis();
        return;
    }

    if (millis() - lastTurnMs >= ctx->settings.turnIntervalMs)
    {
        startTurn();
    }
}