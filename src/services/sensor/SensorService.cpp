/******************************************************************
 * Smart Egg Incubator - Sensor Service Implementation
 * SHT30 I2C temperature/humidity sensor
 * Debug/demo version:
 *   - Prints I2C pins/address
 *   - Retry on NaN
 *   - Recover I2C after repeated failures
 ******************************************************************/

#include "SensorService.h"
#include "../../config/Pins.h"
#include "../../config/Config.h"

SensorService::SensorService()
{
    ctx = nullptr;
    lastReadMs = 0;
    lastRecoverMs = 0;
    currentAddress = SENSOR_I2C_ADDRESS;
    consecutiveFailCount = 0;
}

void SensorService::begin(AppContext* context)
{
    ctx = context;

    Serial.println("[Sensor] Begin SHT30");
    Serial.print("[Sensor] SDA/SCL = GPIO"); Serial.print(PIN_SHT30_SDA); Serial.print(" / GPIO"); Serial.println(PIN_SHT30_SCL);

    Wire.begin(PIN_SHT30_SDA, PIN_SHT30_SCL);
    Wire.setClock(100000);

    if (!initSensor())
    {
        if (ctx != nullptr)
        {
            ctx->state.sensorOk = false;
            ctx->state.errorCode = "SHT30_NOT_FOUND";
        }
        Serial.println("[Sensor] ERROR: SHT30 NOT FOUND at 0x44 or 0x45");
        Serial.println("[Sensor] Check: VCC=3V3, GND common, SDA/SCL not swapped, cable short");
        return;
    }

    if (ctx != nullptr)
    {
        ctx->state.sensorOk = true;
        clearSensorError();
    }

    Serial.print("[Sensor] SHT30 OK at address 0x");
    Serial.println(currentAddress, HEX);
}

void SensorService::loop()
{
    if (ctx == nullptr) return;

    unsigned long now = millis();
    if (now - lastReadMs >= SENSOR_READ_INTERVAL_MS)
    {
        lastReadMs = now;
        readSensor();
    }
}

void SensorService::readSensor()
{
    if (ctx == nullptr) return;

    float temperature = NAN;
    float humidity = NAN;

    if (tryRead(temperature, humidity))
    {
        ctx->state.temperature = temperature;
        ctx->state.humidity = humidity;
        ctx->state.sensorOk = true;
        consecutiveFailCount = 0;
        clearSensorError();

        Serial.print("[Sensor] OK T="); Serial.print(temperature);
        Serial.print("C H="); Serial.print(humidity); Serial.println("%");
        return;
    }

    Serial.println("[Sensor] Read failed once, retrying...");
    delay(50);

    if (tryRead(temperature, humidity))
    {
        ctx->state.temperature = temperature;
        ctx->state.humidity = humidity;
        ctx->state.sensorOk = true;
        consecutiveFailCount = 0;
        clearSensorError();
        Serial.print("[Sensor] OK after retry T="); Serial.print(temperature);
        Serial.print("C H="); Serial.print(humidity); Serial.println("%");
        return;
    }

    consecutiveFailCount++;
    Serial.print("[Sensor] ERROR: Read failed. failCount="); Serial.println(consecutiveFailCount);

    if (consecutiveFailCount >= MAX_FAIL_BEFORE_RECOVER)
    {
        recoverSensor();
        if (ctx->state.sensorOk)
        {
            return;
        }
    }

    ctx->state.sensorOk = false;
    setSensorError("SHT30_READ_ERROR");
}

bool SensorService::tryRead(float& temperature, float& humidity)
{
    temperature = sht30.readTemperature();
    humidity = sht30.readHumidity();

    if (isnan(temperature) || isnan(humidity)) return false;
    if (temperature < -20.0 || temperature > 80.0) return false;
    if (humidity < 0.0 || humidity > 100.0) return false;
    return true;
}

bool SensorService::initSensor()
{
    if (initSensorAt(SENSOR_I2C_ADDRESS)) return true;
    if (SENSOR_I2C_ADDRESS != SHT30_ADDR_PRIMARY && initSensorAt(SHT30_ADDR_PRIMARY)) return true;
    if (SENSOR_I2C_ADDRESS != SHT30_ADDR_SECONDARY && initSensorAt(SHT30_ADDR_SECONDARY)) return true;
    return false;
}

bool SensorService::initSensorAt(uint8_t address)
{
    Serial.print("[Sensor] Trying address 0x"); Serial.println(address, HEX);
    if (sht30.begin(address))
    {
        currentAddress = address;
        consecutiveFailCount = 0;
        return true;
    }
    return false;
}

void SensorService::recoverSensor()
{
    if (ctx == nullptr) return;

    unsigned long now = millis();
    if (now - lastRecoverMs < RECOVER_COOLDOWN_MS)
    {
        Serial.println("[Sensor] Recovery skipped: cooldown");
        return;
    }

    lastRecoverMs = now;
    Serial.println("[Sensor] Recovering I2C bus...");

    Wire.end();
    delay(100);
    Wire.begin(PIN_SHT30_SDA, PIN_SHT30_SCL);
    Wire.setClock(100000);
    delay(100);

    if (initSensor())
    {
        Serial.print("[Sensor] Recovery OK at 0x"); Serial.println(currentAddress, HEX);
        consecutiveFailCount = 0;
        ctx->state.sensorOk = true;
        clearSensorError();
    }
    else
    {
        Serial.println("[Sensor] Recovery failed. Check wire length/noise/power.");
        ctx->state.sensorOk = false;
        setSensorError("SHT30_NOT_FOUND");
    }
}

void SensorService::setSensorError(const String& code)
{
    if (ctx == nullptr) return;
    ctx->state.errorCode = code;
}

void SensorService::clearSensorError()
{
    if (ctx == nullptr) return;
    if (ctx->state.errorCode == "SHT30_READ_ERROR" || ctx->state.errorCode == "SHT30_NOT_FOUND")
    {
        ctx->state.errorCode = "NONE";
    }
}

bool SensorService::isOk() const
{
    if (ctx == nullptr) return false;
    return ctx->state.sensorOk;
}
