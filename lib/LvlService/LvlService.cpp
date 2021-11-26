#include "LvlService.h"

BLEServer *pServer;
BLEService *pService;
BLECharacteristic *pCharacteristic;

void LvlService::setup(BLEServerCallbacks *callbacks)
{
    BLEDevice::init("CarLevel");
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(callbacks);
    pService = pServer->createService(SERVICE_UUID);
    pCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_READ);
    // BLEDescriptor *pDescriptor = new BLEDescriptor(BLEUUID((uint16_t)0x2A3D));
    pCharacteristic->addDescriptor(new BLE2902());
    pCharacteristic->setValue("0;0");
    pService->start();

    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06); // functions that help with iPhone connections issue
    pAdvertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();
}

void LvlService::startAdvertising()
{
    BLEDevice::startAdvertising();
}

void LvlService::update(char buff[9])
{
    pCharacteristic->setValue(buff);
    pCharacteristic->notify();
}