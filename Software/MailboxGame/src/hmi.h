#ifndef HMI_H
#define HMI_H

#include <Arduino.h>
#include "Adafruit_NeoPixel.h"

#define NUM_LEDS_STATUS           1
#define NUM_LEDS_RESULT           3
#define NUM_LEDS_NODE             10

#define TASK_HMI_FREQ             20            // [Hz]


class Hmi
{
  public:
    enum LedStatus {LED_STATUS_BUSY, LED_STATUS_OK, LED_STATUS_ERROR};
    enum LedResult {LED_RESULT_NONE, LED_RESULT_A, LED_RESULT_B, LED_RESULT_C};
    enum LedNode {LED_NODE_DISCONNECTED, LED_NODE_CONNECTED};
    enum BuzzerSound {BUZZER_POWER_ON, BUZZER_POWER_OFF, BUZZER_SUCCESS};

    Hmi(int pinLed, int pinBuzzer): pinLed(pinLed), pinBuzzer(pinBuzzer) {}
    void begin(void);
    void setStatusIndicator(LedStatus status);
    void setResultIndicator(LedResult result);
    void setNodeIndicator(int node, LedNode status);
    void playSound(BuzzerSound sound);


  private:
    int pinLed, pinBuzzer;
    Adafruit_NeoPixel led;
    LedStatus statusIndicator = LED_STATUS_BUSY;
    LedResult resultIndicator = LED_RESULT_NONE;
    LedNode nodeIndicator[NUM_LEDS_NODE];

    static void update(void* pvParameter);
};

#endif