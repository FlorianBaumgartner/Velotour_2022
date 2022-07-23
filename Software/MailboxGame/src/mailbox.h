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
    Mailbox(int pinNfcRst, int pinNfcIrq, int pinNfcScl, int pinNfcSda, System& system, Hmi& hmi, Mesh& mesh):  pinNfcRst(pinNfcRst),
                                                                                                                pinNfcIrq(pinNfcIrq),
                                                                                                                pinNfcScl(pinNfcScl),
                                                                                                                pinNfcSda(pinNfcSda),
                                                                                                                system(system),
                                                                                                                hmi(hmi),
                                                                                                                mesh(mesh) {}
    bool begin(void);

  private:
    const int pinNfcRst;
    const int pinNfcIrq;
    const int pinNfcScl;
    const int pinNfcSda;

    System& system;
    Hmi& hmi;
    Mesh& mesh;

    const uint8_t devAddr = 0x28;
    TwoWire i2cBus = TwoWire(0);
    MFRC522_I2C dev = MFRC522_I2C(pinNfcRst, devAddr, i2cBus);
    MFRC522 mfrc522 = MFRC522(&dev);

    static void update(void* pvParameter);
    uint32_t readCardData(void);
};

#endif
