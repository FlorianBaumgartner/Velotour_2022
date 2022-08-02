/******************************************************************************
* file    mailbox.h
*******************************************************************************
* brief   Main class for handling all mailbox related tasks
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

#ifndef MAILBOX_H

#define MAILBOX_H

#include <Arduino.h>
#include <MFRC522.h>
#include "system.h"
#include "hmi.h"
#include "mesh.h"

#define TASK_MAILBOX_FREQ         10            // [Hz]
#define WIN_STATE_TIMEOUT         120           // [s]
#define NO_CARD_TIMEOUT           20            // [s]
#define MFRC522_VERSION_ID        0x92

class Mailbox
{
  public:
    Mailbox(int pinNfcRst, int pinNfcIrq, int pinNfcScl, int pinNfcSda, System& sys, Hmi& hmi, Mesh& mesh): pinNfcRst(pinNfcRst),
                                                                                                            pinNfcIrq(pinNfcIrq),
                                                                                                            pinNfcScl(pinNfcScl),
                                                                                                            pinNfcSda(pinNfcSda),
                                                                                                            sys(sys),
                                                                                                            hmi(hmi),
                                                                                                            mesh(mesh) {}
    bool begin(void);
    
  private:
    enum State {STATE_READY, STATE_ACTIVE, STATE_WIN, STATE_POWERDOWN};

    const int pinNfcRst;
    const int pinNfcIrq;
    const int pinNfcScl;
    const int pinNfcSda;

    System& sys;
    Hmi& hmi;
    Mesh& mesh;

    bool nfcInitialized = false;
    const uint8_t devAddr = MFRC522_I2C_DEFAULT_ADDR;
    TwoWire i2cBus = TwoWire(0);
    MFRC522_I2C dev = MFRC522_I2C(pinNfcRst, devAddr, i2cBus);
    MFRC522 mfrc522 = MFRC522(&dev);

    static void update(void* pvParameter);
    uint32_t readCardData(void);
    bool initializeNfc(void);
};

#endif
