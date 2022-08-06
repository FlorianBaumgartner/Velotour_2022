/******************************************************************************
* file    system.cpp
*******************************************************************************
* brief   Controls watchdog and power management
*******************************************************************************
* author  Florian Baumgartner
* version 1.0
* date    2022-08-02
*******************************************************************************
* MIT License
*
* Copyright (c) 2022 Crelin - Florian Baumgartner
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell          
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
******************************************************************************/

#include "system.h"
#include "console.h"
#include "esp_task_wdt.h"

#define log   DISABLE_MODULE_LEVEL
//#define DISABLE_BATTERY_MEASUREMENT

void System::begin(uint32_t watchdogTimeout)
{
  digitalWrite(pinPowerOff, 0);
  pinMode(pinPowerOff, OUTPUT);
  pinMode(pinPowerButton, INPUT_PULLUP);
  analogSetAttenuation(ADC_11db);
  if(watchdogTimeout > 0)
  {
    startWatchdog(watchdogTimeout);
  }
}

void System::startWatchdog(uint32_t seconds)
{
  esp_task_wdt_init(seconds, true);   // Enable panic so ESP32 restarts
  esp_task_wdt_add(NULL);             // Add current thread to WDT watch
  esp_task_wdt_reset();
  console.log.printf("[SYSTEM] Watchdog started with timeout of %d s\n", seconds);
}

void System::feedWatchdog(void)
{
  esp_task_wdt_reset();
}

void System::powerDown(bool feedDog)
{
  console.log.println("[SYSTEM] Power down system...");
  delay(10);
  digitalWrite(pinPowerOff, 1);
  while(true)
  {
    if(feedDog)
    {
      feedWatchdog();
    }
    delay(100);
  }
}

bool System::getButtonState(void)
{
  if(pinPowerButton == -1) return false;
  return !digitalRead(pinPowerButton);
}

uint8_t System::getBatteryPercentage(void)
{
  #ifdef DISABLE_BATTERY_MEASUREMENT
    return 100;
  #endif

  float voltage = 0.0;
  for(int i = 0; i < MEASUREMENT_AVR_NUM; i++)
  {
    voltage += analogRead(pinBatMsr);
  }
  voltage /= (float)MEASUREMENT_AVR_NUM;
  voltage /= (float)MEASUREMENT_FACTOR;

  static bool printedWarning = false;
  if(voltage < 0.2)
  {
    if(!printedWarning)
    {
      console.warning.printf("[SYSTEM] No Battery detected (%.2f V)\n", voltage);
      printedWarning = true;
    }
    return 100;
  }
  
  uint16_t soc = 0;
  for (soc = 0; soc <= 100; soc++)
  {
    if (voltage >= batteryVoltagePercentage[soc]) break;
  }
  soc = max(100 - soc, 0);
  console.log.printf("[SYSTEM] Battery Measurement: %.2f V (%d%%)\n", voltage, soc);
  return soc;
}
