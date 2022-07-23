#ifndef SYSTEM_H
#define SYSTEM_H

#include <Arduino.h>

class System
{
  public:
    System(int pinPowerOff, int pinPowerButton, int pinBatMsr): pinPowerOff(pinPowerOff), pinPowerButton(pinPowerButton), pinBatMsr(pinBatMsr) {}
    void begin(uint32_t watchdogTimeout = 0);
    void startWatchdog(uint32_t seconds);
    void feedWatchdog(void);

  private:
    const int pinPowerOff;
    const int pinPowerButton;
    const int pinBatMsr;

};

#endif
