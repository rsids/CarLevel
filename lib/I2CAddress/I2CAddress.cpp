/*********
  Rui Santos
  Complete project details at https://randomnerdtutorials.com
*********/

#include <Wire.h>
#include <Arduino.h>

TwoWire I2C_scanner = TwoWire(0);

void i2cAddressSetup(int SDA, int SCL, int BUS)
{
    I2C_scanner.begin(SDA, SCL, 100000);
    Serial.begin(115200);
    Serial.print("\nI2C Scanner ");
    Serial.print(SDA);
    Serial.print(" / ");
    Serial.println(SCL);
}

void i2cAddressLoop()
{
    byte error, address;
    int nDevices;
    Serial.println("Scanning...");
    nDevices = 0;
    for (address = 1; address < 127; address++)
    {
        I2C_scanner.beginTransmission(address);
        error = I2C_scanner.endTransmission();
        if (error == 0)
        {
            Serial.print("I2C device found at address 0x");
            if (address < 16)
            {
                Serial.print("0");
            }
            Serial.println(address, HEX);
            nDevices++;
        }
        else if (error == 4)
        {
            Serial.print("Unknow error at address 0x");
            if (address < 16)
            {
                Serial.print("0");
            }
            Serial.println(address, HEX);
        }
    }
    if (nDevices == 0)
    {
        Serial.println("No I2C devices found\n");
    }
    else
    {
        Serial.println("done\n");
    }
    delay(5000);
}
