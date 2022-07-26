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
    enum LedMode {LED_MODE_OFF, LED_MODE_POWER_ON, LED_MODE_POWER_OFF, LED_MODE_SUCCESS, LED_MODE_CARD_INSERTED, LED_MODE_NODE_STATUS};
    enum NodeStatus {NODE_DISCONNECTED, NODE_CONNECTED, NODE_ACTIVE};   // Active means, node is connected and card has been inserted
    enum BuzzerSound {BUZZER_NONE, BUZZER_POWER_ON, BUZZER_POWER_OFF, BUZZER_SUCCESS, BUZZER_ERROR};    // TODO: Add sound for Card inserted and card removed
   

    Hmi(int pinLed, int pinBuzzer): pinLed(pinLed), pinBuzzer(pinBuzzer) {}
    void begin(void);
    void setStatusIndicator(LedStatus status);
    void setResultIndicator(LedResult result);
    void setMode(LedMode mode);
    void setNodeStatus(int node, NodeStatus status);
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
    LedMode ledMode = LED_MODE_OFF;
    NodeStatus nodeStatus[NUM_LEDS_NODE];
    BuzzerSound buzzerSound = BUZZER_NONE;
    uint32_t animationTimer = -1;
    bool animationRunning = false;

    const Tone* melody = nullptr;
    int melodyLength = 0;
    bool playing = false;

    const Tone TONE_POWER_ON[4] = {{784, 120}, {0, 20}, {1175, 120}, {0, 20}};
    const Tone TONE_POWER_OFF[4] = {{1175, 120}, {0, 20}, {784, 120}, {0, 20}};
    const Tone TONE_SUCCESS[4] = {{784, 150}, {988, 150}, {1175, 150}, {1568, 150}};
    const Tone TONE_ERROR[8] = {{784, 300}, {0, 300}, {784, 300}, {0, 300}, {784, 300}, {0, 300}, {784, 300}, {0, 300}};

    const uint8_t gamma [256] =
    {
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   1,   1,   1,   1,
        1,   1,   1,   1,   1,   1,   1,   1,   1,   2,   2,   2,   2,   2,   2,   2,
        2,   3,   3,   3,   3,   3,   3,   3,   4,   4,   4,   4,   4,   5,   5,   5,
        5,   6,   6,   6,   6,   7,   7,   7,   7,   8,   8,   8,   9,   9,   9,  10,
       10,  10,  11,  11,  11,  12,  12,  13,  13,  13,  14,  14,  15,  15,  16,  16,
       17,  17,  18,  18,  19,  19,  20,  20,  21,  21,  22,  22,  23,  24,  24,  25,
       25,  26,  27,  27,  28,  29,  29,  30,  31,  32,  32,  33,  34,  35,  35,  36,
       37,  38,  39,  39,  40,  41,  42,  43,  44,  45,  46,  47,  48,  49,  50,  50,
       51,  52,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,  64,  66,  67,  68,
       69,  70,  72,  73,  74,  75,  77,  78,  79,  81,  82,  83,  85,  86,  87,  89,
       90,  92,  93,  95,  96,  98,  99, 101, 102, 104, 105, 107, 109, 110, 112, 114,
      115, 117, 119, 120, 122, 124, 126, 127, 129, 131, 133, 135, 137, 138, 140, 142,
      144, 146, 148, 150, 152, 154, 156, 158, 160, 162, 164, 167, 169, 171, 173, 175,
      177, 180, 182, 184, 186, 189, 191, 193, 196, 198, 200, 203, 205, 208, 210, 213,
      215, 218, 220, 223, 225, 228, 231, 233, 236, 239, 241, 244, 247, 249, 252, 255
    };

    static void update(void* pvParameter);
};

#endif