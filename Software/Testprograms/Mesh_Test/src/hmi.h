#ifndef HMI_H
#define HMI_H

#include <Arduino.h>
#include "Adafruit_NeoPixel.h"

#define NUM_LEDS_STATUS         1
#define NUM_LEDS_RESULT         3
#define NUM_LEDS_NETWORK        10


class Hmi
{
  public:
    Hmi(int pinLed, int pinBuzzer) : pinLed(pinLed), pinBuzzer(pinBuzzer) {}
    void begin(void);

  private:
    int pinLed, pinBuzzer;
    Adafruit_NeoPixel led;
};

#endif