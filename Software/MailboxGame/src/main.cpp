#include <Arduino.h>
#include "console.h"
#include "system.h"
#include "utils.h"
#include "mesh.h"
#include "hmi.h"

#include "office.h"
#include "mailbox.h"

#define USER_BTN          0
#define PWR_OFF           -1    // 1
#define PWR_BTN           -1    // 2
#define BAT_MSR           -1    // 3
#define NFC_RST           1     // 21
#define NFC_IRQ           26
#define NFC_SDA           3     // 33
#define NFC_SCL           2     // 34
#define EXT_A             35
#define EXT_B             36
#define EXT_C             37
#define EXT_D             38
#define SYS_LED           40    // 45
#define SYS_BZR           4     // 46

#define WATCHDOG_TIMEOUT  10    // [s]
#define LOW_BATTERY_SOC   30    // [%]
#define MIN_BATTERY_SOC   10    // [%]


Mesh mesh;
Utils utils;
Hmi hmi(SYS_LED, SYS_BZR);
System sys(PWR_OFF, PWR_BTN, BAT_MSR);
Office office(sys, hmi, mesh);
Mailbox mailbox(NFC_RST, NFC_IRQ, NFC_SCL, NFC_SDA, sys, hmi, mesh);
static bool error = false;
void powerDown(void);


void setup()
{
  pinMode(USER_BTN, INPUT_PULLUP);
  console.begin();
  sys.begin(WATCHDOG_TIMEOUT);
  hmi.begin();
  hmi.setMode(Hmi::LED_MODE_POWER_ON);
  hmi.playSound(Hmi::BUZZER_POWER_ON);
  if(!utils.begin("DRIVE"))
  {
    console.error.println("[MAIN] Could not initialize utilities");
    error = true;
  }

  int id = strtol(utils.getSerialNumber(), NULL, 0);
  mesh.begin(id);

  if(id == 0)   // Check if Device is Post Office (Serial Number "0")
  {
    console.log.println("[MAIN] Device has ID 0 -> Start Post Office");
    if(!office.begin())
    {
      console.error.println("[MAIN] Could not start Post Office");
      error = true;
    }
  }
  else
  {
    console.log.printf("[MAIN] Device has ID %d -> Start Mailbox\n", id);
    if(!mailbox.begin())
    {
      console.error.println("[MAIN] Could not start Mailbox");
      error = true;
    }
  }
  while(hmi.isPlaying()) delay(10);
  hmi.setStatusIndicator(error? Hmi::LED_STATUS_ERROR : Hmi::LED_STATUS_OK);
  hmi.playSound(error? Hmi::BUZZER_ERROR : Hmi::BUZZER_NONE);
  
  pinMode(33, OUTPUT);    // TODO: Remove
}

void loop()
{
  static bool btn = true;
  if(!digitalRead(USER_BTN) && btn)
  {
    console.log.println("[MAIN] Boot button pressed -> Turn off system");
    powerDown();
  }
  btn = digitalRead(USER_BTN);

  static bool lowBat = false;
  float soc = sys.getBatteryPercentage();
  if(lowBat != (soc < LOW_BATTERY_SOC) && !error)
  {
    hmi.setStatusIndicator(lowBat? Hmi::LED_STATUS_LOW_BATTERY : Hmi::LED_STATUS_OK);
    if(lowBat)
    {
      console.warning.printf("[MAIN] Battery state critical: %d%%\n", soc);
    }
  }
  if(soc < MIN_BATTERY_SOC)
  {
    console.error.printf("[MAIN] Low Battery! (%d%%)\n", soc);
    powerDown();
  }
  lowBat = soc < LOW_BATTERY_SOC;

  sys.feedWatchdog();
  delay(50);

  //digitalWrite(33, utils.isConnected());
}

void powerDown(void)
{
  mesh.end();
  hmi.setResultIndicator(Hmi::LED_RESULT_NONE);
  hmi.setStatusIndicator(Hmi::LED_STATUS_OFF);
  hmi.setMode(Hmi::LED_MODE_POWER_OFF);
  hmi.playSound(Hmi::BUZZER_POWER_OFF);
  while(hmi.getMode() != Hmi::LED_MODE_NONE) delay(10);
  hmi.end();
  sys.powerDown(true);
}
