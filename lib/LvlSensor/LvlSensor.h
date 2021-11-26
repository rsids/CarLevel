#ifndef _LVL_SENSOR_
#define _LVL_SENSOR_
class LvlSensor
{

public:
    LvlSensor();
    void setup();
    void calc();
    float readPitch();
    float readRoll();
};
#endif