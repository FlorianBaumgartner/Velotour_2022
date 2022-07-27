#ifndef MAILBOX_H
#define MAILBOX_H

#include <Arduino.h>
#include <MFRC522.h>
#include "system.h"
#include "hmi.h"
#include "mesh.h"

#define TASK_MAILBOX_FREQ         10            // [Hz]

class Mailbox
{
  public:
    Mailbox(int pinNfcRst, int pinNfcIrq, int pinNfcScl, int pinNfcSda, System& sys, Hmi& hmi, Mesh& mesh): pinNfcRst(pinNfcRst),
                                                                                                            pinNfcIrq(pinNfcIrq),
                                                                                                            pinNfcScl(pinNfcScl),
                                                                                                            pinNfcSda(pinNfcSda),
                                                                                                            sys(sys),
                                                                                                            hmi(hmi),
                                                                                                            mesh(mesh) {}
    bool begin(void);

  private:
    enum State {STATE_READY, STATE_ACTIVE, STATE_WIN, STATE_POWERDOWN};

    const int pinNfcRst;
    const int pinNfcIrq;
    const int pinNfcScl;
    const int pinNfcSda;

    System& sys;
    Hmi& hmi;
    Mesh& mesh;

    bool nfcInitialized = false;
    const uint8_t devAddr = 0x28;
    TwoWire i2cBus = TwoWire(0);
    MFRC522_I2C dev = MFRC522_I2C(pinNfcRst, devAddr, i2cBus);
    MFRC522 mfrc522 = MFRC522(&dev);
    State state = STATE_READY;
    

    static void update(void* pvParameter);
    void initializeNfc(void);
    uint32_t readCardData(void);
};

#endif
