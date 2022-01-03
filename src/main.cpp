#include <Arduino.h>
#include <BLEServer.h>
#include <BLEDevice.h>
#include <math.h>
#include "utils.h"
#include "LvlService.h"
#include "FmRadio.h"
#include "LvlSensor.h"
#include "Comm.h"
// #include "I2CAddress.h"
#include <LiquidCrystal_I2C.h>

bool deviceConnected = false;
bool levelEnabled = false;
bool radioEnabled = true;

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

int loop_count = 0;
int incline_count = 0;
const int MAX_INCLINE_COUNT = 1000;
const int MAX_COUNT = 100;
float pitches = 0;
float rolls = 0;
float frequency = 0.0;
float inclines[MAX_INCLINE_COUNT] = {};

//---------- TEST VARS
bool isSeeking = false;
const int BTN_SEEK_UP = 34;
const int BTN_SEEK_DOWN = 35;
const int BTN_MHZ_UP = 36;
const int BTN_MHZ_DOWN = 39;
LiquidCrystal_I2C lcd(0x27, 16, 2);
const int LCD_UPDATE_FREQ = 1280000;
const int SEEK_TIMEOUT = 4000;
int lcd_counter = 0;
int seek_timeout = 0;
//---------- END TEST VARS

float getIncline()
{
  float incline = 0;
  for (int i = 0; i < MAX_INCLINE_COUNT; i++)
  {
    incline += inclines[i];
  }
  return tan(deg2rad(incline / MAX_INCLINE_COUNT)) * 100;
}

void testSetup()
{
  pinMode(BTN_SEEK_UP, INPUT);
  pinMode(BTN_SEEK_DOWN, INPUT);
  pinMode(BTN_MHZ_DOWN, INPUT);
  pinMode(BTN_MHZ_UP, INPUT);
  lcd.init(); // initialize the lcd
  lcd.backlight();
}

void testLoop()
{

  int seekDownState = digitalRead(BTN_SEEK_DOWN);
  int seekUpState = digitalRead(BTN_SEEK_UP);
  int mhzDownState = digitalRead(BTN_MHZ_DOWN);
  int mhzUpState = digitalRead(BTN_MHZ_UP);
  isSeeking = seekDownState || seekUpState || mhzDownState || mhzUpState;
  if (isSeeking)
  {

    if (seek_timeout == 0)
    {
      lcd.clear();
      lcd.print("Seeking...");
      lcd.cursor_on();
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
    else if (++seek_timeout == SEEK_TIMEOUT)
    {
      seek_timeout = 0;
    }
  }
  else
  {
    seek_timeout = 0;
  }
}

void setup()
{
  // i2cAddressSetup();
  pinMode(2, OUTPUT);
  testSetup();
  Serial.begin(115200);
  Serial.println("Start CarLevelFM");
  comm->setup();
  if (levelEnabled)
  {
    bleServer->setup(new LevelBLEServerCallbacks());
    lvlSensor->setup();
  }
  if (radioEnabled)
  {
    fmRadio->setup();
    // frequency = 97.5;
    // fmRadio->setFrequency(frequency);
  }
}

void loop()
{
  // i2cAddressLoop();
  // read_sensors();
  digitalWrite(2, LOW);
  testLoop();
  comm->loop();
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
  if (radioEnabled)
  {
    fmRadio->loop();
    if (++lcd_counter == LCD_UPDATE_FREQ)
    {

      frequency = fmRadio->getFrequency();
      char buffer[14];
      sprintf(buffer, "%03.2f", frequency);
      lcd.cursor_off();
      lcd.clear();
      lcd.print(buffer);
      lcd.setCursor(0, 1);
      lcd.print("S: ");
      lcd.print(fmRadio->getSignalStrength());
      // Serial.println(buffer);
      Serial.print(".");
      lcd_counter = 0;
    }
  }
  if (levelEnabled)
  {

    lvlSensor->calc();
    int pitch = lvlSensor->readPitch();
    if (pitch < 90)
    {
      pitch = 360 + pitch;
    }

    int roll = lvlSensor->readRoll();
    if (roll < 90)
    {
      roll = 360 + roll;
    }
    pitches += pitch;
    rolls += roll;
    loop_count++;
    inclines[incline_count] = pitch;
    incline_count++;
    if (incline_count == MAX_INCLINE_COUNT)
    {
      incline_count = 0;
    }
    if (loop_count == MAX_COUNT)
    {
      char buffer[14];
      pitches /= MAX_COUNT;
      if (pitches > 360)
      {
        pitches -= 360;
      }

      rolls /= MAX_COUNT;
      if (rolls > 360)
      {
        rolls -= 360;
      }

      pitches = clamp(pitches, -127, 127);
      rolls = clamp(rolls, -127, 127);
      float incline = clamp(getIncline(), -127, 127);

      // sprintf(buffer, "%03.0f;%03.0f;%03.0f", pitches, rolls, incline);
      // Serial.println(buffer);

      if (deviceConnected)
      {
        bleServer->update(buffer);
      }
      loop_count = 0;
      pitches = 0;
      rolls = 0;
      // delay(3); // stop the program for some time
    }
  }
  // }
  // bleServer->update(buffer2);
  // //Reset the loop timer
  // loop_timer = micros();
}