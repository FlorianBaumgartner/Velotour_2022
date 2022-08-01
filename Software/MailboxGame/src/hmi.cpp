#include "hmi.h"
#include "console.h"
#include "freertos/task.h"

//#define log   DISABLE_MODULE_LEVEL
#define DISABLE_BUZZER

void Hmi::begin(void)
{
  digitalWrite(pinBuzzer, 0);
  pinMode(pinBuzzer, OUTPUT);
  led.setPin(pinLed);
  led.updateLength(NUM_LEDS_STATUS + NUM_LEDS_RESULT + NUM_LEDS_NODE);
  led.begin();
  led.clear();
  led.show();

  ledcSetup(BUZZER_PWM_CHANNEL, 1000, 8);       // Channel, Frequency, Resolution
  ledcAttachPin(pinBuzzer, BUZZER_PWM_CHANNEL);
  ledcWriteTone(BUZZER_PWM_CHANNEL, 0);

  initialized = true;
  xTaskCreate(update, "task_hmi", 2048, this, 10, NULL);
}

void Hmi::end(void)
{
  initialized = false;
  ledcWriteTone(BUZZER_PWM_CHANNEL, 0);
  led.clear();
  led.show();
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
   animationRunning = true;
   animationTimer = millis();
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
    case BUZZER_CARD_INSERTED:
      melody = TONE_CARD_INSERTED;
      melodyLength = sizeof(TONE_CARD_INSERTED) / sizeof(Tone);
      break;
    case BUZZER_CARD_REMOVED:
      melody = TONE_CARD_REMOVED;
      melodyLength = sizeof(TONE_CARD_REMOVED) / sizeof(Tone);
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

  while(ref->initialized)
  {
    TickType_t task_last_tick = xTaskGetTickCount();

    // Status LED
    uint8_t blinking = (millis() % 400) > 200? 255 : 0;
    uint8_t breathing = ref->gamma[map(abs((int)(millis() % 3000) - 1500), 0, 1500, 0, 255)];
    switch(ref->statusIndicator)
    {
      case LED_STATUS_OK:
        ref->led.setPixelColor(0, ref->led.Color(0, blinking, 0));   // Green blinking
        break;
      case LED_STATUS_BUSY:
        ref->led.setPixelColor(0, ref->led.Color(breathing, breathing, breathing));    // White breathing
        break;
      case LED_STATUS_ERROR:
        ref->led.setPixelColor(0, ref->led.Color(blinking, 0, 0));   // Red blinking
        break;
      case LED_STATUS_LOW_BATTERY:
        ref->led.setPixelColor(0, ref->led.Color(blinking, blinking, 0));   // Yellow blinking
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
        case LED_MODE_POWER_ON:
          if(ref->animationRunning)
          {
            int j = (millis() - ref->animationTimer) / 100;
            uint8_t c = abs(i * 10 - 45) / 10 <= j? 255 : 0;
            ref->led.setPixelColor(l, ref->led.Color(c, c, c));
            if(j > 12) ref->animationRunning = false;
          }
          else
          {
            ref->ledMode = LED_MODE_NONE;
          }
          break;
        case LED_MODE_POWER_OFF:
          if(ref->animationRunning)
          {
            int j = (millis() - ref->animationTimer) / 100;
            uint8_t c = 4 - abs(i * 10 - 45) / 10 <= j? 255 : 0;
            ref->led.setPixelColor(l, ref->led.Color(c, c, c));
            if(j > 12) ref->animationRunning = false;
          }
          else
          {
            ref->ledMode = LED_MODE_NONE;
          }
          break;
        case LED_MODE_CARD_INSERTED:
          if(ref->animationRunning)
          {
            int j = (millis() - ref->animationTimer) / 100;
            uint8_t c = (millis() % 200) > 100? 255 : 0;
            ref->led.setPixelColor(l, ref->led.Color(c, c, c));     // White blinking for 2s
            if(j > 20) ref->animationRunning = false;
          }
          else
          {
            ref->ledMode = LED_MODE_NONE;
          }
          break;
        case LED_MODE_SUCCESS:
          ref->animationRunning = false;
          ref->led.setPixelColor(l, ref->led.Color(0, 255, 0));   // Green steady
          break;
        case LED_MODE_NODE_STATUS:
          ref->animationRunning = false;
          ref->led.setPixelColor(l, 0);   // LED OFF per default
          if(ref->nodeStatus[i] == NODE_DISCONNECTED)
          {
            ref->led.setPixelColor(l, ref->led.Color(255, 0, 0));     // Red steady
          }
          else if(ref->nodeStatus[i] == NODE_ACTIVE)
          {
            ref->led.setPixelColor(l, ref->led.Color(0, 255, 0));     // Green steady
          }
          else if(ref->nodeStatus[i] == NODE_CONNECTED)
          {
            ref->led.setPixelColor(l, ref->led.Color(255, 255, 0));   // Yellow steady
          }
          break;
        case LED_MODE_NONE:
          ref->animationRunning = false;
          ref->led.setPixelColor(l, 0);
          break;
      }
    }
    //taskENTER_CRITICAL(0);
    ref->led.show();
    //taskEXIT_CRITICAL(0);


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
          #ifndef DISABLE_BUZZER
            ledcWriteTone(BUZZER_PWM_CHANNEL, freq);
          #endif
        }
      }
    }
  
    vTaskDelayUntil(&task_last_tick, (const TickType_t) 1000 / TASK_HMI_FREQ);
  }
  vTaskDelete(NULL);
}


