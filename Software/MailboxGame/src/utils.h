/******************************************************************************
* file    utils.h
*******************************************************************************
* brief   General utilities for file system support, MSC, configuration, etc.
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
#pragma once

#include <Arduino.h>
#include "SdFat.h"


#define TASK_UTILS_FREQ           10            // [Hz]
#define MSC_STARTUP_DELAY         3500          // [ms]
#define CONFIG_FILE_NAME          "system.json"

extern FatFileSystem fatfs;

class Utils
{
  public:
    Utils(void);
    bool begin(const char* labelName, bool forceFormat = false);
    bool isUpdated(bool clearFlag = true);
    bool isConnected(void);
    bool format(const char* labelName);
    const char* getSerialNumber(void) {return serial;}
    const char* getSsid(void) {return (ssid[0] == '\0')? nullptr : ssid;}
    const char* getPassword(void) {return (password[0] == '\0')? nullptr : password;}
    operator bool() const {return mscReady;}

  private:
    const char* serial = "0";
    const char* ssid = nullptr;
    const char* password = nullptr;
    volatile bool mscReady = false;

    static void update(void* pvParameter);
};
