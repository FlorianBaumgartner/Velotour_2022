#ifndef CONSOLE_H
#define CONSOLE_H

#include <Arduino.h>
#include "USB.h"

#define INTERFACE_UPDATE_RATE           10            // [hz]
#define QUEUE_BUFFER_LENGTH             (1<<12)       // [#]    Buffer Size must be power of 2
#define CONSOLE_ACTIVE_DELAY            3000          // [ms]   Data transmission hold-back delay after console object has been enabled
#define INTERFACE_ACTIVE_DELAY          1500          // [ms]   Data transmission hold-back delay after physical connection has been established (Terminal opened)


#define CONSOLE_CLEAR                   "\033[2J\033[1;1H"

#define CONSOLE_COLOR_BLACK             "\033[0;30"
#define CONSOLE_COLOR_RED               "\033[0;31"
#define CONSOLE_COLOR_GREEN             "\033[0;32"
#define CONSOLE_COLOR_YELLOW            "\033[0;33"
#define CONSOLE_COLOR_BLUE              "\033[0;34"
#define CONSOLE_COLOR_MAGENTA           "\033[0;35"
#define CONSOLE_COLOR_CYAN              "\033[0;36"
#define CONSOLE_COLOR_WHITE             "\033[0;37"
#define CONSOLE_COLOR_DEFAULT           "\033[0;39"

#define CONSOLE_COLOR_BOLD_BLACK        "\033[1;30"
#define CONSOLE_COLOR_BOLD_RED          "\033[1;31"
#define CONSOLE_COLOR_BOLD_GREEN        "\033[1;32"
#define CONSOLE_COLOR_BOLD_YELLOW       "\033[1;33"
#define CONSOLE_COLOR_BOLD_BLUE         "\033[1;34"
#define CONSOLE_COLOR_BOLD_MAGENTA      "\033[1;35"
#define CONSOLE_COLOR_BOLD_CYAN         "\033[1;36"
#define CONSOLE_COLOR_BOLD_WHITE        "\033[1;37"
#define CONSOLE_COLOR_BOLD_DEFAULT      "\033[1;39"

#define CONSOLE_BACKGROUND_BLACK        ";40m"
#define CONSOLE_BACKGROUND_RED          ";41m"
#define CONSOLE_BACKGROUND_GREEN        ";42m"
#define CONSOLE_BACKGROUND_YELLOW       ";43m"
#define CONSOLE_BACKGROUND_BLUE         ";44m"
#define CONSOLE_BACKGROUND_MAGENTA      ";45m"
#define CONSOLE_BACKGROUND_CYAN         ";46m"
#define CONSOLE_BACKGROUND_WHITE        ";47m"
#define CONSOLE_BACKGROUND_DEFAULT      ";49m"

#define CONSOLE_OK                      (CONSOLE_COLOR_GREEN CONSOLE_BACKGROUND_DEFAULT)
#define CONSOLE_LOG                     (CONSOLE_COLOR_DEFAULT CONSOLE_BACKGROUND_DEFAULT)
#define CONSOLE_ERROR                   (CONSOLE_COLOR_RED CONSOLE_BACKGROUND_DEFAULT)
#define CONSOLE_WARNING                 (CONSOLE_COLOR_YELLOW CONSOLE_BACKGROUND_DEFAULT)

#define DISABLE_MODULE_LEVEL            dummy

class ConsoleStatus: public Stream
{
  public:
    Stream* console = nullptr;
    enum ConsoleType {StatusOk_t, StatusLog_t, StatusWarning_t, StatusError_t, StatusDummy_t};
    ConsoleType type;
    bool enabled = true;
    
    ConsoleStatus(ConsoleType t): type(t) {}
    inline void ref(Stream* c) {console = c;}
    inline void enable(bool s) {enabled = s;}
    inline int available(void) {return console->available();}
    inline int read(void) {return console->read();}
    inline int peek(void) {return console->peek();}
    inline size_t write(uint8_t c) {return write((const uint8_t*) &c, 1);}
    inline size_t write(const char* buffer, size_t size) {return write((uint8_t*) buffer, size);}
    size_t write(const uint8_t* buffer, size_t size)
    {
      if(!enabled || type == StatusDummy_t) return 0;
      switch(type)
      {
        case StatusOk_t:
          console->print(CONSOLE_OK);
          break;
        case StatusWarning_t:
          console->print(CONSOLE_WARNING);
          break;
        case StatusError_t:
          console->print(CONSOLE_ERROR);
          break;
        default:
          console->print(CONSOLE_LOG);
          break;
      }
      size = console->write((const uint8_t*) buffer, size);
      console->print(CONSOLE_LOG);
      return size;
    }
};


class Console: public Stream
{
  private:
    enum SerialType {Virtual_t, USBCDC_t, HardwareSerial_t};
    Stream &stream;
    SerialType type = Virtual_t;
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
    enum ConsoleLevel {LEVEL_LOG = 0, LEVEL_OK = 1, LEVEL_WARNING = 2, LEVEL_ERROR = 3, LEVEL_OFF = 4};
    ConsoleStatus ok = ConsoleStatus(ConsoleStatus::StatusOk_t);
    ConsoleStatus log = ConsoleStatus(ConsoleStatus::StatusLog_t);
    ConsoleStatus error = ConsoleStatus(ConsoleStatus::StatusError_t);
    ConsoleStatus warning = ConsoleStatus(ConsoleStatus::StatusWarning_t);
    ConsoleStatus dummy = ConsoleStatus(ConsoleStatus::StatusDummy_t);
  
    Console(USBCDC &stream): stream(stream), type(USBCDC_t) {ok.ref(this); log.ref(this); error.ref(this); warning.ref(this); dummy.ref(this);}
    Console(HardwareSerial &stream): stream(stream), type(HardwareSerial_t) {ok.ref(this); log.ref(this); error.ref(this); warning.ref(this); dummy.ref(this);}
    bool begin(void);                 // Used for USBSerial
    bool begin(unsigned long baud, uint32_t config=SERIAL_8N1, int8_t rxPin=-1, int8_t txPin=-1, bool invert=false, unsigned long timeout_ms = 20000UL, uint8_t rxfifo_full_thrhd = 112);    // Used for HardwareSerial
    void end(void);
    void enable(bool state) {enabled = state;}
    void flush(void) {readIdx = writeIdx;}
    void printTimestamp(void);        // TODO: Add possibillity to add string as parameter
    void setLevel(ConsoleLevel level)
    {
      log.enable(level <= LEVEL_LOG);
      ok.enable(level <= LEVEL_OK);
      warning.enable(level <= LEVEL_WARNING);
      error.enable(level <= LEVEL_ERROR);
    }

    operator bool() const {return streamActive;}
    inline int available(void) {return stream.available();}
    inline int read(void) {return stream.read();}
    inline int peek(void) {return stream.peek();}
    inline size_t write(uint8_t c) {return write(&c, 1);}
    inline size_t write(const char* buffer, size_t size) {return write((const uint8_t*) buffer, size);}
    inline size_t write(const char * s) {return write((const uint8_t*) s, strlen(s));}
    inline size_t write(unsigned long n) {return write((uint8_t) n);}
    inline size_t write(long n) {return write((uint8_t) n);}
    inline size_t write(unsigned int n) {return write((uint8_t) n);}
    inline size_t write(int n) {return write((uint8_t) n);}
    size_t write(const uint8_t *buffer, size_t size);    
};


#ifndef USE_CUSTOM_CONSOLE
  extern Console console;
#endif

#endif
