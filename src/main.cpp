#include <Arduino.h>
#include <BLEServer.h>
#include <BLEDevice.h>
#include "utils.h"
#include "LvlService.h"
#include "Comm.h"
#include "LvlSensor.h"
#include "I2CAddress.h"

bool deviceConnected = false;
bool levelEnabled = true;
bool commEnabled = true;

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
Comm *comm;

static std::vector<byte> dataOut;
const int OUTPUT_LEVEL = 0;
const int OUTPUT_FM = 1;

const int OUTPUT_LEVEL_INTERVAL = 100;
int output_mode = OUTPUT_LEVEL;
int output_interval = OUTPUT_LEVEL_INTERVAL;
int output_counter = 0;

void setup()
{
  Serial.begin(115200);

  pinMode(2, OUTPUT);

  if (levelEnabled)
  {
    bleServer->setup(new LevelBLEServerCallbacks());
    lvlSensor->setup();
  }
  if (commEnabled)
  {
    comm->setup();
  }
}

void loop()
{
  dataOut.clear();
  digitalWrite(2, LOW);
  // testLoop();
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
  bool writeOut = false;
  bool lvlMode = false;
  bool lvlConfigChanged = false;
  bool lvlOffsetRequested = false;

  int8_t offsetX, offsetY;
  bool invertX, invertY;

  if (commEnabled)
  {
    comm->loop();

    deviceConnected = comm->isSerialConnected();
    lvlMode = comm->isLevelMode();
    lvlConfigChanged = lvlMode && comm->configChanged();
    lvlOffsetRequested = lvlMode && comm->offsetRequested();
    if (lvlConfigChanged)
    {
      offsetX = comm->getOffsetX();
      offsetY = comm->getOffsetY();
      invertX = comm->isInvertedX();
      invertY = comm->isInvertedY();
    }
  }

  if (levelEnabled)
  {
    if (lvlConfigChanged)
    {
      lvlSensor->setOffsetX(offsetX);
      lvlSensor->setOffsetY(offsetY);
      lvlSensor->setInvertX(invertX);
      lvlSensor->setInvertY(invertY);
      lvlSensor->writeSettings();
    }
    lvlSensor->loop();
    if (lvlOffsetRequested)
    {
      offsetX = lvlSensor->getOffsetX();
      offsetY = lvlSensor->getOffsetY();
      invertX = lvlSensor->isInvertedX();
      invertY = lvlSensor->isInvertedY();
      dataOut[0] = offsetX;
      dataOut[1] = offsetY;
      dataOut[2] = invertX << 1 | invertY;
      dataOut[3] = 0;
      dataOut[4] = 0x03;
      writeOut = true;
      output_counter = 0;
    }
  }
  if (++output_counter == output_interval)
  {
    output_counter = 0;
    // Write out data
    if (output_mode == OUTPUT_LEVEL)
    {
      // Write out x / y / incline
      int8_t pitch = lvlSensor->readPitch();
      int8_t roll = lvlSensor->readRoll();
      int8_t incline = lvlSensor->readIncline();

      dataOut.clear();
      dataOut.push_back(pitch);
      dataOut.push_back(roll);
      dataOut.push_back(incline);
      dataOut.push_back(0);
      dataOut.push_back(0x01);
      writeOut = true;
    }
  }
  // }
  // bleServer->update(buffer2);
  // //Reset the loop timer
  // loop_timer = micros();
  if (commEnabled && writeOut)
  {
    comm->sendData(dataOut);
  }
}