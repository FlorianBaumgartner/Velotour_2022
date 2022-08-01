#ifndef OFFICE_H
#define OFFICE_H

#include <Arduino.h>
#include "system.h"
#include "hmi.h"
#include "mesh.h"

#define TASK_OFFICE_FREQ          10            // [Hz]
#define PRINT_INTERVAL            100           // [ms]
#define SUCCESS_STATE_TIMEOUT     90            // [s]

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
