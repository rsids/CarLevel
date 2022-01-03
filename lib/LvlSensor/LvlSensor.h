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
    int readPitch();
    int readRoll();
    int readIncline();
};
#endif