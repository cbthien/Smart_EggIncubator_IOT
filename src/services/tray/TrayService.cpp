/******************************************************************
 * Smart Egg Incubator - Tray Service Implementation
 * DTY50 220VAC motor: 1 relay ON/OFF, 2 limit switches
 * Features:
 *   - Debounce on limit switches (50ms)
 *   - Auto-home on boot if position unknown
 *   - Timeout protection (60s)
 *   - Auto-turn every 2 hours
 ******************************************************************/

#include "TrayService.h"
#include "../../config/Pins.h"
#include "../../config/Config.h"

TrayService::TrayService()
{
    ctx = nullptr;
    motorStartMs = 0;
    lastTurnMs = 0;
    waitingForFirstTurn = true;
    leftDebounceMs = 0;
    rightDebounceMs = 0;
    leftStable = false;
    rightStable = false;
}

void TrayService::begin(AppContext* context)
{
    ctx = context;

    // Configure relay pin
    pinMode(PIN_RELAY_MOTOR, OUTPUT);
    digitalWrite(PIN_RELAY_MOTOR, RELAY_OFF);  // Motor OFF at boot

    // Configure limit switch pins
    pinMode(PIN_LIMIT_LEFT, INPUT_PULLUP);
    pinMode(PIN_LIMIT_RIGHT, INPUT_PULLUP);

    // Detect initial position
    detectPosition();

    Serial.println("[Tray] Init OK - DTY50 1-relay, 2 limit switches, debounce");
    Serial.print("[Tray] Initial position: ");

    switch (ctx->state.trayPosition)
    {
    case TRAY_LEFT:  Serial.println("LEFT"); break;
    case TRAY_RIGHT: Serial.println("RIGHT"); break;
    default:         Serial.println("UNKNOWN"); break;
    }

    // Auto-home if position unknown after power loss
    homeIfNeeded();
}

void TrayService::loop()
{
    if (ctx == nullptr) return;

    updateLimitState();

    // Check for both limits active (error condition)
    if (ctx->state.limitLeft && ctx->state.limitRight)
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

    // Direct read (no debounce needed at boot - switches are stable)
    bool left = (digitalRead(PIN_LIMIT_LEFT) == LOW);
    bool right = (digitalRead(PIN_LIMIT_RIGHT) == LOW);

    ctx->state.limitLeft = left;
    ctx->state.limitRight = right;

    if (left && right)
    {
        ctx->state.trayPosition = TRAY_ERROR;
        ctx->state.errorCode = "BOTH_LIMIT_ACTIVE";
    }
    else if (left)
    {
        ctx->state.trayPosition = TRAY_LEFT;
    }
    else if (right)
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
        Serial.println("[Tray] Position UNKNOWN - auto-homing...");
        startTurn();  // Motor will run until it hits a limit switch
    }
}

void TrayService::startTurn()
{
    if (ctx == nullptr) return;
    if (ctx->state.trayTurning) return;
    if (ctx->state.trayPosition == TRAY_ERROR) return;

    // Start motor
    ctx->state.trayTurning = true;
    ctx->state.trayPosition = TRAY_MOVING;
    motorStartMs = millis();

    digitalWrite(PIN_RELAY_MOTOR, RELAY_ON);

    Serial.println("[Tray] Motor started - turning");
}

void TrayService::stopMotor()
{
    digitalWrite(PIN_RELAY_MOTOR, RELAY_OFF);

    if (ctx != nullptr)
    {
        ctx->state.trayTurning = false;
    }
}

void TrayService::handleMoving()
{
    if (ctx == nullptr) return;
    if (!ctx->state.trayTurning) return;

    // Check if we've reached a limit switch (motor completed 180°)
    if (ctx->state.limitLeft)
    {
        stopMotor();
        ctx->state.trayPosition = TRAY_LEFT;
        ctx->state.errorCode = "NONE";
        lastTurnMs = millis();

        Serial.println("[Tray] Reached LEFT - motor stopped");
    }
    else if (ctx->state.limitRight)
    {
        stopMotor();
        ctx->state.trayPosition = TRAY_RIGHT;
        ctx->state.errorCode = "NONE";
        lastTurnMs = millis();

        Serial.println("[Tray] Reached RIGHT - motor stopped");
    }
}

void TrayService::checkMotorTimeout()
{
    if (ctx == nullptr) return;
    if (!ctx->state.trayTurning) return;

    if (millis() - motorStartMs > ctx->settings.motorTimeoutMs)
    {
        stopMotor();
        ctx->state.trayPosition = TRAY_ERROR;
        ctx->state.errorCode = "MOTOR_TIMEOUT";
        ctx->state.safety = SAFETY_MOTOR_TIMEOUT;

        Serial.println("[Tray] ERROR: Motor timeout! Stopped.");
    }
}

void TrayService::scheduleAutoTurn()
{
    if (ctx == nullptr) return;
    if (!ctx->settings.autoTurnEnabled) return;
    if (ctx->state.trayTurning) return;
    if (ctx->state.trayPosition == TRAY_ERROR) return;

    // First turn: start timer from boot
    if (waitingForFirstTurn)
    {
        lastTurnMs = millis();
        waitingForFirstTurn = false;
        return;
    }

    // Check if it's time to turn
    if (millis() - lastTurnMs >= ctx->settings.turnIntervalMs)
    {
        Serial.println("[Tray] Auto-turn triggered");
        startTurn();
    }
}

// Debounced limit switch reading
bool TrayService::readLeftLimit()
{
    bool raw = (digitalRead(PIN_LIMIT_LEFT) == LOW);

    if (raw != leftStable)
    {
        leftDebounceMs = millis();
    }

    if (millis() - leftDebounceMs >= DEBOUNCE_MS)
    {
        leftStable = raw;
    }

    return leftStable;
}

bool TrayService::readRightLimit()
{
    bool raw = (digitalRead(PIN_LIMIT_RIGHT) == LOW);

    if (raw != rightStable)
    {
        rightDebounceMs = millis();
    }

    if (millis() - rightDebounceMs >= DEBOUNCE_MS)
    {
        rightStable = raw;
    }

    return rightStable;
}

void TrayService::updateLimitState()
{
    if (ctx == nullptr) return;

    ctx->state.limitLeft = readLeftLimit();
    ctx->state.limitRight = readRightLimit();
}
