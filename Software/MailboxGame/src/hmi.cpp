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

void Hmi::setStatusIndicator(LedStatus status)
{
  statusIndicator = status;
}

void Hmi::setResultIndicator(LedResult result)
{
  resultIndicator = result;
}

void Hmi::setNodeIndicator(int node, LedNode status)
{
  nodeIndicator[node % NUM_LEDS_NODE] = status;
}

void Hmi::playSound(BuzzerSound sound)
{
  
}


void Hmi::update(void* pvParameter)
{
  Hmi* ref = (Hmi*)pvParameter;

  while(true)
  {
    TickType_t task_last_tick = xTaskGetTickCount();

    for(int i = 0; i < NUM_LEDS_RESULT; i++)
    {
      ref->led.setPixelColor(NUM_LEDS_STATUS + i, 0);
    }
    switch(ref->resultIndicator)
    {
      case LED_RESULT_A: ref->led.setPixelColor(NUM_LEDS_STATUS + 0, ref->led.Color(255, 255, 255)); break;
      case LED_RESULT_B: ref->led.setPixelColor(NUM_LEDS_STATUS + 1, ref->led.Color(255, 255, 255)); break;
      case LED_RESULT_C: ref->led.setPixelColor(NUM_LEDS_STATUS + 2, ref->led.Color(255, 255, 255)); break;
      default: break;
    }


    vTaskDelayUntil(&task_last_tick, (const TickType_t) 1000 / TASK_HMI_FREQ);
  }
  vTaskDelete(NULL);
}


