/******************************************************************************
* file    hmi.h
*******************************************************************************
* brief   Controls onboard LEDs and buzzer for playing sounds
*******************************************************************************
* author  Florian Baumgartner
* version 1.0
* date    2022-08-02
*******************************************************************************
* MIT License
*
* Copyright (c) 2022 Crelin - Florian Baumgartner
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell          
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
******************************************************************************/

#ifndef HMI_H
#define HMI_H

#include <Arduino.h>
#include "Adafruit_NeoPixel.h"

#define NUM_LEDS_STATUS           1
#define NUM_LEDS_RESULT           3
#define NUM_LEDS_NODE             10
#define BUZZER_PWM_CHANNEL        1
#define LED_MAX_BRIGHTNESS        150

#define TASK_HMI_FREQ             30            // [Hz]

class Hmi
{
  public:
    enum LedStatus {LED_STATUS_OFF, LED_STATUS_BUSY, LED_STATUS_OK, LED_STATUS_ERROR, LED_STATUS_LOW_BATTERY};
    enum LedResult {LED_RESULT_NONE, LED_RESULT_A, LED_RESULT_B, LED_RESULT_C};
    enum LedMode {LED_MODE_NONE, LED_MODE_POWER_ON, LED_MODE_POWER_OFF, LED_MODE_SUCCESS, LED_MODE_CARD_INSERTED, LED_MODE_NODE_STATUS};
    enum NodeStatus {NODE_IGNORED, NODE_DISCONNECTED, NODE_CONNECTED, NODE_ACTIVE};   // Active means, node is connected and card has been inserted
    enum BuzzerSound {BUZZER_NONE, BUZZER_CARD_INSERTED, BUZZER_CARD_REMOVED, BUZZER_POWER_ON, BUZZER_POWER_OFF, BUZZER_SUCCESS, BUZZER_ERROR};
   

    Hmi(int pinLed, int pinBuzzer): pinLed(pinLed), pinBuzzer(pinBuzzer) {}
    void begin(void);
    void end(void);
    void setStatusIndicator(LedStatus status);
    void setResultIndicator(LedResult result);
    void setMode(LedMode mode);
    LedMode getMode(void) {return ledMode;}
    void setNodeStatus(int node, NodeStatus status);
    void playSound(BuzzerSound sound);
    bool isPlaying(void) {return playing;}


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
    LedMode ledMode = LED_MODE_NONE;
    NodeStatus nodeStatus[NUM_LEDS_NODE];
    BuzzerSound buzzerSound = BUZZER_NONE;
    uint32_t animationTimer = -1;
    bool animationRunning = false;
    bool initialized = false;

    const Tone* melody = nullptr;
    int melodyLength = 0;
    volatile bool playing = false;

    const Tone TONE_POWER_ON[4] = {{784, 120}, {0, 25}, {1175, 120}, {0, 200}};
    const Tone TONE_POWER_OFF[4] = {{1175, 120}, {0, 25}, {784, 120}, {0, 200}};
    const Tone TONE_CARD_INSERTED[4] = {{1175, 120}, {0, 25}, {1175, 120}, {0, 200}};
    const Tone TONE_CARD_REMOVED[2] = {{784, 200}, {0, 200}};
    const Tone TONE_SUCCESS[4] = {{784, 150}, {988, 150}, {1175, 150}, {1568, 150}};
    const Tone TONE_ERROR[16] = {{988, 200}, {0, 200}, {988, 200}, {0, 200}, {988, 200}, {0, 200}, {988, 200}, {0, 200},
                                 {988, 200}, {0, 200}, {988, 200}, {0, 200}, {988, 200}, {0, 200}, {988, 200}, {0, 200}};

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
