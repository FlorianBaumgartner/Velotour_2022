#include "mailbox.h"
#include "console.h"

#include "freertos/task.h"


bool Mailbox::begin(void)
{
  i2cBus.begin(pinNfcSda, pinNfcScl, 400000U); 
  mfrc522.PCD_Init();		        // Init MFRC522

  xTaskCreate(update, "task_mailbox", 2048, this, 1, NULL);
  return true;
}


void Mailbox::update(void* pvParameter)
{
  Mailbox* ref = (Mailbox*)pvParameter;

  while(true)
  {
    TickType_t task_last_tick = xTaskGetTickCount();


    vTaskDelayUntil(&task_last_tick, (const TickType_t) 1000 / TASK_MAILBOX_FREQ);
  }
  vTaskDelete(NULL);
}

uint32_t Mailbox::readCardData(void)
{
  uint8_t req_buff[2];
  uint8_t req_buff_size = sizeof(req_buff);
  mfrc522.PCD_StopCrypto1();
  mfrc522.PICC_HaltA();                                 // TODO: Delay after this functioncall necessary?
  mfrc522.PICC_WakeupA(req_buff,&req_buff_size);        // TODO: Delay after this functioncall necessary?
  uint8_t s = mfrc522.PICC_Select(&(mfrc522.uid), 0);
  if(mfrc522.GetStatusCodeName((MFRC522::StatusCode)s) == F("Timeout in communication.")) return -1;
  return *(uint32_t*) mfrc522.uid.uidByte;
}