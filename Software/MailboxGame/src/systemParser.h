/*
 * Fleet-Monitor Software
 * Copyright (C) Florian Baumgartner 2022
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
#include <ArduinoJson.h>

#define MAX_SYSTEM_FILE_SIZE 1 * 1024

class SystemParser {
 public:
  SystemParser(void);
  bool loadFile(const char* path);
  uint16_t getUsbVid(void);
  uint16_t getUsbPid(void);
  const char* getUsbSerial(void);
  const char* getSsid(void);
  const char* getPassword(void);

 private:
  StaticJsonDocument<MAX_SYSTEM_FILE_SIZE> doc;
  const char* filePath;

  bool saveFile(const char* path = NULL);
};