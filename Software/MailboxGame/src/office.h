#ifndef OFFICE_H
#define OFFICE_H

#include <Arduino.h>
#include "system.h"
#include "hmi.h"
#include "mesh.h"

#define TASK_OFFICE_FREQ          10            // [Hz]
#define PRINT_INTERVAL            1000          // [ms]
#define SUCCESS_STATE_TIMEOUT     120           // [s]

class Office
{
  public:
    enum MailboxStatus {MAILBOX_IGNORED, MAILBOX_DISCONNECTED, MAILBOX_CONNECTED, MAILBOX_ACTIVE};      // Active means valid card is present

    Office(System& sys, Hmi& hmi, Mesh& mesh): sys(sys), hmi(hmi), mesh(mesh) {}
    bool begin(void);

  private:
    enum State {STATE_READY, STATE_WIN, STATE_POWERDOWN};

    System& sys;
    Hmi& hmi;
    Mesh& mesh;
    MailboxStatus mailboxStatus[MAX_NODES_NUM];
    uint32_t mailboxCompareCode[MAX_NODES_NUM];
    State state = STATE_READY;
    uint32_t returnPayload = 0x00000000;
    uint32_t stateTimer = -1;
    Hmi::LedResult solution = Hmi::LED_RESULT_NONE;

    static void update(void* pvParameter);
    void printInfo(void);
    bool loadMailboxFile(const char* path);
    bool loadOfficeFile(const char* path);
};

#endif
