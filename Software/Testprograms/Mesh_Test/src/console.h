#ifndef CONSOLE_H
#define CONSOLE_H

#include <Arduino.h>
#include "USB.h"

#define INTERFACE_UPDATE_RATE     10            // [hz]
#define QUEUE_BUFFER_LENGTH       2048          // [#]    Buffer Size must be power of 2
#define CONSOLE_ACTIVE_DELAY      1500          // [ms]   Data transmission hold-back delay after console object has been enabled
#define INTERFACE_ACTIVE_DELAY    1500          // [ms]   Data transmission hold-back delay after physical connection has been established (Terminal opened)

#define CLEAR_TERMINAL "\033[2J\033[1;1H"

class Console: public Stream
{
  private:
    enum SerialType {USBCDC_t, HardwareSerial_t};
    Stream &stream;
    SerialType type;
    volatile bool initialized = false;
    volatile bool enabled = false;               // Indicates if the stream is enabled (e.g. is set after USB MSC setup is done)
    volatile bool streamActive = false;          // Indicates if the console is opened and data is tranmitted
    volatile char ringBuffer[QUEUE_BUFFER_LENGTH];
    volatile uint32_t writeIdx = 0, readIdx = 0;
    SemaphoreHandle_t bufferAccessSemaphore = nullptr;
    TaskHandle_t writeTaskHandle = nullptr;

    bool initialize(void);
    void printStartupMessage(void);
    static void writeTask(void *pvParameter);
    static void interfaceTask(void *pvParameter);
    bool getInterfaceState(void)
    {
      if(type == USBCDC_t)
      {
        return *(USBCDC*)&stream;
      }
      else if (type == HardwareSerial_t)
      {
        return *(HardwareSerial*)&stream;
      }
      return true;
    }
    
  public:
    Console(USBCDC &stream): stream(stream), type(USBCDC_t) {}
    Console(HardwareSerial &stream): stream(stream), type(HardwareSerial_t) {}
    bool begin(void);       // Used for USBSerial
    bool begin(unsigned long baud, uint32_t config=SERIAL_8N1, int8_t rxPin=-1, int8_t txPin=-1, bool invert=false, unsigned long timeout_ms = 20000UL, uint8_t rxfifo_full_thrhd = 112);    // Used for HardwareSerial
    void end(void);
    void enable(bool state) {enabled = state;}
    void flush(void) {readIdx = writeIdx;}

    int available(void) {return stream.available();}
    int read(void) {return stream.read();}
    int peek(void) {return stream.peek();}
    size_t write(uint8_t c);
    size_t write(const uint8_t *buffer, size_t size);
    operator bool() const {return streamActive;}
};

#ifndef USE_CUSTOM_CONSOLE
  //extern USBCDC USBSerial;
  extern Console console;
#endif

#endif
