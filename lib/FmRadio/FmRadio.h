#ifndef _FM_RADIO_
#define _FM_RADIO_
class FmRadio
{
    int MhzToPLL(double mhz);
    double PLLToMhz(int pll);
    void scan(double mhz, bool dir);
    void i2cRead();
    void i2cWrite(byte a, byte b, byte c, byte d, byte e);

public:
    FmRadio();
    void setup(int pin);
    void loop();
    void setFrequency(double mhz);
    void scanUp(double mhz);
    void scanDown(double mhz);
    void enableStereo();
    void disableStereo();
    void enableStereoNoiceCancceling();
    void disableStereoNoiceCancceling();
    void store();
    void setSleep(int isSleeping);
    bool bandLimitReached();
    bool stationFound();
    bool isStereo();
    std::vector<byte> getData();
    double getFrequency();
    int getSignalStrength();
    // void scan();
    // void setVolume(int volume);
};
#endif