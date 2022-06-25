#include <Arduino.h>
#include "utils.h"

#define LED               33
#define BLINK_INTERVAL    500

Utils utils;

void setup()
{
  pinMode(LED, OUTPUT);
  utils.begin("TEST");
 
}

void loop()
{
  static int t = 0;
  if(millis() - t > 1000)
  {
    t = millis();
    Serial.printf("Time: %d\n", t);

    //Serial.printf("VID: 0x%04X\nPID: 0x%04X\nSerial Number: %s\n\n", USB.VID(), USB.PID(), USB.serialNumber());
  }
  digitalWrite(LED, (millis() / BLINK_INTERVAL) & 1);
  delay(1);
}