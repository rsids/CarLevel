#include <Wire.h>
#include <Arduino.h>
#include "FmRadio.h"
#include <EEPROM.h>
#include <math.h>

const int TEA_ADDR = 0x60;
const int WRITE_MODE = 0x61;
const int READ_MODE = 0x60;
const int MAX_COUNTS = 6400000;

const double fIF = 225 * pow(10, 3);

int signalStrength = 0;
int snc = 2;
unsigned int counter = 0;
unsigned int pll = 0;
int pllFromEeprom = 0;

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
    if (++counter == MAX_COUNTS)
    {
        FmRadio::store();
    }
}

void FmRadio::setFrequency(double mhz)
{
    counter = 0;
    int pll = MhzToPLL(mhz);
    byte a, b;
    a = pll >> 8;
    b = pll & 0xFF;
    i2cWrite(a, b, 0x10, 0x10, 0x40);
}

void FmRadio::scanUp(double mhz)
{
    scan(mhz, true);
}
void FmRadio::scanDown(double mhz)
{
    scan(mhz, false);
}

void FmRadio::scan(double mhz, bool dir)
{
    counter = 0;
    int pll = MhzToPLL(mhz);
    byte a, b, c;
    a = 0x40 | (pll >> 8);
    b = pll & 0xFF;
    c = 0x70 | (dir << 7);
    // Serial.println(a, HEX);
    // Serial.println(a, HEX);
    // Serial.println(c, HEX);
    i2cWrite(a, b, c, 0x10 | snc, 0x40);
}

void FmRadio::store()
{
    if (pll != pllFromEeprom)
    {
        EEPROM.begin(4);
        Serial.print("Storing freq into db ");
        Serial.println(pll);
        EEPROM.writeInt(0, pll);
        EEPROM.commit();
        EEPROM.end();
    }
}

double FmRadio::getFrequency()
{
    Wire.requestFrom(TEA_ADDR, 5);
    byte a = Wire.read();
    byte b = Wire.read();
    byte c = Wire.read();
    byte d = Wire.read();
    // byte e = Wire.read();
    // Strip off first two bits, add byte b
    pll = (a & 0x3f) << 8 | b;
    bool found = a & (1 << 7);
    bool blf = a & (1 << 6);
    // Serial.print("Found: ");
    // Serial.print(found);
    // Serial.print(" / BLF: ");
    // Serial.println(blf);
    // Serial.println("*****");
    // Serial.println(c, BIN);
    // Serial.println(d, BIN);
    if (!found)
    {
        signalStrength = 0;
        return 0.0;
    }
    signalStrength = ((d & 0xf0) >> 4);
    // Serial.println(a, HEX);
    // Serial.println(b, HEX);
    // Serial.println(c, HEX);
    // Serial.println(d, HEX);
    // Serial.println(e, HEX);
    // Serial.println("------");
    return PLLToMhz(pll);
}

int FmRadio::MhzToPLL(double mhz)
{
    return floor((4 * ((mhz * pow(10, 6) + fIF))) / 32768);
}

int FmRadio::getSignalStrength()
{
    return signalStrength;
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
}

double FmRadio::PLLToMhz(int pll)
{
    double fPll = pll * 1.0;
    return ((fPll * 32768 / 4) - fIF) / pow(10, 6);
}

void FmRadio::enableStereoNoiceCancceling()
{
    snc = 2;
}
void FmRadio::disableStereoNoiceCancceling()
{
    snc = 0;
}