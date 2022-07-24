#ifndef HMI_H
#define HMI_H

#include <Arduino.h>
#include "Adafruit_NeoPixel.h"

#define NUM_LEDS_STATUS           1
#define NUM_LEDS_RESULT           3
#define NUM_LEDS_NODE             10
#define BUZZER_PWM_CHANNEL        1

#define TASK_HMI_FREQ             20            // [Hz]

class Hmi
{
  public:
    enum LedStatus {LED_STATUS_BUSY, LED_STATUS_OK, LED_STATUS_ERROR};
    enum LedResult {LED_RESULT_NONE, LED_RESULT_A, LED_RESULT_B, LED_RESULT_C};
    enum LedNode {LED_NODE_DISCONNECTED, LED_NODE_CONNECTED};
    enum BuzzerSound {BUZZER_NONE, BUZZER_POWER_ON, BUZZER_POWER_OFF, BUZZER_SUCCESS, BUZZER_ERROR};

    Hmi(int pinLed, int pinBuzzer): pinLed(pinLed), pinBuzzer(pinBuzzer) {}
    void begin(void);
    void setStatusIndicator(LedStatus status);
    void setResultIndicator(LedResult result);
    void setNodeIndicator(int node, LedNode status);
    void playSound(BuzzerSound sound);


  private:
    struct Tone
    {
      int freq;       // 0 means OFF
      int duration;   // Time in ms
    };

    int pinLed, pinBuzzer;
    Adafruit_NeoPixel led;
    LedStatus statusIndicator = LED_STATUS_BUSY;
    LedResult resultIndicator = LED_RESULT_NONE;
    LedNode nodeIndicator[NUM_LEDS_NODE];
    BuzzerSound buzzerSound = BUZZER_NONE;

    const Tone* melody = nullptr;
    int melodyLength = 0;
    bool playing = false;

    const Tone TONE_POWER_ON[4] = {{784, 120}, {0, 20}, {1175, 120}, {0, 20}};
    const Tone TONE_POWER_OFF[4] = {{1175, 120}, {0, 20}, {784, 120}, {0, 20}};
    const Tone TONE_SUCCESS[4] = {{784, 150}, {988, 150}, {1175, 150}, {1568, 150}};
    const Tone TONE_ERROR[8] = {{784, 300}, {0, 300}, {784, 300}, {0, 300}, {784, 300}, {0, 300}, {784, 300}, {0, 300}};

    static void update(void* pvParameter);
};

#endif