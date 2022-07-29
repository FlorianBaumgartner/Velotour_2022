#include "mailbox.h"
#include "console.h"
#include "freertos/task.h"

#define DISABLE_NFC


bool Mailbox::begin(void)
{
  #ifndef DISABLE_NFC
    initializeNfc();
  #endif

  xTaskCreate(update, "task_mailbox", 2048, this, 1, NULL);
  console.ok.println("[MAILBOX] Initialization successfull!");
  return true;
}

void Mailbox::update(void* pvParameter)
{
  Mailbox* ref = (Mailbox*)pvParameter;
  uint32_t cardTimeout = millis();
  uint32_t stateTimer = -1;
  State state = STATE_READY;
  console.log.println("[MMAILBOX] Waiting on card to be inserted...");

  while(true)
  {
    TickType_t task_last_tick = xTaskGetTickCount();

    uint8_t nodeStatus = (ref->mesh.getNodePayload(0) >> (ref->mesh.getPersonalId() * 2)) & 0x03;
    bool allCorrect = (ref->mesh.getNodePayload(0) & 0x80000000) && ref->mesh.getNodeState(0);
    uint32_t card = ref->readCardData();
    ref->mesh.setPayload(card);

    switch(state)
    {
      case STATE_READY:
        if(card != -1)
        {
          ref->hmi.setMode(Hmi::LED_MODE_CARD_INSERTED);
          ref->hmi.playSound(Hmi::BUZZER_CARD_INSERTED);
          console.log.printf("[MAILBOX] Card inserted with code: %08X\n", card);
          state = STATE_ACTIVE;
        }
        if(millis() - cardTimeout > NO_CARD_TIMEOUT * 1000)
        {
          ref->hmi.setMode(Hmi::LED_MODE_POWER_OFF);
          ref->hmi.playSound(Hmi::BUZZER_POWER_OFF);
          console.warning.printf("[MAILBOX] Timout occured: No card has been inserted within the last %d s\n", NO_CARD_TIMEOUT);
          state = STATE_POWERDOWN;
        }
        break;
      case STATE_ACTIVE:
        if(card == -1)
        {
          ref->hmi.playSound(Hmi::BUZZER_CARD_REMOVED);
          state = STATE_READY;
          console.log.println("[MAILBOX] Card removed");
          cardTimeout = millis();
        }
        if(allCorrect)
        {
          ref->hmi.playSound(Hmi::BUZZER_SUCCESS);
          state = STATE_WIN;
          console.log.println("[MAILBOX] All cards are correct -> Go to win state");
          stateTimer = millis();
        }
        break;
      case STATE_WIN:
        if(!ref->mesh.getNodeState(0) || (millis() - stateTimer > WIN_STATE_TIMEOUT * 1000))
        {
          ref->hmi.setMode(Hmi::LED_MODE_POWER_OFF);
          ref->hmi.playSound(Hmi::BUZZER_POWER_OFF);
          console.log.println("[MAILBOX] Enter Powerdown sequence (command received or timeout occured)");
          state = STATE_POWERDOWN;
        }
        break;
      case STATE_POWERDOWN:
        if(ref->hmi.getMode() == Hmi::LED_MODE_OFF)
        {
          ref->sys.powerDown();
        }
        break;
      default:
        console.error.printf("[MAILBOX] Undefined State: %d\n", state);
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
  if(!nfcInitialized) return -1;
  uint8_t req_buff[2];
  uint8_t req_buff_size = sizeof(req_buff);
  mfrc522.PCD_StopCrypto1();
  mfrc522.PICC_HaltA();                                 // TODO: Delay after this functioncall necessary?
  mfrc522.PICC_WakeupA(req_buff, &req_buff_size);       // TODO: Delay after this functioncall necessary?
  uint8_t s = mfrc522.PICC_Select(&(mfrc522.uid), 0);
  if(mfrc522.GetStatusCodeName((MFRC522::StatusCode)s) == F("Timeout in communication.")) return -1;
  return *(uint32_t*) mfrc522.uid.uidByte;
}