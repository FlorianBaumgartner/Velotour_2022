#include <Arduino.h>
#include "console.h"
#include "utils.h"
#include "mesh.h"
#include "hmi.h"
#include <TFT_eSPI.h>
#include <MFRC522.h>

#define USER_BTN          0
#define USER_LED          13
#define BACKLIGHT         33
#define RST_PIN           1
#define SCL               2
#define SDA               3
#define LED               40
#define BUZZER            4

#define BLINK_INTERVAL    500


Utils utils;
Mesh mesh;
Hmi hmi(LED, BUZZER);
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite framebuf = TFT_eSprite(&tft);

uint8_t devAddr = 0x28;
TwoWire i2cBus = TwoWire(0);
MFRC522_I2C dev = MFRC522_I2C(RST_PIN, devAddr, i2cBus);
MFRC522 mfrc522 = MFRC522(&dev);

void taskA(void *pvParameter)
{
  while(true)
  {
    TickType_t task_last_tick = xTaskGetTickCount();
    //console.ok.println("This is an ok message");
    console.ok.printf("Time: %d\n", millis());
    vTaskDelayUntil(&task_last_tick, (const TickType_t) 1000);
  }
}

void taskB(void *pvParameter)
{
  while(true)
  {
    TickType_t task_last_tick = xTaskGetTickCount();
    console.warning.println("This is a warning message");
    vTaskDelayUntil(&task_last_tick, (const TickType_t) 1000);
  }
}

void taskC(void *pvParameter)
{
  while(true)
  {
    TickType_t task_last_tick = xTaskGetTickCount();
    console.error.println("This is an error message");
    vTaskDelayUntil(&task_last_tick, (const TickType_t) 1500);
  }
}

void setup()
{
  pinMode(BACKLIGHT, OUTPUT);
  pinMode(USER_LED, OUTPUT);
  pinMode(USER_BTN, INPUT_PULLUP);
  console.begin();
  console.setLevel(Console::LEVEL_LOG);
  hmi.begin();
  utils.begin("TEST");

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  framebuf.createSprite(TFT_HEIGHT, TFT_WIDTH);
  framebuf.fillSprite(TFT_BLACK);
  framebuf.pushSprite(0, 0);
  digitalWrite(BACKLIGHT, 1);   // Backlight on

  if(strtol(utils.getSerialNumber(), NULL, 0) == 0)
  {
    i2cBus.begin(SDA, SCL, 400000U); 
    mfrc522.PCD_Init();		        // Init MFRC522
  }

  mesh.begin(strtol(utils.getSerialNumber(), NULL, 0));

  xTaskCreate(taskA, "task_A", 2048, NULL, 1, NULL);
  //xTaskCreate(taskB, "task_B", 2048, NULL, 1, NULL);
  //xTaskCreate(taskC, "task_C", 2048, NULL, 1, NULL);
}

uint32_t getCardData(void)
{
  uint8_t req_buff[2];
  uint8_t req_buff_size = sizeof(req_buff);
  mfrc522.PCD_StopCrypto1();
  mfrc522.PICC_HaltA();                             // TODO: Delay after this functioncall necessary?
  mfrc522.PICC_WakeupA(req_buff,&req_buff_size);    // TODO: Delay after this functioncall necessary?
  uint8_t s = mfrc522.PICC_Select(&(mfrc522.uid), 0);
  if(mfrc522.GetStatusCodeName((MFRC522::StatusCode)s) == F("Timeout in communication.")) return -1;
  return *(uint32_t*) mfrc522.uid.uidByte;
}

void loop()
{
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

    framebuf.fillSprite(TFT_BLACK);
    framebuf.setCursor(0, 0, 2);
    framebuf.setTextColor(TFT_WHITE);
    framebuf.printf("Serial Number: %s    Time: %d\n", utils.getSerialNumber(), millis());

    //console.printf("Card: %08X\n", getCardData());
    //console.printTimestamp(); console.println("Test");
    //console.log.printf("Time: %d\n", millis());
    framebuf.fillRect(3, 135 - 10 - 3, 10, 10, utils.isConnected()? TFT_GREEN : TFT_RED);   // Shows if USB is connected
    framebuf.fillRect(3 + 10 + 3, 135 - 10 - 3, 10, 10, utils? TFT_GREEN : TFT_RED);        // Shows if USB MSC is ready
    framebuf.fillRect(3 + 20 + 6, 135 - 10 - 3, 10, 10, console? TFT_GREEN : TFT_RED);      // Shows if USB Serial port is opened
    
    int8_t ids[MAX_NODES_NUM];
    mesh.getNodeIds(ids);
    for(int i = 0; i < mesh.getNodeCount(); i++)
    {
      int y = 20 + i * 15;
      framebuf.setCursor(0, y, 2);
      uint32_t payload = mesh.getNodePayload(ids[i]);
      framebuf.setTextColor(TFT_WHITE);
      framebuf.printf("Node %d = ", ids[i]);
      framebuf.setTextColor((payload != -1)? TFT_GREEN : TFT_RED);
      framebuf.printf("%08X", payload);
      framebuf.setTextColor(TFT_WHITE);
      framebuf.printf(" (%s)", mesh.getNodeOrigin(ids[i])? "Origin" : "Relayed");
    }

    framebuf.setTextColor(TFT_WHITE);
    framebuf.setCursor(100, 120, 2);
    framebuf.print("Card: ");
    if(card != -1)
    {
      framebuf.setTextColor(TFT_GREEN);
      framebuf.printf("%08X", card);
    }
    else
    {
      framebuf.setTextColor(TFT_RED);
      framebuf.print("No Card");
    }

    framebuf.pushSprite(0, 0);
  }

  delay(1);
}