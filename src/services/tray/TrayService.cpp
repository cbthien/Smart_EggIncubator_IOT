/******************************************************************
 * Smart Egg Incubator - Tray Service Implementation
 * DTY50 220VAC motor: 1 relay ON/OFF, 2 limit switches
 * Debug/demo version:
 *   - Auto-home disabled at boot
 *   - Direction-aware tray movement
 *   - Fixed debounce
 *   - Serial logs for GPIO limit, motor start/stop, timeout
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
    leftLastRaw = false;
    rightLastRaw = false;
    prevLimitLeft = false;
    prevLimitRight = false;

    startPosition = TRAY_UNKNOWN;
    targetPosition = TRAY_UNKNOWN;
    leftReleased = true;
    rightReleased = true;
}

void TrayService::begin(AppContext* context)
{
    ctx = context;

    pinMode(PIN_RELAY_MOTOR, OUTPUT);
    digitalWrite(PIN_RELAY_MOTOR, RELAY_OFF);

    pinMode(PIN_LIMIT_LEFT, INPUT_PULLUP);
    pinMode(PIN_LIMIT_RIGHT, INPUT_PULLUP);

    delay(50);
    detectPosition();

    leftStable = ctx->state.limitLeft;
    rightStable = ctx->state.limitRight;
    leftLastRaw = ctx->state.limitLeft;
    rightLastRaw = ctx->state.limitRight;
    prevLimitLeft = ctx->state.limitLeft;
    prevLimitRight = ctx->state.limitRight;

    Serial.println("[Tray] Init OK - DTY50 1 relay + 2 limit switches");
    Serial.print("[Tray] PIN motor relay: GPIO"); Serial.println(PIN_RELAY_MOTOR);
    Serial.print("[Tray] PIN limit left/right: GPIO"); Serial.print(PIN_LIMIT_LEFT); Serial.print(" / GPIO"); Serial.println(PIN_LIMIT_RIGHT);
    Serial.print("[Tray] Initial limit L/R: "); Serial.print(ctx->state.limitLeft ? "PRESSED" : "OPEN"); Serial.print(" / "); Serial.println(ctx->state.limitRight ? "PRESSED" : "OPEN");
    Serial.print("[Tray] Initial position: "); Serial.println(positionToString(ctx->state.trayPosition));
    Serial.println("[Tray] Auto-home DISABLED for demo. Motor will not move on boot.");
    Serial.println("[Tray] To test motor, send MQTT command: {\"cmd\":\"turn_tray\"}");
}

void TrayService::loop()
{
    if (ctx == nullptr) return;

    updateLimitState();

    if (ctx->state.limitLeft && ctx->state.limitRight)
    {
        stopMotor();
        ctx->state.trayPosition = TRAY_ERROR;
        ctx->state.errorCode = "BOTH_LIMIT_ACTIVE";
        Serial.println("[Tray] ERROR: BOTH limits active. Check NO/NC wiring. Use COM->GND, NO->GPIO.");
        return;
    }

    handleMoving();
    checkMotorTimeout();
    scheduleAutoTurn();
}

void TrayService::detectPosition()
{
    if (ctx == nullptr) return;

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
        ctx->state.errorCode = "NONE";
    }
    else if (right)
    {
        ctx->state.trayPosition = TRAY_RIGHT;
        ctx->state.errorCode = "NONE";
    }
    else
    {
        ctx->state.trayPosition = TRAY_UNKNOWN;
        ctx->state.errorCode = "NONE";
    }
}

void TrayService::homeIfNeeded()
{
    if (ctx == nullptr) return;

    if (ctx->state.trayPosition == TRAY_UNKNOWN)
    {
        Serial.println("[Tray] Manual home requested - position unknown, starting motor...");
        startTurn();
    }
}

void TrayService::startTurn()
{
    if (ctx == nullptr) return;
    if (ctx->state.trayTurning)
    {
        Serial.println("[Tray] startTurn ignored: tray already moving");
        return;
    }
    if (ctx->state.trayPosition == TRAY_ERROR)
    {
        Serial.print("[Tray] startTurn blocked: tray error = "); Serial.println(ctx->state.errorCode);
        return;
    }

    updateLimitState();

    if (ctx->state.limitLeft && ctx->state.limitRight)
    {
        stopMotor();
        ctx->state.trayPosition = TRAY_ERROR;
        ctx->state.errorCode = "BOTH_LIMIT_ACTIVE";
        Serial.println("[Tray] Cannot start: both limits active");
        return;
    }

    startPosition = ctx->state.trayPosition;

    if (startPosition == TRAY_LEFT)
    {
        targetPosition = TRAY_RIGHT;
        leftReleased = false;
        rightReleased = true;
    }
    else if (startPosition == TRAY_RIGHT)
    {
        targetPosition = TRAY_LEFT;
        rightReleased = false;
        leftReleased = true;
    }
    else
    {
        targetPosition = TRAY_UNKNOWN;
        leftReleased = true;
        rightReleased = true;
    }

    ctx->state.trayTurning = true;
    ctx->state.trayPosition = TRAY_MOVING;
    motorStartMs = millis();

    digitalWrite(PIN_RELAY_MOTOR, RELAY_ON);

    Serial.print("[Tray] MOTOR ON. From="); Serial.print(positionToString(startPosition));
    Serial.print(" Target="); Serial.print(positionToString(targetPosition));
    Serial.print(" Limit L/R="); Serial.print(ctx->state.limitLeft ? "PRESSED" : "OPEN");
    Serial.print("/"); Serial.println(ctx->state.limitRight ? "PRESSED" : "OPEN");
}

void TrayService::stopMotor()
{
    digitalWrite(PIN_RELAY_MOTOR, RELAY_OFF);

    if (ctx != nullptr)
    {
        if (ctx->state.trayTurning)
        {
            Serial.println("[Tray] MOTOR OFF");
        }
        ctx->state.trayTurning = false;
    }
}

void TrayService::handleMoving()
{
    if (ctx == nullptr) return;
    if (!ctx->state.trayTurning) return;

    if (!ctx->state.limitLeft) leftReleased = true;
    if (!ctx->state.limitRight) rightReleased = true;

    if (targetPosition == TRAY_RIGHT)
    {
        if (leftReleased && ctx->state.limitRight)
        {
            stopMotor();
            ctx->state.trayPosition = TRAY_RIGHT;
            ctx->state.errorCode = "NONE";
            lastTurnMs = millis();
            Serial.println("[Tray] Reached RIGHT - OK");
        }
    }
    else if (targetPosition == TRAY_LEFT)
    {
        if (rightReleased && ctx->state.limitLeft)
        {
            stopMotor();
            ctx->state.trayPosition = TRAY_LEFT;
            ctx->state.errorCode = "NONE";
            lastTurnMs = millis();
            Serial.println("[Tray] Reached LEFT - OK");
        }
    }
    else
    {
        if (ctx->state.limitLeft)
        {
            stopMotor();
            ctx->state.trayPosition = TRAY_LEFT;
            ctx->state.errorCode = "NONE";
            lastTurnMs = millis();
            Serial.println("[Tray] Homed LEFT - OK");
        }
        else if (ctx->state.limitRight)
        {
            stopMotor();
            ctx->state.trayPosition = TRAY_RIGHT;
            ctx->state.errorCode = "NONE";
            lastTurnMs = millis();
            Serial.println("[Tray] Homed RIGHT - OK");
        }
    }
}

void TrayService::checkMotorTimeout()
{
    if (ctx == nullptr) return;
    if (!ctx->state.trayTurning) return;

    unsigned long elapsed = millis() - motorStartMs;

    if (elapsed > ctx->settings.motorTimeoutMs)
    {
        stopMotor();
        ctx->state.trayPosition = TRAY_ERROR;
        ctx->state.errorCode = "MOTOR_TIMEOUT";
        ctx->state.safety = SAFETY_MOTOR_TIMEOUT;

        Serial.print("[Tray] ERROR: MOTOR TIMEOUT after "); Serial.print(elapsed / 1000); Serial.println("s");
        Serial.println("[Tray] Check: KM12 COM must go to ESP32 GND, NO goes to GPIO17/GPIO18");
    }
}

void TrayService::scheduleAutoTurn()
{
    if (ctx == nullptr) return;
    if (!ctx->settings.autoTurnEnabled) return;
    if (ctx->state.trayTurning) return;
    if (ctx->state.trayPosition == TRAY_ERROR) return;

    if (waitingForFirstTurn)
    {
        lastTurnMs = millis();
        waitingForFirstTurn = false;
        Serial.println("[Tray] Auto-turn timer started");
        return;
    }

    if (millis() - lastTurnMs >= ctx->settings.turnIntervalMs)
    {
        Serial.println("[Tray] Auto-turn triggered");
        startTurn();
    }
}

bool TrayService::readLeftLimit()
{
    bool raw = (digitalRead(PIN_LIMIT_LEFT) == LOW);

    if (raw != leftLastRaw)
    {
        leftLastRaw = raw;
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

    if (raw != rightLastRaw)
    {
        rightLastRaw = raw;
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
    // Log changes
    if (ctx->state.limitLeft != prevLimitLeft || ctx->state.limitRight != prevLimitRight)
    {
        Serial.print("[Tray] LIMIT changed L/R = ");
        Serial.print(ctx->state.limitLeft ? "PRESSED" : "OPEN");
        Serial.print(" / ");
        Serial.println(ctx->state.limitRight ? "PRESSED" : "OPEN");

        // If any manual button/limit is pressed, immediately activate relay (active LOW)
        if (ctx->state.limitLeft || ctx->state.limitRight)
        {
            digitalWrite(PIN_RELAY_MOTOR, RELAY_ON);
            Serial.print("[Tray] Manual limit detected -> MOTOR ON (GPIO"); Serial.print(PIN_RELAY_MOTOR); Serial.println(")");
        }
        else
        {
            digitalWrite(PIN_RELAY_MOTOR, RELAY_OFF);
            Serial.print("[Tray] Limits released -> MOTOR OFF (GPIO"); Serial.print(PIN_RELAY_MOTOR); Serial.println(")");
        }

        prevLimitLeft = ctx->state.limitLeft;
        prevLimitRight = ctx->state.limitRight;
    }
}

const char* TrayService::positionToString(TrayPosition pos)
{
    switch (pos)
    {
    case TRAY_LEFT:    return "LEFT";
    case TRAY_RIGHT:   return "RIGHT";
    case TRAY_MOVING:  return "MOVING";
    case TRAY_ERROR:   return "ERROR";
    case TRAY_UNKNOWN: return "UNKNOWN";
    default:           return "INVALID";
    }
}
