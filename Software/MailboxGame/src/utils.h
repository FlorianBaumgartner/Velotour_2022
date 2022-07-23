/*
 * Fleet-Monitor Software
 * Copyright (C) 2021 Institute of Networked Solutions OST
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

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


