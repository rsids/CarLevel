#ifndef _LVL_SERVICE_
#define _LVL_SERVICE_
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

#define SERVICE_UUID "ac159216-381a-11ec-8d3d-0242ac130003"
#define CHARACTERISTIC_UUID "b08cd61a-381a-11ec-8d3d-0242ac130003"
#define DESCRIPTOR_UUID "72ccbe10-cfd1-4d31-9d67-50b76db62e95"

class LvlService
{
public:
    LvlService();
    void setup(BLEServerCallbacks *callbacks);
    void startAdvertising();
    void update(char buff[9]);
};

#endif