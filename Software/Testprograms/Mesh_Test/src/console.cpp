#include "console.h"

bool Console::begin(void)
{
  if(type == USBCDC_t)
  {
    (*(USBCDC*)&stream).enableReboot(true);     // Enables entering bootloader when changing to baudrate of 1200 bit/s (normaly not used, due to dedicated DFU USB-Endpoint)
    (*(USBCDC*)&stream).begin();
  }
  else return false;
  return initialize();
}

bool Console::begin(unsigned long baud, uint32_t config, int8_t rxPin, int8_t txPin, bool invert, unsigned long timeout_ms, uint8_t rxfifo_full_thrhd)
{
  if(type == HardwareSerial_t)
  {
    (*(HardwareSerial*)&stream).begin(baud, config, rxPin, txPin, invert, timeout_ms, rxfifo_full_thrhd);
  }
  else return false;
  return initialize();
}

bool Console::initialize(void)
{
  bufferAccessSemaphore = xSemaphoreCreateMutex();
  xTaskCreate(writeTask, "task_consoleWrite", 1024, this, 1, &writeTaskHandle);
  xTaskCreate(interfaceTask, "task_consoleIface", 4096, this, 1, NULL);    // TODO: Stack size must be that large?!
  initialized = true;
  return true;
}

void Console::end(void)
{
  initialized = false;
}

void Console::writeTask(void *pvParameter)
{
  Console* ref = (Console*)pvParameter;

  while(ref->initialized)
  {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);          // Wait on notification for data in buffer or console opened
    if(ref->streamActive)
    {
      if(xSemaphoreTake(ref->bufferAccessSemaphore, portMAX_DELAY))
      {
        if(ref->readIdx < ref->writeIdx)              // Regular case, no wrap around needed
        {
          ref->stream.write((const uint8_t*) ref->ringBuffer + ref->readIdx, ref->writeIdx - ref->readIdx);
        }
        else if(ref->readIdx > ref->writeIdx)         // Need to send buffer in two parts (ReadIdx to End | 0 to WriteIdx)
        {
          ref->stream.write((const uint8_t*) ref->ringBuffer + ref->readIdx, QUEUE_BUFFER_LENGTH - ref->readIdx);
          ref->stream.write((const uint8_t*) ref->ringBuffer, ref->writeIdx);
        }
        ref->readIdx = ref->writeIdx;
        xSemaphoreGive(ref->bufferAccessSemaphore);
      }
    }
  }
  vTaskDelete(NULL);
}

void Console::interfaceTask(void *pvParameter)
{
  Console* ref = (Console*)pvParameter;

  TickType_t interfaceTimer = 0;
  TickType_t enabledTimer = 0;
  bool enabledOld = false, enabledDelayed = false;
  bool interfaceOld = false, interfaceDelayed = false;
  bool streamActiveOld = false;
  while(ref->initialized)
  {
    TickType_t task_last_tick = xTaskGetTickCount();

    if(ref->enabled && !enabledOld)
    {
      enabledTimer = xTaskGetTickCount() + CONSOLE_ACTIVE_DELAY;
    }
    enabledOld = ref->enabled;
    enabledDelayed = (xTaskGetTickCount() > enabledTimer) && ref->enabled;

    if(ref->getInterfaceState() && !interfaceOld)
    {
      interfaceTimer = xTaskGetTickCount() + INTERFACE_ACTIVE_DELAY;
    }
    interfaceOld = ref->getInterfaceState();
    interfaceDelayed = (xTaskGetTickCount() > interfaceTimer) && ref->getInterfaceState();

    ref->streamActive = enabledDelayed && interfaceDelayed;
    if(ref->streamActive && !streamActiveOld)
    {
      ref->printStartupMessage();
      vTaskDelay((const TickType_t) 10);                   // Make sure that startup message is printed befor everything else
      xTaskNotifyGive(ref->writeTaskHandle);               // Send signal to update task (for sending out data in queue buffer)
    }
    streamActiveOld = ref->streamActive;

    vTaskDelayUntil(&task_last_tick, (const TickType_t) 1000 / INTERFACE_UPDATE_RATE);
  }
  vTaskDelete(NULL);
}

size_t Console::write(uint8_t c)
{
  return write(&c, 1);
}

size_t Console::write(const uint8_t *buffer, size_t size)
{
  if(size == 0) return 0;
  size = min(size, (size_t) QUEUE_BUFFER_LENGTH - 1);
  if(xSemaphoreTake(bufferAccessSemaphore, portMAX_DELAY))
  {
    int free;
    if(writeIdx + size <= QUEUE_BUFFER_LENGTH)
    {
      memcpy((uint8_t*) ringBuffer + writeIdx, buffer, size);
      free = QUEUE_BUFFER_LENGTH - (writeIdx - readIdx);
    }
    else
    {
      int firstPartSize = QUEUE_BUFFER_LENGTH - writeIdx;
      memcpy((uint8_t*) ringBuffer + writeIdx, buffer, firstPartSize);
      memcpy((uint8_t*) ringBuffer, buffer + firstPartSize, size - firstPartSize);
      free = readIdx - writeIdx;
    } 
    writeIdx = (writeIdx + size) & (QUEUE_BUFFER_LENGTH - 1);
    if(size > free)
    {
      readIdx = (readIdx + (size - free)) & (QUEUE_BUFFER_LENGTH - 1);
    }

    xSemaphoreGive(bufferAccessSemaphore);
    xTaskNotifyGive(writeTaskHandle);               // Send signal to update task (for sending out data)
    return size;
  }
  return 0;
}

void Console::printStartupMessage(void)
{
  stream.printf(CLEAR_TERMINAL);
  stream.printf("\033[0;32;49m");
  stream.println("****************************************************");
  stream.println("*                  ESP32-S2 Utility                *");
  stream.println("*             2022, Florian Baumgartner            *");
  stream.println("****************************************************");
  stream.printf("\033[0;39;49m");
  stream.println();
}


#ifndef USE_CUSTOM_CONSOLE
  USBCDC USBSerial;
  Console console(USBSerial);
#endif
