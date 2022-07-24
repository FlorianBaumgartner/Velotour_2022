#include "hmi.h"
#include "console.h"
#include "freertos/task.h"

void Hmi::begin(void)
{
  digitalWrite(pinBuzzer, 0);
  pinMode(pinBuzzer, OUTPUT);
  led.setPin(pinLed);
  led.updateLength(NUM_LEDS_STATUS + NUM_LEDS_RESULT + NUM_LEDS_NODE);
  led.clear();
  led.begin();

  led.setPixelColor(0, led.Color(255, 0, 0));
  led.setPixelColor(1, led.Color(0, 255, 0));
  led.setPixelColor(2, led.Color(0, 0, 255));
  led.show();

  int freq = 2000;
  int channel = 1;
  int resolution = 8;

  ledcSetup(channel, freq, resolution);
  ledcAttachPin(pinBuzzer, channel);

  ledcWriteTone(channel, 800);
  delay(200);
  ledcWriteTone(channel, 1180);
  delay(200);
  ledcWriteTone(channel, 0);

  xTaskCreate(update, "task_hmi", 2048, this, 1, NULL);
}

void Hmi::update(void* pvParameter)
{
  Hmi* ref = (Hmi*)pvParameter;

  while(true)
  {
    TickType_t task_last_tick = xTaskGetTickCount();


    vTaskDelayUntil(&task_last_tick, (const TickType_t) 1000 / TASK_HMI_FREQ);
  }
  vTaskDelete(NULL);
}


