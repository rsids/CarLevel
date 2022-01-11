#include <Arduino.h>
#include <BLEServer.h>
#include <BLEDevice.h>
#include "utils.h"
#include "LvlService.h"
#include "FmRadio.h"
#include "Comm.h"
#include "LvlSensor.h"
#include "I2CAddress.h"

bool deviceConnected = false;
bool levelEnabled = true;
bool radioEnabled = false;
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
FmRadio *fmRadio;
Comm *comm;

static std::vector<byte> dataOut;
const int OUTPUT_LEVEL = 0;
const int OUTPUT_FM = 1;

const int OUTPUT_LEVEL_INTERVAL = 10000;
const int OUTPUT_FM_INTERVAL = 1000000;

int output_mode = OUTPUT_LEVEL;
int output_interval = OUTPUT_LEVEL_INTERVAL;
int output_counter = 0;

float frequency = 0.0;

//---------- TEST VARS
bool isSeeking = false;
const int OUTPUT_NPN = 32;
const int BTN_SEEK_UP = 34;
const int BTN_SEEK_DOWN = 35;
const int BTN_MHZ_UP = 36;
const int BTN_MHZ_DOWN = 39;
const int BTN_DISABLE_LEVEL = 33;
const int LCD_UPDATE_FREQ = 128000;
const int SEEK_TIMEOUT = 4000;
int lcd_counter = 0;
int seek_timeout = 0;
int sleeping = 0;
//---------- END TEST VARS

void testSetup()
{
  pinMode(BTN_SEEK_UP, INPUT);
  pinMode(BTN_SEEK_DOWN, INPUT);
  pinMode(BTN_MHZ_DOWN, INPUT);
  pinMode(BTN_MHZ_UP, INPUT);
  pinMode(BTN_DISABLE_LEVEL, INPUT);
  pinMode(OUTPUT_NPN, OUTPUT);
  digitalWrite(OUTPUT_NPN, 0);
}

void testLoop()
{

  int seekDownState = digitalRead(BTN_SEEK_DOWN);
  int seekUpState = digitalRead(BTN_SEEK_UP);
  int mhzDownState = digitalRead(BTN_MHZ_DOWN);
  int mhzUpState = digitalRead(BTN_MHZ_UP);
  int sleepState = digitalRead(BTN_DISABLE_LEVEL);
  // levelEnabled = !digitalRead(OUTPUT_NPN);
  isSeeking = seekDownState || seekUpState || mhzDownState || mhzUpState;
  levelEnabled = !sleepState;
  // if (levelEnabled != sleepState)
  // {
  //   levelEnabled = sleepState;
  //   // fmRadio->setSleep(sleeping);
  // }
  if (isSeeking)
  {
    // levelEnabled = false;
    if (seek_timeout == 0)
    {
      seek_timeout++;
      Serial.println("Seeking...");
      Serial.print(seekDownState);
      Serial.print(seekUpState);
      Serial.print(mhzDownState);
      Serial.println(mhzUpState);
      frequency = fmRadio->getFrequency();
      // Serial.println(sleepState);
      if (seekDownState == HIGH)
      {
        fmRadio->scanDown(frequency);
      }
      else if (seekUpState == HIGH)
      {
        fmRadio->scanUp(frequency);
      }
      else if (mhzDownState == HIGH)
      {
        fmRadio->setFrequency(frequency - 0.1);
      }
      else if (mhzUpState == HIGH)
      {
        fmRadio->setFrequency(frequency + 0.1);
      }
    }
    else if (seek_timeout == SEEK_TIMEOUT)
    {
      seek_timeout = 0;
      // levelEnabled = true;
    }
  }
  else
  {
    seek_timeout = 0;
    // levelEnabled = true;
  }
}

void setup()
{
  Serial.begin(115200);
  // testSetup();
  // i2cAddressSetup(26, 25, 0);
  // i2cAddressSetup(21, 22, 0);
  pinMode(2, OUTPUT);
  if (radioEnabled)
  {
    fmRadio->setup(OUTPUT_NPN);
  }
  if (levelEnabled)
  {
    bleServer->setup(new LevelBLEServerCallbacks());
    lvlSensor->setup();
  }
  if (commEnabled)
  {
    Serial.println("Comm setup");
    comm->setup();
  }
}

void loop()
{
  // i2cAddressLoop();
  // read_sensors();
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
    // lvlMode = comm->
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

  if (radioEnabled)
  {
    fmRadio->loop();
    // Serial.printf("%d / %d\n", lcd_counter, LCD_UPDATE_FREQ);
    if (++lcd_counter == LCD_UPDATE_FREQ)
    {
      frequency = fmRadio->getFrequency();
      // lcd.cursor_off();
      // lcd.clear();
      // lcd.print(buffer);
      // lcd.setCursor(0, 1);
      // lcd.print("S: ");
      // lcd.print(fmRadio->getSignalStrength());
      // lcd.print(fmRadio->isStereo() ? "[ S ]" : "[ M ]");
      Serial.printf("FREQ: %03.2f\n", frequency);
      Serial.print(".");
      lcd_counter = 0;
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
      // Serial.write(pitch);
      // Serial.write(roll);
      // Serial.write(incline);
      // Serial.write(0);
      // Serial.write(0);
      // Serial.flush();
      dataOut.clear();
      dataOut.push_back(pitch);
      dataOut.push_back(roll);
      dataOut.push_back(incline);
      dataOut.push_back(0);
      dataOut.push_back(0x01);
      writeOut = true;
    }
    else
    {
      // Pass through data from tea
      // std::vector<byte> bytes = fmRadio->getData();
      // for (int i = 0; i < 5; i++)
      // {
      //   Serial.write(bytes[0]);
      // }
      // Serial.flush();
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