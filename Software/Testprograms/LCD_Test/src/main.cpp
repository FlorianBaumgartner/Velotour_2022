#include <Arduino.h>
#include "utils.h"
#include <TFT_eSPI.h>

#define LED               33
#define BLINK_INTERVAL    500

Utils utils;
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite framebuf = TFT_eSprite(&tft);

void setup()
{
  pinMode(LED, OUTPUT);
  utils.begin("TEST");

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  framebuf.createSprite(TFT_WIDTH, TFT_HEIGHT);
  framebuf.fillSprite(TFT_BLACK);
  digitalWrite(LED, 1);  // Backlight on

  // (cursor will move to next line automatically during printing with 'tft.println'
  //  or stay on the line is there is room for the text with tft.print)
  framebuf.setCursor(0, 0, 2);  // Set "cursor" at top left corner of display (0,0) and select font 2
  // Set the font colour to be white with a black background, set text size multiplier to 1
  framebuf.setTextColor(TFT_WHITE,TFT_BLACK);  tft.setTextSize(1);
  // We can now plot text on screen using the "print" class
  framebuf.println("Hello World!");
  
  // Set the font colour to be yellow with no background, set to font 7
  framebuf.setTextColor(TFT_YELLOW); tft.setTextFont(2);
  framebuf.println(1234.56);
  
  // Set the font colour to be red with black background, set to font 4
  framebuf.setTextColor(TFT_RED,TFT_BLACK);    tft.setTextFont(4);
  framebuf.println((long)3735928559, HEX); // Should print DEADBEEF

  // Set the font colour to be green with black background, set to font 2
  framebuf.setTextColor(TFT_GREEN,TFT_BLACK);
  framebuf.setTextFont(2);
  framebuf.println("Groop");

  // Test some print formatting functions
  float fnumber = 123.45;
  framebuf.setTextColor(TFT_BLUE);    tft.setTextFont(2);
  framebuf.print("Float = "); tft.println(fnumber);           // Print floating point number
  framebuf.print("Binary = "); tft.println((int)fnumber, BIN); // Print as integer value in binary
  framebuf.print("Hexadecimal = "); tft.println((int)fnumber, HEX); // Print as integer number in Hexadecimal

  framebuf.pushSprite(0, 0);
}

void loop()
{
  static float fps = 0.0;
  static int t = 0, f = 0;
  if(millis() - t > 100)
  {
    t = millis();
    Serial.printf("FPS: %.1f\n", fps);
  }

  fps = 1000.0 / (millis() - f);
  f = millis();

  framebuf.fillSprite(TFT_BLACK);
  framebuf.setCursor(0, 0, 2);
  framebuf.setTextColor(TFT_WHITE);
  framebuf.printf("FPS: %.1f", fps);

  
  framebuf.pushSprite(0, 0);

  //digitalWrite(LED, (millis() / BLINK_INTERVAL) & 1);
  //delay(1);
}
