#include <Arduino.h>
#include <BLEServer.h>
#include <BLEDevice.h>
#include "utils.h"
#include "LvlService.h"
#include "FmRadio.h"
#include "LvlSensor.h"
#include "Comm.h"
// #include "I2CAddress.h"
#include <LiquidCrystal_I2C.h>

bool deviceConnected = false;
bool levelEnabled = true;
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
  levelEnabled = (mhzDownState == HIGH);
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
        // fmRadio->setFrequency(frequency + 0.1);
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
  comm->setup();
  if (levelEnabled)
  {
    bleServer->setup(new LevelBLEServerCallbacks());
    lvlSensor->setup();
  }
  if (radioEnabled)
  {
    fmRadio->setup();
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
      lcd.print(fmRadio->isStereo() ? "[ S ]" : "[ M ]");
      // Serial.println(buffer);
      // Serial.print(".");
      lcd_counter = 0;
    }
  }
  if (levelEnabled)
  {
    lvlSensor->loop();
  }

  if (++output_counter == output_interval)
  {
    output_counter = 0;
    // Write out data
    if (output_mode == OUTPUT_LEVEL)
    {
      // Write out x / y / incline
      int pitch = lvlSensor->readPitch();
      int roll = lvlSensor->readRoll();
      int incline = lvlSensor->readIncline();
      Serial.write(pitch);
      Serial.write(roll);
      Serial.write(incline);
      Serial.write(0);
      Serial.write(0);
      Serial.flush();
    }
    else
    {
      // Pass through data from tea
      std::vector<byte> bytes = fmRadio->getData();
      for (int i = 0; i < 5; i++)
      {
        Serial.write(bytes[0]);
      }
      Serial.flush();
    }
  }
  // }
  // bleServer->update(buffer2);
  // //Reset the loop timer
  // loop_timer = micros();
}