#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

#define SERVICE_UUID "ac159216-381a-11ec-8d3d-0242ac130003"
#define CHARACTERISTIC_UUID "b08cd61a-381a-11ec-8d3d-0242ac130003"
#define DESCRIPTOR_UUID "72ccbe10-cfd1-4d31-9d67-50b76db62e95"

class Service
{
private:
    /* data */

    BLEServer *pServer;
    BLEService *pService;
    BLECharacteristic *pCharacteristic;

public:
    bool isConnected = false;
    void setup(BLEServerCallbacks *callbacks)
    {
        BLEDevice::init("CarLevel");
        this->pServer = BLEDevice::createServer();
        this->pServer->setCallbacks(callbacks);
        this->pService = this->pServer->createService(SERVICE_UUID);
        this->pCharacteristic = this->pService->createCharacteristic(
            CHARACTERISTIC_UUID,
            BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_READ);
        // BLEDescriptor *pDescriptor = new BLEDescriptor(BLEUUID((uint16_t)0x2A3D));
        this->pCharacteristic->addDescriptor(new BLE2902());
        this->pCharacteristic->setValue("0;0");
        this->pService->start();

        BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
        pAdvertising->addServiceUUID(SERVICE_UUID);
        pAdvertising->setScanResponse(true);
        pAdvertising->setMinPreferred(0x06); // functions that help with iPhone connections issue
        pAdvertising->setMinPreferred(0x12);
        BLEDevice::startAdvertising();
    }
    void startAdvertising()
    {
        BLEDevice::startAdvertising();
    }

    void update(char buff[9])
    {
        this->pCharacteristic->setValue(buff);
        this->pCharacteristic->notify();
    }
};
