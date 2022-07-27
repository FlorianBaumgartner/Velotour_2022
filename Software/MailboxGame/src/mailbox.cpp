#include "mailbox.h"
#include "console.h"

#include "freertos/task.h"


bool Mailbox::begin(void)
{
  //initializeNfc();

  xTaskCreate(update, "task_mailbox", 2048, this, 1, NULL);
  return true;
}


void Mailbox::update(void* pvParameter)
{
  Mailbox* ref = (Mailbox*)pvParameter;

  while(true)
  {
    TickType_t task_last_tick = xTaskGetTickCount();

    uint8_t nodeStatus = ref->mesh.getNodePayload(0) >> ref->mesh.getPersonalId() & 0x03;
    bool allCorrect = (ref->mesh.getNodePayload(0) & 0x80000000) && ref->mesh.getNodeState(0);
    uint32_t card = ref->readCardData();
    ref->mesh.setPayload(card);

    switch(ref->state)
    {
      case STATE_READY:
        if(card != -1)
        {
          ref->hmi.setMode(Hmi::LED_MODE_CARD_INSERTED);
          ref->hmi.playSound(Hmi::BUZZER_CARD_INSERTED);
          ref->state = STATE_ACTIVE;
        }
        break;
      case STATE_ACTIVE:
        if(card == -1)
        {
          ref->hmi.playSound(Hmi::BUZZER_CARD_REMOVED);
          ref->state = STATE_READY;
        }
        if(allCorrect)
        {
          ref->hmi.playSound(Hmi::BUZZER_SUCCESS);
          ref->state = STATE_WIN;
        }
        break;
      case STATE_WIN:
        // TODO: Wait until node 0 disconnected or timeout
        break;
      case STATE_POWERDOWN:
        break;
    }

    vTaskDelayUntil(&task_last_tick, (const TickType_t) 1000 / TASK_MAILBOX_FREQ);
  }
  vTaskDelete(NULL);
}

void Mailbox::initializeNfc(void)
{
  if(!nfcInitialized)
  {
    i2cBus.begin(pinNfcSda, pinNfcScl, 400000U); 
    mfrc522.PCD_Init();     // Init MFRC522
    nfcInitialized = true;
  }
}

uint32_t Mailbox::readCardData(void)
{
  if(!nfcInitialized) return 0xFFFFFFFF;
  uint8_t req_buff[2];
  uint8_t req_buff_size = sizeof(req_buff);
  mfrc522.PCD_StopCrypto1();
  mfrc522.PICC_HaltA();                                 // TODO: Delay after this functioncall necessary?
  mfrc522.PICC_WakeupA(req_buff,&req_buff_size);        // TODO: Delay after this functioncall necessary?
  uint8_t s = mfrc522.PICC_Select(&(mfrc522.uid), 0);
  if(mfrc522.GetStatusCodeName((MFRC522::StatusCode)s) == F("Timeout in communication.")) return -1;
  return *(uint32_t*) mfrc522.uid.uidByte;
}