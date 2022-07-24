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


Mesh mesh;
Utils utils;
Hmi hmi(SYS_LED, SYS_BZR);
System sys(PWR_OFF, PWR_BTN, BAT_MSR);

void setup()
{
  pinMode(USER_BTN, INPUT_PULLUP);
  console.begin();
  sys.begin(10);   // Set Watchdog Timout to 10s
  hmi.begin();
  utils.begin("DRIVE");

  int id = strtol(utils.getSerialNumber(), NULL, 0);
  mesh.begin(id);

  if(id == 0)   // Check if Device is Post Office (Serial Number "0")
  {
    console.log.println("[MAIN] Device has ID 0 -> Start Post Office");
    Office office(sys, hmi, mesh);
    office.begin();
  }
  else
  {
    console.log.printf("[MAIN] Device has ID %d -> Start Mailbox\n", id);
    Mailbox mailbox(NFC_RST, NFC_IRQ, NFC_SCL, NFC_SDA, sys, hmi, mesh);
    //mailbox.begin();
  }
}

void loop()
{
  sys.feedWatchdog();
  delay(100);

/*
  static bool update = true;
  static uint8_t nodeCount = 0;
  if(mesh.getNodeCount() != nodeCount)
  {
    nodeCount = mesh.getNodeCount();
    update = true;
  }
  
  static int t = 0;
  if(millis() - t > 100 || update)
  {
    t = millis();
    update = false;
    uint32_t card = -1;
    if(mesh.getPersonalId() == 0) card = getCardData();
    mesh.setPayload(card);
  */
}