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

#include "utils.h"
#include "Console.h"
#include "SPI.h"
#include "Adafruit_SPIFlash.h"
#include "Adafruit_TinyUSB.h"
#include "SystemParser.h"

#include "format/ff.h"
#include "format/diskio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static void task(void* pvParameter);
static int32_t msc_read_cb(uint32_t lba, void* buffer, uint32_t bufsize);
static int32_t msc_write_cb(uint32_t lba, uint8_t* buffer, uint32_t bufsize);
static void msc_flush_cb(void);
static volatile bool updated = false;
static volatile bool connected = false;

Adafruit_USBD_MSC usb_msc;
Adafruit_FlashTransport_ESP32 flashTransport;
Adafruit_SPIFlash flash(&flashTransport);
FatFileSystem fatfs;
SystemParser systemParser;

Utils::Utils(void)
{
}

bool Utils::begin(const char* labelName, bool forceFormat)
{
  if(!flash.begin())
  {
    return false;
  }
  if(!fatfs.begin(&flash) || forceFormat)  // Check if disk must be formated
  {
    if(!format(labelName))
    {
      return false;
    }
  }
  delay(200);

  uint16_t vid = USB_VID;
  uint16_t pid = USB_PID;

  if(systemParser.loadFile(CONFIG_FILE_NAME))
  {
    if(systemParser.getUsbVid() != -1)
    {
      vid = systemParser.getUsbVid();
    }
    if(systemParser.getUsbPid() != -1)
    {
      pid = systemParser.getUsbPid();
    }
    if(strlen(systemParser.getUsbSerial()))
    {
      serial = systemParser.getUsbSerial();
    }
    ssid = systemParser.getSsid();
    password = systemParser.getPassword();
    console.println("[UTILS] System config loading was successful.");
  }
  else
  {
    console.println("[UTILS] System config loading failed.");
  }

  USB.VID(vid);
  USB.PID(pid);
  USB.serialNumber(serial);
  USB.enableDFU();
  USB.productName(USB_PRODUCT);
  USB.manufacturerName(USB_MANUFACTURER);
  USB.begin();

  usb_msc.setID(USB_MANUFACTURER, USB_PRODUCT, FIRMWARE_VERSION);
  usb_msc.setReadWriteCallback(msc_read_cb, msc_write_cb, msc_flush_cb);  // Set callback
  usb_msc.setCapacity(flash.size() / 512, 512);    // Set disk size, block size should be 512 regardless of spi flash page size
  usb_msc.begin();

  xTimerPendFunctionCall(startMsc, this, 0, (const TickType_t) MSC_STARTUP_DELAY);
  return true;
}

void Utils::startMsc(void *pvParameter1, uint32_t ulParameter2)
{
  usb_msc.setUnitReady(true);                 // MSC is ready for read/write
  console.enable(true);
}

bool Utils::isConnected(void)
{
  return connected;
}

bool Utils::isUpdated(bool clearFlag)
{
  bool status = updated;
  if(clearFlag) updated = false;
  return status;
}

