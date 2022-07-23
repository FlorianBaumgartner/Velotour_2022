#include "system.h"
#include "console.h"
#include "esp_task_wdt.h"

void System::begin(uint32_t watchdogTimeout)
{
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
  console.log.printf("[SYSTEM] Watchdog started with timout of %d s\n", seconds);
}

void System::feedWatchdog(void)
{
  esp_task_wdt_reset();
}