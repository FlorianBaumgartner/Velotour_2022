#include "hmi.h"
#include "console.h"


void Hmi::begin(void)
{
  digitalWrite(pinBuzzer, 0);
  pinMode(pinBuzzer, OUTPUT);
  led.setPin(pinLed);
  led.updateLength(NUM_LEDS_STATUS + NUM_LEDS_RESULT + NUM_LEDS_NETWORK);
  led.clear();
  led.begin();

  led.setPixelColor(0, led.Color(255, 0, 0));
  led.setPixelColor(1, led.Color(0, 255, 0));
  led.setPixelColor(2, led.Color(0, 0, 255));
  led.show();

  int freq = 2000;
  int channel = 1;
  int resolution = 8;

  ledcSetup(channel, freq, resolution);
  ledcAttachPin(pinBuzzer, channel);

  ledcWriteTone(channel, 800);
  delay(200);
  ledcWriteTone(channel, 1180);
  delay(200);
  ledcWriteTone(channel, 0);
}



