#include <Arduino.h>
#include <BLEServer.h>
#include <BLEDevice.h>

#include "LvlService.h"
#include "LvlSensor.h"
// #include "AdaFrtSensor.h"
// #include "I2CAddress.h"

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

LvlService *bleServer;
LvlSensor *lvlSensor;
// AdaFrtSensor *aSensor;

int loop_count = 0;
int max_count = 100;
float pitches = 0;
float rolls = 0;

void setup()
{
  // i2cAddressSetup();
  pinMode(2, OUTPUT);
  Serial.begin(115200);
  // SerialBT.begin("CarLevel"); //Bluetooth device name
  Serial.println("Start app");

  bleServer->setup(new LevelBLEServerCallbacks());
  lvlSensor->setup();
  // aSensor->setup();
}

void loop()
{
  // read_sensors();
  digitalWrite(2, LOW);

  if (deviceConnected)
  {
    digitalWrite(2, HIGH);
  }
  // if (deviceConnected)
  // {
  //   digitalWrite(2, HIGH);
  //   // put your main code here, to run repeatedly:
  //   float valX = ((float)analogRead(xPin) * scale) - 180;
  //   float valY = ((float)analogRead(yPin) * scale) - 180;
  //   sprintf(buffer2, "%0.0f;%0.0f", valX, valY);
  //   Serial.println(buffer2);

  //   pCharacteristic->setValue(buffer2);
  //   pCharacteristic->notify();

  lvlSensor->calc();
  float pitch = lvlSensor->readPitch();
  if (pitch < 90)
  {
    pitch = 360 + pitch;
  }

  float roll = lvlSensor->readRoll();
  if (roll < 90)
  {
    roll = 360 + roll;
  }
  pitches += pitch;
  rolls += roll;
  loop_count++;
  if (loop_count == max_count)
  {
    char buffer[18];
    pitches /= max_count;
    if (pitches > 360)
    {
      pitches -= 360;
    }

    rolls /= max_count;
    if (rolls > 360)
    {
      rolls -= 360;
    }
    sprintf(buffer, "%0.2f;%0.2f", pitches, rolls);
    Serial.println(buffer);

    if (deviceConnected)
    {
      bleServer->update(buffer);
    }
    loop_count = 0;
    pitches = 0.0;
    rolls = 0.0;
    // delay(3); // stop the program for some time
  }

  // aSensor->read();

  // i2cAddressLoop();
  // }
  // bleServer->update(buffer2);
  // //Reset the loop timer
  // loop_timer = micros();
}