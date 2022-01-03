#include <Wire.h>
#include <Arduino.h>
#include "Comm.h"

byte dataBuffer[5];
void Comm::setup()
{
}
void Comm::loop()
{
    if (Serial.available() > 0)
    {
        Serial.readBytes(dataBuffer, 5);
        Serial.println("Received... ");
        Serial.println(dataBuffer[0], HEX);
        Serial.println(dataBuffer[1], HEX);
        Serial.println(dataBuffer[2], HEX);
        Serial.println(dataBuffer[3], HEX);
        Serial.println(dataBuffer[4], HEX);
    }
}

// void Comm::get(byte *result)
// {
//     for (int i = 0; i < 5; i++)
//     {
//         result[i] = dataBuffer[i];
//         dataBuffer[i] = 0;
//     }
// }