#include <Arduino.h>
#include "utils.h"
#include <TFT_eSPI.h>

#define LED               33
#define BLINK_INTERVAL    500

Utils utils;
TFT_eSPI tft = TFT_eSPI();

void setup()
{
  pinMode(LED, OUTPUT);
  utils.begin("TEST");

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  digitalWrite(LED, 1);  // Backlight on

  // (cursor will move to next line automatically during printing with 'tft.println'
  //  or stay on the line is there is room for the text with tft.print)
  tft.setCursor(0, 0, 2);  // Set "cursor" at top left corner of display (0,0) and select font 2
  // Set the font colour to be white with a black background, set text size multiplier to 1
  tft.setTextColor(TFT_WHITE,TFT_BLACK);  tft.setTextSize(1);
  // We can now plot text on screen using the "print" class
  tft.println("Hello World!");
  
  // Set the font colour to be yellow with no background, set to font 7
  tft.setTextColor(TFT_YELLOW); tft.setTextFont(2);
  tft.println(1234.56);
  
  // Set the font colour to be red with black background, set to font 4
  tft.setTextColor(TFT_RED,TFT_BLACK);    tft.setTextFont(4);
  tft.println((long)3735928559, HEX); // Should print DEADBEEF

  // Set the font colour to be green with black background, set to font 2
  tft.setTextColor(TFT_GREEN,TFT_BLACK);
  tft.setTextFont(2);
  tft.println("Groop");

  // Test some print formatting functions
  float fnumber = 123.45;
  tft.setTextColor(TFT_BLUE);    tft.setTextFont(2);
  tft.print("Float = "); tft.println(fnumber);           // Print floating point number
  tft.print("Binary = "); tft.println((int)fnumber, BIN); // Print as integer value in binary
  tft.print("Hexadecimal = "); tft.println((int)fnumber, HEX); // Print as integer number in Hexadecimal
}

void loop()
{
  static float fps = 0.0;
  static int t = 0, f = millis();
  if(millis() - t > 1000)
  {
    t = millis();
    Serial.printf("FPS: %.1f\n", fps);
  }
  


  
  uint16_t color = i;
  tft.fillScreen(color);
  tft.setCursor(0, 0, 2);
  tft.setTextColor(~color);
  tft.print(millis());
  color++;
  count++;

  //digitalWrite(LED, (millis() / BLINK_INTERVAL) & 1);
  //delay(1);
}