bool Utils::format(const char* labelName)
{
  static FATFS elmchamFatfs;
  static uint8_t workbuf[4096];  // Working buffer for f_fdisk function.

  console.println("[UTILS] Partitioning flash with 1 primary partition...");
  static DWORD plist[] = {100, 0, 0, 0};      // 1 primary partition with 100% of space.
  static uint8_t buf[512] = {0};              // Working buffer for f_fdisk function.
  static FRESULT r = f_fdisk(0, plist, buf);  // Partition the flash with 1 partition that takes the entire space.
  if(r != FR_OK)
  {
    console.print("[UTILS] Error, f_fdisk failed with error code: ");
    console.println(r, DEC);
    return 0;
  }
  console.println("[UTILS] Partitioned flash!");
  console.println("[UTILS] Creating and formatting FAT filesystem (this takes ~60 seconds)...");
  r = f_mkfs("", FM_FAT | FM_SFD, 0, workbuf, sizeof(workbuf));  // Make filesystem.
  if(r != FR_OK)
  {
    console.print("[UTILS] Error, f_mkfs failed with error code: ");
    console.println(r, DEC);
    return 0;
  }

  r = f_mount(&elmchamFatfs, "0:", 1);  // mount to set disk label
  if (r != FR_OK)
  {
    console.print("[UTILS] Error, f_mount failed with error code: ");
    console.println(r, DEC);
    return 0;
  }
  console.print("[UTILS] Setting disk label to: ");
  console.println(labelName);
  r = f_setlabel(labelName);  // Setting label
  if (r != FR_OK)
  {
    console.print("[UTILS] Error, f_setlabel failed with error code: ");
    console.println(r, DEC);
    return 0;
  }
  f_unmount("0:");     // unmount
  flash.syncBlocks();  // sync to make sure all data is written to flash
  console.println("[UTILS] Formatted flash!");
  if (!fatfs.begin(&flash))  // Check new filesystem
  {
    console.println("[UTILS] Error, failed to mount newly formatted filesystem!");
    return 0;
  }
  console.println("[UTILS] Flash chip successfully formatted with new empty filesystem!");
  yield();
  return 1;
}


// Callback invoked when received READ10 command.
// Copy disk's data to buffer (up to bufsize) and
// return number of copied bytes (must be multiple of block size)
static int32_t msc_read_cb(uint32_t lba, void* buffer, uint32_t bufsize)
{
  return flash.readBlocks(lba, (uint8_t*)buffer, bufsize / 512) ? bufsize : -1;
}

// Callback invoked when received WRITE10 command.
// Process data in buffer to disk's storage and
// return number of written bytes (must be multiple of block size)
static int32_t msc_write_cb(uint32_t lba, uint8_t* buffer, uint32_t bufsize)
{
  return flash.writeBlocks(lba, buffer, bufsize / 512) ? bufsize : -1;
}

// Callback invoked when WRITE10 command is completed (status received and accepted by host). Used to flush any pending
// cache.
static void msc_flush_cb(void)
{
  flash.syncBlocks();  // sync with flash
  fatfs.cacheClear();  // clear file system's cache to force refresh
  updated = true;
}

//--------------------------------------------------------------------+
// fatfs diskio
//--------------------------------------------------------------------+
extern "C" {
DSTATUS disk_status(BYTE pdrv) {
  (void)pdrv;
  return 0;
}

DSTATUS disk_initialize(BYTE pdrv) {
  (void)pdrv;
  return 0;
}

DRESULT disk_read(BYTE pdrv,     // Physical drive nmuber to identify the drive
                  BYTE* buff,    // Data buffer to store read data
                  DWORD sector,  // Start sector in LBA
                  UINT count     // Number of sectors to read
) {
  (void)pdrv;
  return flash.readBlocks(sector, buff, count) ? RES_OK : RES_ERROR;
}

DRESULT disk_write(BYTE pdrv,         // Physical drive nmuber to identify the drive
                   const BYTE* buff,  // Data to be written
                   DWORD sector,      // Start sector in LBA
                   UINT count         // Number of sectors to write
) {
  (void)pdrv;
  return flash.writeBlocks(sector, buff, count) ? RES_OK : RES_ERROR;
}

DRESULT disk_ioctl(BYTE pdrv,  // Physical drive nmuber (0..)
                   BYTE cmd,   // Control code
                   void* buff  // Buffer to send/receive control data
) {
  (void)pdrv;

  switch (cmd) {
    case CTRL_SYNC:
      flash.syncBlocks();
      return RES_OK;

    case GET_SECTOR_COUNT:
      *((DWORD*)buff) = flash.size() / 512;
      return RES_OK;

    case GET_SECTOR_SIZE:
      *((WORD*)buff) = 512;
      return RES_OK;

    case GET_BLOCK_SIZE:
      *((DWORD*)buff) = 8;  // erase block size in units of sector size
      return RES_OK;

    default:
      return RES_PARERR;
  }
}
}
