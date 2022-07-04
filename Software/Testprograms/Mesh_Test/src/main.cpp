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
  framebuf.createSprite(TFT_WIDTH, TFT_HEIGHT);
  framebuf.fillSprite(TFT_BLACK);
  framebuf.pushSprite(0, 0);
  digitalWrite(BACKLIGHT, 1);  // Backlight on

  mesh.begin(strtol(utils.getSerialNumber(), NULL, 0));
}

void loop()
{
  bool update = false;
  static bool ledOld = false;
  static bool btn = true;
  if(btn && !digitalRead(USER_BTN))
  {
    Serial.println("Send Mesage");
    mesh.setLedState(!mesh.getLedState());
    mesh.broadcast(mesh.getLedState()? "on":"off");
  }
  btn = digitalRead(USER_BTN);

  if(ledOld != mesh.getLedState())
  {
    ledOld = mesh.getLedState();
    digitalWrite(LED, ledOld);
    update = true;
  }
  
  static int t = 0;
  if(millis() - t > 1000 || update)
  {
    update = false;
    t = millis();
    Serial.printf("Time: %d\n", millis());

    framebuf.fillSprite(TFT_BLACK);
    framebuf.setCursor(0, 0, 2);
    framebuf.setTextColor(TFT_WHITE);
    framebuf.printf("Serial Number: %s\n", utils.getSerialNumber());
    framebuf.printf("Time: %d\n", millis());
    framebuf.printf("LED: %d\n", mesh.getLedState());
    framebuf.pushSprite(0, 0);
  }

  delay(1);
}