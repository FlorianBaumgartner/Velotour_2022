#include <Arduino.h>
#include "utils.h"
#include "mesh.h"
#include <TFT_eSPI.h>

#define USER_BTN          0
#define BACKLIGHT         33
#define LED               13
#define BLINK_INTERVAL    500

Utils utils;
Mesh mesh;
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite framebuf = TFT_eSprite(&tft);

void setup()
{
  pinMode(LED, OUTPUT);
  pinMode(BACKLIGHT, OUTPUT);
  pinMode(USER_BTN, INPUT_PULLUP);
  utils.begin("TEST");

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  framebuf.createSprite(TFT_HEIGHT, TFT_WIDTH);
  framebuf.fillSprite(TFT_BLACK);
  framebuf.pushSprite(0, 0);
  digitalWrite(BACKLIGHT, 1);  // Backlight on

  mesh.begin(strtol(utils.getSerialNumber(), NULL, 0));
}

void loop()
{
  static bool update = true;
  static uint8_t nodeCount = 0;
  mesh.setPayload(!digitalRead(USER_BTN));
  if(mesh.getNodeCount() != nodeCount)
  {
    nodeCount = mesh.getNodeCount();
    update = true;
  }
  
  static int t = 0;
  if(millis() - t > 1000 || update)
  {
    t = millis();
    update = false;

    framebuf.fillSprite(TFT_BLACK);
    framebuf.setCursor(0, 0, 2);
    framebuf.setTextColor(TFT_WHITE);
    framebuf.printf("Serial Number: %s    Time: %d\n", utils.getSerialNumber(), millis());
    
    int8_t ids[MAX_NODES_NUM];
    mesh.getNodeIds(ids);
    for(int i = 0; i < mesh.getNodeCount(); i++)
    {
      int y = 20 + i * 15;
      framebuf.setCursor(0, y, 2);
      framebuf.printf("Node %d = %d (%s)", ids[i], mesh.getNodePayload(ids[i]), mesh.getNodeOrigin(ids[i])? "Origin" : "Relayed");
      framebuf.fillRect(150, y + 1, 50, 13, mesh.getNodePayload(ids[i]) == 1? TFT_GREEN : TFT_BLACK);
    }
    framebuf.pushSprite(0, 0);
  }

  delay(1);
}