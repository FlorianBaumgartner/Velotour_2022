/******************************************************************************
* file    main.cpp
*******************************************************************************
* brief   Main Program
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
#define SYS_LED           45
#define SYS_BZR           44    // RDX-Pin

#define LONG_PRESS_TIME   5     // [s]
#define WATCHDOG_TIMEOUT  10    // [s]
#define LOW_BATTERY_SOC   30    // [%]
#define MIN_BATTERY_SOC   10    // [%]

Mesh mesh;
Hmi hmi(SYS_LED, SYS_BZR);
System sys(PWR_OFF, PWR_BTN, BAT_MSR, EXT_A, EXT_B, EXT_C, EXT_D);
Office office(sys, hmi, mesh);
Mailbox mailbox(NFC_RST, NFC_IRQ, NFC_SCL, NFC_SDA, sys, hmi, mesh);
static bool error = false;
void powerDown(void);


void setup()
{
  sys.startWatchdog(WATCHDOG_TIMEOUT);
  pinMode(USER_BTN, INPUT_PULLUP);
  console.begin();
  sys.begin();
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
}

void loop()
{
  static int btnTimer = millis();
  static bool btn = false;
  
  if(btn && digitalRead(USER_BTN) && (millis() - btnTimer < LONG_PRESS_TIME * 1000))
  {
    console.log.println("[MAIN] Boot button short press detected -> Turn off system");
    powerDown();
  }
  else if(!sys.getBatteryDischargeState() && (millis() - btnTimer > LONG_PRESS_TIME * 1000))
  {
    console.log.println("[MAIN] Boot button long press detected -> Battery discharge mode started");
    sys.startBatteryDischarge();
    hmi.playSound(Hmi::BUZZER_DISCHARGING);
  }
  btn = !digitalRead(USER_BTN);
  if(!btn)
  {
    btnTimer = millis();
  }
  if(sys.getBatteryDischargeState())
  {
    hmi.setBatteryDischargeMode(sys.getBatteryDischargeProgress());
  }
  if(sys.getBatteryDischargeProgress() == 100)
  {
    console.ok.printf("[MAIN] Battery has been discharged to storage voltage: %.2f V, power down now...\n", sys.getBatteryVoltage());
    powerDown();
  }

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
  if(soc < MIN_BATTERY_SOC && !utils.isConnected() && millis() > 10000)
  {
    console.error.printf("[MAIN] Low Battery! (%d%%)\n", soc);
    powerDown();
  }
  lowBat = soc < LOW_BATTERY_SOC;

  if(!error || utils.isConnected() || sys.getBatteryDischargeState())
  {
    sys.feedWatchdog();
  }
  delay(50);
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
