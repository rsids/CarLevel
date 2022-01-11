#include <Wire.h>
#include <Arduino.h>
#include "Comm.h"

static const int MODE_LVL = 0x80;
static const int MODE_FM = 0x0;

static const byte CMD_SET_OFFSET = 1 << 6;
static const byte CMD_GET_OFFSET = 1 << 5;
static const byte CMD_SET_INVERT_X = 1 << 4;
static const byte CMD_SET_INVERT_Y = 1 << 2;

static bool isLvlMode = true;
static bool isOffsetRequested = false;
static bool isConfigChanged = false;
static bool invertedX, invertedY;

static byte dataBuffer[5];
static int8_t offsetX, offsetY, offsetIncline;

void Comm::setup()
{
    offsetX = -3;
    offsetY = 3;
    offsetIncline = 1;
}
void Comm::loop()
{
    if (Serial.available() > 0)
    {
        Serial.readBytes(dataBuffer, 5);
        byte cmd = dataBuffer[0];
        isLvlMode = cmd & MODE_LVL;
        if (isLvlMode)
        {
            isConfigChanged = cmd & (CMD_SET_OFFSET | CMD_SET_INVERT_X | CMD_SET_INVERT_Y);
            isOffsetRequested = (cmd & CMD_GET_OFFSET);
            if (cmd & CMD_SET_OFFSET)
            {
                // Set Offset
                offsetX = dataBuffer[1];
                offsetY = dataBuffer[2];
                offsetIncline = dataBuffer[3];
            }

            if (cmd & CMD_SET_INVERT_X)
            {
                invertedX = cmd & (1 << 3);
            }

            if (cmd & CMD_SET_INVERT_Y)
            {

                invertedY = cmd & (1 << 1);
            }
        }
    }
}

bool Comm::isLevelMode()
{
    return isLvlMode;
}

bool Comm::offsetRequested()
{
    return isOffsetRequested;
}

bool Comm::configChanged()
{
    return isConfigChanged;
}

bool Comm::isInvertedX()
{
    return invertedX;
}
bool Comm::isInvertedY()
{
    return invertedY;
}

int8_t Comm::getOffsetX()
{
    return offsetX;
}
int8_t Comm::getOffsetY()
{
    return offsetY;
}
int8_t Comm::getOffsetIncline()
{
    return offsetIncline;
}

void Comm::sendData(std::vector<byte> data)
{
    byte bytes[5] = {data[0], data[1], data[2], data[3], data[4]};
    isConfigChanged = false;
    isOffsetRequested = false;
    Serial.write(bytes, 5);
    Serial.flush();
}

std::vector<byte> Comm::getRaw()
{
    std::vector<byte> raw;
    raw.reserve(5);
    for (int i = 0; i < 5; i++)
    {
        raw.push_back(dataBuffer[i]);
        dataBuffer[i] = 0;
    }
    return raw;
}