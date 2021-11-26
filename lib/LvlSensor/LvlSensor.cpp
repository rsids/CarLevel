#include <Wire.h>
#include <Arduino.h>
#include "LvlSensor.h"

const int MPU_addr = 0x68;
int16_t AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ;

int minVal = 265;
int maxVal = 402;

double x;
double y;
double z;

void LvlSensor::setup()
{
    Wire.begin();
    Wire.beginTransmission(MPU_addr);
    Wire.write(0x6B);
    Wire.write(0);
    Wire.endTransmission(true);
}
void LvlSensor::calc()
{
    Wire.beginTransmission(MPU_addr);
    Wire.write(0x3B);
    Wire.endTransmission(false);
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

float LvlSensor::readPitch()
{
    return x;
}

float LvlSensor::readRoll()
{
    return y;
}