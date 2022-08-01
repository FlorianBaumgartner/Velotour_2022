#include "system.h"
#include "console.h"
#include "esp_task_wdt.h"

//#define log   DISABLE_MODULE_LEVEL
#define DISABLE_BATTERY_MEASUREMENT

void System::begin(uint32_t watchdogTimeout)
{
  digitalWrite(pinPowerOff, 0);
  pinMode(pinPowerButton, OUTPUT);
  pinMode(pinPowerButton, INPUT_PULLUP);
  pinMode(pinBatMsr, INPUT);
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
  
  uint16_t soc = 0;
  for (soc = 0; soc <= 100; soc++)
  {
    if (voltage >= batteryVoltagePercentage[soc]) break;
  }
  console.log.printf("[SYSTEM] Battery Measurement: %.2f V (%d%%)\n", voltage, max(100 - soc, 0));
  return max(100 - soc, 0);
}
