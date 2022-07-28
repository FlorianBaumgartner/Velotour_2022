#include <Arduino.h>
#include "console.h"
#include "system.h"
#include "utils.h"
#include "mesh.h"
#include "hmi.h"

#include "office.h"
#include "mailbox.h"

#define USER_BTN          0
#define PWR_OFF           1
#define PWR_BTN           2
#define BAT_MSR           3
#define NFC_RST           21
#define NFC_IRQ           26
#define NFC_SDA           33
#define NFC_SCL           34
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
    hmi.setStatusIndicator(Hmi::LED_STATUS_ERROR);
    hmi.playSound(Hmi::BUZZER_ERROR);
  }

  console.print("[MAIN] ");
  console[COLOR_BLUE].print("This text is blue with a ");
  console[COLOR_MAGENTA].print("pink");
  console[COLOR_BLUE].print(" word in it. ");
  console.println("- now back to normal mode");

  console.print("[MAIN] ");
  console[COLOR_CYAN].print("This text is cyan with some ");
  console[COLOR_CYAN][COLOR_RED].print("highlighted words");
  console[COLOR_CYAN].println(" which can be very usefull.");

  int id = strtol(utils.getSerialNumber(), NULL, 0);
  mesh.begin(id);

  if(id == 0)   // Check if Device is Post Office (Serial Number "0")
  {
    console.log.println("[MAIN] Device has ID 0 -> Start Post Office");
    Office office(sys, hmi, mesh);
    if(!office.begin())
    {
      console.error.println("[MAIN] Could not start Post Office");
      hmi.setStatusIndicator(Hmi::LED_STATUS_ERROR);
      hmi.playSound(Hmi::BUZZER_ERROR);
    }
  }
  else
  {
    console.log.printf("[MAIN] Device has ID %d -> Start Mailbox\n", id);
    Mailbox mailbox(NFC_RST, NFC_IRQ, NFC_SCL, NFC_SDA, sys, hmi, mesh);
    if(!mailbox.begin())
    {
      console.error.println("[MAIN] Could not start Mailbox");
      hmi.setStatusIndicator(Hmi::LED_STATUS_ERROR);
      hmi.playSound(Hmi::BUZZER_ERROR);
    }
  }
}

void loop()
{
  static bool btn = true;
  if(!digitalRead(USER_BTN) && btn)
  {

  }
  btn = digitalRead(USER_BTN);

  static bool lowBat = false;
  float soc = sys.getBatteryPercentage();
  if(lowBat != LOW_BATTERY_SOC)
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
    sys.powerDown();
  }
  lowBat = soc < LOW_BATTERY_SOC;

  sys.feedWatchdog();
  delay(50);
}