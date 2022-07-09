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

#include "SystemParser.h"
#include "console.h"
#include "utils.h"
#include "USB.h"

SystemParser::SystemParser(void) {}

/**
 * @brief Load a system configuration file
 *
 * @param path is the path of the file
 * @return true on success
 * @return false on error
 */
bool SystemParser::loadFile(const char* path) {
  filePath = path;
  File file = fatfs.open(filePath);

  if(!file)
  {
    console.println("open file failed");
    return false;
  }

  DeserializationError error = deserializeJson(doc, file);
  if(error)
  {
    file.close();
    console.printf("Failed to read file, using default configuration: %d\n", error);
    return false;
  }

  file.close();
  return true;
}

/**
 * @brief Get the USB Vender Identifier
 *
 * @return uin16_t with the USB VID
 */
uint16_t SystemParser::getUsbVid(void)
{
  if(doc.containsKey("usb_vid"))
  {
    return (uint16_t)strtol(doc["usb_vid"].as<const char*>(), NULL, 0);
  }
  return -1;
}

/**
 * @brief Get the USB Product Identifier
 *
 * @return uin16_t with the USB PID
 */
uint16_t SystemParser::getUsbPid(void)
{
  if(doc.containsKey("usb_pid"))
  {
    return (uint16_t)strtol(doc["usb_pid"].as<const char*>(), NULL, 0);
  }
  return -1;
}

/**
 * @brief Get the USB Serial Number
 *
 * @return const char* with the serial number as string
 */
const char* SystemParser::getUsbSerial(void)
{
  if(doc.containsKey("usb_serial"))
  {
    return doc["usb_serial"].as<const char*>();
  }
  return "";
}

/**
 * @brief Get the name of the access point
 *
 * @return const char* with the name
 */
const char* SystemParser::getSsid(void)
{
  if(doc.containsKey("ssid"))
  {
    return doc["ssid"].as<const char*>();
  }
  return "";
}

/**
 * @brief get the password of the access point
 *
 * @return const char* with the password
 */
const char* SystemParser::getPassword(void)
{
  if(doc.containsKey("password"))
  {
    return doc["password"].as<const char*>();
  }
  return "";
}

/**
 * @brief Save the current loaded system config as a file
 *
 * @param path to location
 * @return true on success
 * @return false on error
 */
bool SystemParser::saveFile(const char* path) {
  if(path != NULL)
  {
    filePath = path;
  }
  if(fatfs.exists(filePath))
  {
    if(!fatfs.remove(filePath)) 
    {
      console.println("Could not remove file");
      return false;
    }
  }
  File file = fatfs.open(filePath, FILE_WRITE);
  if(!file)
  {
    console.println("open file failed");
    return false;
  }
  if(serializeJson(doc, file) == 0)
  {
    file.close();
    console.println("Failed to write to file");
    return false;
  }
  file.close();
  return true;
}