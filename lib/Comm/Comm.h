#ifndef _COMM_
#define _COMM_
class Comm
{

public:
    Comm();
    void setup();
    void loop();
    std::vector<byte> getRaw();
    bool isSerialConnected();
    bool isLevelMode();
    bool offsetRequested();
    bool configChanged();
    bool isInvertedX();
    bool isInvertedY();
    int8_t getOffsetX();
    int8_t getOffsetY();
    int8_t getOffsetIncline();
    void sendData(std::vector<byte> data);
};
#endif