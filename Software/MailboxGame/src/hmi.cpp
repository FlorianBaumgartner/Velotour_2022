#include "hmi.h"
#include "console.h"
#include "freertos/task.h"

//#define log   DISABLE_MODULE_LEVEL

void Hmi::begin(void)
{
  digitalWrite(pinBuzzer, 0);
  pinMode(pinBuzzer, OUTPUT);
  led.setPin(pinLed);
  led.updateLength(NUM_LEDS_STATUS + NUM_LEDS_RESULT + NUM_LEDS_NODE);
  led.clear();
  led.begin();
  led.show();

  ledcSetup(BUZZER_PWM_CHANNEL, 1000, 8);       // Channel, Frequency, Resolution
  ledcAttachPin(pinBuzzer, BUZZER_PWM_CHANNEL);
  ledcWriteTone(BUZZER_PWM_CHANNEL, 0);

  xTaskCreate(update, "task_hmi", 4096, this, 1, NULL);
}

void Hmi::setStatusIndicator(LedStatus status)
{
  statusIndicator = status;
}

void Hmi::setResultIndicator(LedResult result)
{
  resultIndicator = result;
}

void Hmi::setMode(LedMode mode)
{
   ledMode = mode;
   ledModeTimer = millis();
}

void Hmi::setNodeStatus(int node, NodeStatus status)
{
  nodeStatus[node % NUM_LEDS_NODE] = status;
}

void Hmi::playSound(BuzzerSound sound)
{
  buzzerSound = sound;
  switch(buzzerSound)
  {
    case BUZZER_POWER_ON:
      melody = TONE_POWER_ON;
      melodyLength = sizeof(TONE_POWER_ON) / sizeof(Tone);
      break;
    case BUZZER_POWER_OFF:
      melody = TONE_POWER_OFF;
      melodyLength = sizeof(TONE_POWER_OFF) / sizeof(Tone);
      break;
    case BUZZER_SUCCESS:
      melody = TONE_SUCCESS;
      melodyLength = sizeof(TONE_SUCCESS) / sizeof(Tone);
      break;
    case BUZZER_ERROR:
      melody = TONE_ERROR;
      melodyLength = sizeof(TONE_ERROR) / sizeof(Tone);
      break;
    default:
      melody = nullptr;
      melodyLength = 0;
      return;
  }
  playing = true;
}


void Hmi::update(void* pvParameter)
{
  Hmi* ref = (Hmi*)pvParameter;
  bool playing = false;
  int32_t buzzerTimer = -1;

  while(true)
  {
    TickType_t task_last_tick = xTaskGetTickCount();

    // Status LED
    uint8_t breathing = map(abs((int)(millis() % 3000) - 1500), 0, 1500, 0, 255);
    switch(ref->statusIndicator)
    {
      case LED_STATUS_OK:
        ref->led.setPixelColor(0, ref->led.Color(0, 255, 0));   // Green steady
        break;
      case LED_STATUS_BUSY:
        ref->led.setPixelColor(0, ref->led.Color(gamma[breathing], gamma[breathing], gamma[breathing]));    // White breathing
        break;
      case LED_STATUS_ERROR:
        ref->led.setPixelColor(0, ref->led.Color((millis() % 400) > 200? 255 : 0, 0, 0));   // Red blinking
        break;
      default:
        ref->led.setPixelColor(0, 0);
    }

    // Result LEDs
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

    // Animation / Network LEDs
    for(int i = 0; i < NUM_LEDS_NODE; i++)
    {
      uint8_t l = NUM_LEDS_STATUS + NUM_LEDS_RESULT + i;
      switch(ref->ledMode)
      {
        case LED_MODE_OFF:
          ref->led.setPixelColor(l, 0);
          break;
        case LED_MODE_POWER_ON:
          
          break;
        case LED_MODE_POWER_OFF:
          break;
        case LED_MODE_SUCCESS:
          ref->led.setPixelColor(l, ref->led.Color(0, (millis() % 400) > 200? 255 : 0, 0));   // Green blinking
          break;
        case LED_MODE_CARD_INSERTED:
          break;
        case LED_MODE_NODE_STATUS:
          ref->led.setPixelColor(l, 0);   // LED OFF per default
          if(ref->nodeStatus[i] == NODE_ACTIVE)
          {
            ref->led.setPixelColor(l, ref->led.Color(0, 255, 0));     // Green steady
          }
          else if(ref->nodeStatus[i] == NODE_CONNECTED)
          {
            ref->led.setPixelColor(l, ref->led.Color(255, 255, 0));   // Yellow steady
          }
          break;
      }
    }
    ref->led.show();


    if(ref->playing != playing)
    {
      playing = ref->playing;
      if(playing)
      {
        buzzerTimer = millis();
      }
      else
      {
        ledcWriteTone(BUZZER_PWM_CHANNEL, 0);   // Turn off buzzer
        buzzerTimer = -1;
        ref->buzzerSound = BUZZER_NONE;
      }
    }
    if(buzzerTimer != -1)
    {
      static int freq = 0, freqOld = 0;
      if(ref->playing && ref->melody != nullptr)
      {
        int i = 0, t = 0;
        for(i = 0; i < ref->melodyLength; i++)
        {
          if(t >= millis() - buzzerTimer) break;
          t += ref->melody[i].duration;
          freq = ref->melody[i].freq;
        }
        if((i == ref->melodyLength) && (millis() - buzzerTimer > t))
        {
          buzzerTimer = -1;
          ref->playing = false;
        }
        else if(freq != freqOld)
        {
          freqOld = freq;
          ledcWriteTone(BUZZER_PWM_CHANNEL, freq);
        }
      }
    }
  
    vTaskDelayUntil(&task_last_tick, (const TickType_t) 1000 / TASK_HMI_FREQ);
  }
  vTaskDelete(NULL);
}


