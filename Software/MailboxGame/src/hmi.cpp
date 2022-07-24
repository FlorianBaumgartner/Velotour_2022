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

void Hmi::setNodeIndicator(int node, LedNode status)
{
  nodeIndicator[node % NUM_LEDS_NODE] = status;
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


