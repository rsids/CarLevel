#include <Wire.h>
#include <Arduino.h>
#include <EEPROM.h>
#include <math.h>
#include "FmRadio.h"

const int TEA_ADDR = 0x60;
const int STORE_TIMEOUT = 6400000;
const int SCAN_TIMEOUT_LONG = 6400000;
const int SCAN_TIMEOUT_SHORT = 5000;

const double fIF = 225 * pow(10, 3);

int scanTimeout = SCAN_TIMEOUT_LONG;
int snc = 2;
unsigned int storeCounter = 0;
unsigned int scanCounter = 0;
unsigned int pll = 0;
int pllFromEeprom = 0;
std::vector<byte> response = {0, 0, 0, 0, 0};

void FmRadio::setup()
{
    Wire.begin();
    EEPROM.begin(4);
    pllFromEeprom = EEPROM.readInt(0);
    setFrequency(PLLToMhz(pllFromEeprom));
    EEPROM.end();
}

void FmRadio::loop()
{
    if (++storeCounter == STORE_TIMEOUT)
    {
        store();
    }

    scanCounter++;
    if (scanCounter == scanTimeout)
    {
        i2cRead();
        scanCounter = 0;
        if (stationFound())
        {
            scanTimeout = SCAN_TIMEOUT_LONG;
        }
    }
}

bool FmRadio::bandLimitReached()
{
    return response[0] & (1 << 6);
}

void FmRadio::disableStereoNoiceCancceling()
{
    snc = 0;
}

void FmRadio::enableStereoNoiceCancceling()
{
    snc = 2;
}

std::vector<byte> FmRadio::getData()
{
    return response;
}

double FmRadio::getFrequency()
{
    return PLLToMhz(pll);
}

int FmRadio::getSignalStrength()
{
    return response[3] >> 4;
}

bool FmRadio::isStereo()
{
    return response[2] >> 7;
}

void FmRadio::i2cRead()
{
    Wire.requestFrom(TEA_ADDR, 5);
    for (int i = 0; i < 5; i++)
    {
        response[i] = Wire.read();
    }

    // Strip off first two bits, add byte b
    pll = (response[0] & 0x3f) << 8 | response[1];
}

void FmRadio::i2cWrite(byte a, byte b, byte c, byte d, byte e)
{
    Wire.beginTransmission(TEA_ADDR);
    Wire.write(a);
    Wire.write(b);
    Wire.write(c);
    Wire.write(d);
    Wire.write(e);
    Wire.endTransmission();

    storeCounter = 0;
    scanCounter = 0;
    scanTimeout = SCAN_TIMEOUT_SHORT;
}

void FmRadio::scan(double mhz, bool dir)
{
    byte a, b, c;
    a = 0x40 | (pll >> 8);
    b = MhzToPLL(mhz) & 0xFF;
    c = 0x70 | (dir << 7);
    i2cWrite(a, b, c, 0x10 | snc, 0x40);
}

void FmRadio::scanDown(double mhz)
{
    scan(mhz - .1, false);
}

void FmRadio::scanUp(double mhz)
{
    scan(mhz + .1, true);
}

void FmRadio::setFrequency(double mhz)
{
    int newPll = MhzToPLL(mhz);
    byte a, b;
    a = newPll >> 8;
    b = newPll & 0xFF;
    i2cWrite(a, b, 0x10, 0x10, 0x40);
}

bool FmRadio::stationFound()
{
    return (response[0] & (1 << 7));
}

void FmRadio::store()
{
    if (pll != pllFromEeprom)
    {
        EEPROM.begin(4);
        EEPROM.writeInt(0, pll);
        EEPROM.commit();
        EEPROM.end();
    }
}

int FmRadio::MhzToPLL(double mhz)
{
    return floor((4 * ((mhz * pow(10, 6) + fIF))) / 32768);
}

double FmRadio::PLLToMhz(int p)
{
    double fPll = p * 1.0;
    return ((fPll * 32768 / 4) - fIF) / pow(10, 6);
}