#ifndef _LVL_SENSOR_
#define _LVL_SENSOR_
class LvlSensor
{
    float getIncline();

public:
    LvlSensor();
    void setup();
    void loop();
    void calc();
    int8_t readPitch();
    int8_t readRoll();
    int8_t readIncline();

    int8_t getOffsetX();
    int8_t getOffsetY();
    bool isInvertedX();
    bool isInvertedY();
    void setOffsetX(int8_t offset);
    void setOffsetY(int8_t offset);
    void setInvertX(bool inverted);
    void setInvertY(bool inverted);
    void writeSettings();
};
#endif