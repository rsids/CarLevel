#include <stdint.h>
#include "utils.h"
#include <Arduino.h>

uint8_t twos_complement(uint8_t val)
{
    return -(unsigned int)val;
}

double deg2rad(double deg)
{
    return deg * DEG_TO_RAD;
}

double clamp(double d, double min, double max) {
  const double t = d < min ? min : d;
  return t > max ? max : t;
}
