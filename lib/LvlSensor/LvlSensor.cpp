#include <Wire.h>
#include <Arduino.h>
#include "utils.h"
#include <EEPROM.h>
#include "LvlSensor.h"

const int MPU_addr = 0x68;
int16_t AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ;

int minVal = 265;
int maxVal = 402;

double x;
double y;
double z;

int pitches = 0;
int rolls = 0;
int loopCount = 0;
int inclineCount = 0;
int roll, pitch, incline;
const int MAX_INCLINE_COUNT = 1000;
const int MAX_COUNT = 100;
float inclines[MAX_INCLINE_COUNT] = {};

static int8_t offsetX, offsetY;
static bool invertX, invertY;

void LvlSensor::setup()
{
    Wire.begin();
    Wire.beginTransmission(MPU_addr);
    Wire.write(0x6B);
    Wire.write(0);
    Wire.endTransmission(true);

    EEPROM.begin(5);
    offsetX = EEPROM.readByte(2);
    offsetY = EEPROM.readByte(3);
    int inverted = EEPROM.readByte(3);
    invertX = inverted & 2;
    invertY = inverted & 1;
    EEPROM.end();
}

void LvlSensor::loop()
{
    calc();
    int loopPitch = x;
    if (loopPitch < 90)
    {
        loopPitch = 360 + loopPitch;
    }

    int loopRoll = y;
    if (loopRoll < 90)
    {
        loopRoll = 360 + loopRoll;
    }
    pitches += loopPitch;
    rolls += loopRoll;
    loopCount++;
    inclines[inclineCount] = loopPitch;
    inclineCount++;
    if (inclineCount == MAX_INCLINE_COUNT)
    {
        inclineCount = 0;
    }
    if (loopCount == MAX_COUNT)
    {
        pitches /= MAX_COUNT;
        if (pitches > 360)
        {
            pitches -= 360;
        }

        rolls /= MAX_COUNT;
        if (rolls > 360)
        {
            rolls -= 360;
        }

        pitch = clamp(pitches, -127, 127);
        roll = clamp(rolls, -127, 127);
        incline = clamp(getIncline(), -127, 127);
        loopCount = 0;
        pitches = 0;
        rolls = 0;
    }
}

void LvlSensor::calc()
{
    Wire.beginTransmission(MPU_addr);
    Wire.write(0x3B);
    Wire.endTransmission(true);
    Wire.requestFrom(MPU_addr, 14);
    AcX = Wire.read() << 8 | Wire.read();
    AcY = Wire.read() << 8 | Wire.read();
    AcZ = Wire.read() << 8 | Wire.read();
    int xAng = map(AcX, minVal, maxVal, -90, 90);
    int yAng = map(AcY, minVal, maxVal, -90, 90);
    int zAng = map(AcZ, minVal, maxVal, -90, 90);

    x = RAD_TO_DEG * (atan2(-yAng, -zAng) + PI);
    y = RAD_TO_DEG * (atan2(-xAng, -zAng) + PI);
    z = RAD_TO_DEG * (atan2(-yAng, -xAng) + PI);
}

float LvlSensor::getIncline()
{
    float incline = 0;
    for (int i = 0; i < MAX_INCLINE_COUNT; i++)
    {
        incline += inclines[i];
    }
    return tan(deg2rad(incline / MAX_INCLINE_COUNT)) * 100;
}

int8_t LvlSensor::readPitch()
{
    return pitch;
}

int8_t LvlSensor::readRoll()
{
    return roll;
}

int8_t LvlSensor::readIncline()
{
    return incline;
}

int8_t LvlSensor::getOffsetX()
{
    return offsetX;
};

int8_t LvlSensor::getOffsetY()
{
    return offsetY;
};

bool LvlSensor::isInvertedX()
{
    return invertX;
};

bool LvlSensor::isInvertedY()
{
    return invertY;
};

void LvlSensor::setOffsetX(int8_t offset)
{
    offsetX = offset;
};

void LvlSensor::setOffsetY(int8_t offset)
{
    offsetY = offset;
};

void LvlSensor::setInvertX(bool inverted)
{
    invertX = inverted;
};

void LvlSensor::setInvertY(bool inverted)
{
    invertY = inverted;
};

void LvlSensor::writeSettings()
{
    EEPROM.begin(5);
    EEPROM.writeByte(2, getOffsetX());
    EEPROM.writeByte(3, getOffsetY());
    EEPROM.writeByte(4, isInvertedX() << 1 | isInvertedY());
    EEPROM.commit();
    EEPROM.end();
}
