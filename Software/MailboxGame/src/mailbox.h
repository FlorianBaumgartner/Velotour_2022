#ifndef MAILBOX_H
#define MAILBOX_H

#include <Arduino.h>
#include <MFRC522.h>
#include "system.h"
#include "hmi.h"
#include "mesh.h"

#define TASK_MAILBOX_FREQ         10            // [Hz]
#define WIN_STATE_TIMEOUT         120           // [s]
#define NO_CARD_TIMEOUT           20            // [s]
#define MFRC522_VERSION_ID        0x92

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
    const uint8_t devAddr = MFRC522_I2C_DEFAULT_ADDR;
    TwoWire i2cBus = TwoWire(0);
    MFRC522_I2C dev = MFRC522_I2C(pinNfcRst, devAddr, i2cBus);
    MFRC522 mfrc522 = MFRC522(&dev);

    static void update(void* pvParameter);
    uint32_t readCardData(void);
    bool initializeNfc(void);
};

#endif
