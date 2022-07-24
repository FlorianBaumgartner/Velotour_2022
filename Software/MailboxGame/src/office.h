#ifndef OFFICE_H
#define OFFICE_H

#include <Arduino.h>
#include "system.h"
#include "hmi.h"
#include "mesh.h"

#define TASK_OFFICE_FREQ          10            // [Hz]
#define PRINT_INTERVAL            1000          // [ms]

class Office
{
  public:
    enum MailboxStatus {MAILBOX_IGNORED, MAILBOX_DISCONNECTED, MAILBOX_CONNECTED, MAILBOX_ACTIVE};      // Active means valid card is present

    Office(System& system, Hmi& hmi, Mesh& mesh): system(system), hmi(hmi), mesh(mesh) {}
    bool begin(void);

  private:
    enum OfficeState {OFFICE_READY, OFFICE_WIN, OFFICE_POWERDOWN};

    System& system;
    Hmi& hmi;
    Mesh& mesh;
    MailboxStatus mailboxStatus[MAX_NODES_NUM];
    uint32_t mailboxCompareCode[MAX_NODES_NUM];
    OfficeState officeState = OFFICE_READY;
    uint32_t returnPayload = 0x00000000;
    Hmi::LedResult solution = Hmi::LED_RESULT_NONE;

    static void update(void* pvParameter);
    void printInfo(void);
    bool loadMailboxFile(const char* path);
    bool loadOfficeFile(const char* path);
};

#endif
