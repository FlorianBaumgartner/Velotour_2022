/******************************************************************************
* file    office.h
*******************************************************************************
* brief   Main class for handling all office related tasks
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

#ifndef OFFICE_H
#define OFFICE_H

#include <Arduino.h>
#include "system.h"
#include "hmi.h"
#include "mesh.h"

#define TASK_OFFICE_FREQ          10            // [Hz]
#define PRINT_INTERVAL            100           // [ms]
#define SUCCESS_STATE_TIMEOUT     90            // [s]
#define SHOW_OWN_NODE_STATUS      true

class Office
{
  public:
    Office(System& sys, Hmi& hmi, Mesh& mesh): sys(sys), hmi(hmi), mesh(mesh) {}
    bool begin(void);

  private:
    enum State {STATE_READY, STATE_WIN, STATE_POWERDOWN};

    System& sys;
    Hmi& hmi;
    Mesh& mesh;
    Hmi::LedResult solution = Hmi::LED_RESULT_NONE;
    Hmi::NodeStatus mailboxStatus[MAX_NODES_NUM];
    uint32_t mailboxCompareCode[MAX_NODES_NUM];
    uint32_t returnPayload = 0x00000000;
    
    static void update(void* pvParameter);
    void printInfo(bool forceUpdate = false);
    bool loadMailboxFile(const char* path);
    bool loadOfficeFile(const char* path);
};

#endif
