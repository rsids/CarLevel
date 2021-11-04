#include <Arduino.h>
// #include "BluetoothSerial.h"
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

// BT CLASSIC
// #if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
// #error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
// #endif

// BluetoothSerial SerialBT;

#define SERVICE_UUID "ac159216-381a-11ec-8d3d-0242ac130003"
#define CHARACTERISTIC_UUID "b08cd61a-381a-11ec-8d3d-0242ac130003"
#define DESCRIPTOR_UUID "72ccbe10-cfd1-4d31-9d67-50b76db62e95"
//#define CHARACTERISTIC_UUID 0x2763

BLEServer *pServer;
BLEService *pService;
BLECharacteristic *pCharacteristic;

bool deviceConnected = false;

class LevelBLEServerCallbacks : public BLEServerCallbacks
{
  void onConnect(BLEServer *pServer)
  {
    deviceConnected = true;
  };

  void onDisconnect(BLEServer *pServer)
  {
    deviceConnected = false;
    BLEDevice::startAdvertising();
  };
};

int xPin = 36;
int yPin = 33;
float val = 0;
int maxVal = 4095;
float scale = 1;
void setup()
{
  pinMode(2, OUTPUT);
  Serial.begin(115200);
  // SerialBT.begin("CarLevel"); //Bluetooth device name
  Serial.println("Start app");
  scale = 360.0 / maxVal;

  BLEDevice::init("CarLevel");
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new LevelBLEServerCallbacks());
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
  Serial.println("Characteristic defined! Now you can read it in your phone!");
}

void loop()
{
  digitalWrite(2, LOW);
  if (deviceConnected)
  {
    digitalWrite(2, HIGH);
    char buffer2[9];
    // put your main code here, to run repeatedly:
    float valX = ((float)analogRead(xPin) * scale) - 180;
    float valY = ((float)analogRead(yPin) * scale) - 180;
    sprintf(buffer2, "%0.0f;%0.0f", valX, valY);
    Serial.println(buffer2);

    pCharacteristic->setValue(buffer2);
    pCharacteristic->notify();

    delay(500); // stop the program for some time
  }
}