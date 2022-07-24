#include "office.h"
#include "console.h"
#include "utils.h"
#include "freertos/task.h"
#include <ArduinoJson.h>


bool Office::begin(void)
{
  if(!loadMailboxFile("cards.json"))
  {
    console.error.println("[OFFICE] Could not read cards configuration file");
    return false;
  }
  if(!loadOfficeFile("office.json"))
  {
    console.error.println("[OFFICE] Could not read office configuration file");
    return false;
  }

  xTaskCreate(update, "task_office", 2048, this, 1, NULL);
  return true;
}


void Office::update(void* pvParameter)
{
  Office* ref = (Office*)pvParameter;
  uint32_t printTimer = 0;

  while(true)
  {
    TickType_t task_last_tick = xTaskGetTickCount();

    bool allCorrect = true;
    ref->returnPayload = 0x00000000;                // Divided into 2-Bit node status segments: 00 = ignored node, 01 = disconnected, 10 = wrong code, 11 = correct code
    for(int i = 0; i < MAX_NODES_NUM; i++)
    {
      if(ref->mailboxStatus[i] != MAILBOX_IGNORED)  // Only care about valid mailboxes, unlisted IDs are ignored
      {
        ref->mailboxStatus[i] = MAILBOX_DISCONNECTED;
        if(ref->mesh.getNodeState(i))               // Check if node is present in network
        {
          ref->mailboxStatus[i] = ref->mesh.getNodePayload(i) == -1? MAILBOX_CONNECTED : MAILBOX_ACTIVE;
        }

        bool nodeCorrect = ref->mesh.getNodePayload(i) == ref->mailboxCompareCode[i];   // Check if received card number matches with reference
        if(ref->mailboxStatus[i] == MAILBOX_ACTIVE)
        {
          ref->returnPayload |= (nodeCorrect? 0x03 : 0x02) << i * 2;
        }
        else
        {
          ref->returnPayload |= 0x01 << i * 2;                      // Shows that node has not yet joined the network
        }
        allCorrect &= nodeCorrect;
      }
    }
    ref->returnPayload |= allCorrect? 0x80000000 : 0x00000000;      // MSB is set in payload if all cards are correct -> game finished
    ref->mesh.setPayload(ref->returnPayload);                       // Send game info back to all mailboxes

    switch(ref->officeState)
    {
      case OFFICE_READY:
        if(allCorrect)
        {
          ref->hmi.setResultIndicator(ref->solution);
          ref->hmi.playSound(Hmi::BUZZER_SUCCESS);
          ref->officeState = OFFICE_WIN;
        }
        break;
      case OFFICE_WIN:
          // TODO: Wait on button to turn off or timeout
        break;
      case OFFICE_POWERDOWN:
          // TODO: Start Shutdown process
        break;
    }

    if((millis() - printTimer > PRINT_INTERVAL) && millis() > 10000)
    {
      printTimer = millis();
      ref->printInfo();
    }
    
    vTaskDelayUntil(&task_last_tick, (const TickType_t) 1000 / TASK_OFFICE_FREQ);
  }
  vTaskDelete(NULL);
}

void Office::printInfo(void)
{ 
  console.log.print("[OFFICE]\n[OFFICE] **********");
  for(int i = 1; i < MAX_NODES_NUM; i++)
  {
    console.log.print("***********");
  }
  for(int i = 1; i < MAX_NODES_NUM; i++)
  {
    if(i == 1) console.log.print("\n[OFFICE] Node:    |");
    console.log.printf(" %8d |", i);
  }
  for(int i = 1; i < MAX_NODES_NUM; i++)
  {
    const char* status;
    switch(mailboxStatus[i])
    {
      case MAILBOX_IGNORED:       status = "IGNORED"; break;
      case MAILBOX_DISCONNECTED:  status = "DISCONN"; break;
      case MAILBOX_CONNECTED:     status = "CONNECT"; break;
      case MAILBOX_ACTIVE:        status = " ACTIVE"; break;
      default:                    status = "UNKNOWN"; break;
    }    
    if(i == 1) console.log.print("\n[OFFICE] Status:  |");
    console.log.printf(" %8s |", status);
  }
  for(int i = 1; i < MAX_NODES_NUM; i++)
  {
    if(i == 1) console.log.print("\n[OFFICE] Compare: |");
    console.log.printf(" %08X |", mailboxCompareCode[i]);
  }
  for(int i = 1; i < MAX_NODES_NUM; i++)
  {
    if(i == 1) console.log.print("\n[OFFICE] Payload: |");
    if(mesh.getNodePayload(i) != -1)
    {
      console.log.printf(" %08X |", mesh.getNodePayload(i));
    }
    else console.log.print(" -------- |");
  }
  console.log.print("\n[OFFICE] **********");
  for(int i = 1; i < MAX_NODES_NUM; i++)
  {
    console.log.print("***********");
  }
  console.log.println();
}

bool Office::loadMailboxFile(const char* path)
{
  File file = fatfs.open(path);

  if(!file)
  {
    console.error.println("[OFFICE] Open file failed");
    return false;
  }

  StaticJsonDocument<1024> doc;
  DeserializationError error = deserializeJson(doc, file);
  if(error)
  {
    file.close();
    console.error.printf("[OFFICE] Failed to read file, using default configuration: %d\n", error);
    return false;
  }
  JsonObject root = doc.as<JsonObject>();

  for(int i = 0; i < MAX_NODES_NUM; i++)
  {
    mailboxStatus[i] = MAILBOX_IGNORED;
    mailboxCompareCode[i] = -1;
  }
  for (JsonPair kv : root)
  {
    int i = strtol(kv.key().c_str(), NULL, 0);
    mailboxStatus[i] = MAILBOX_DISCONNECTED;
    mailboxCompareCode[i] = strtoul(kv.value().as<const char*>(), NULL, 0);
  }
  file.close();
  console.ok.println("[OFFICE] Mailbox File loaded and parsed successfully");
  return true;
}

bool Office::loadOfficeFile(const char* path)
{
  File file = fatfs.open(path);

  if(!file)
  {
    console.error.println("[OFFICE] Open file failed");
    return false;
  }

  StaticJsonDocument<1024> doc;
  DeserializationError error = deserializeJson(doc, file);
  if(error)
  {
    file.close();
    console.error.printf("[OFFICE] Failed to read file, using default configuration: %d\n", error);
    return false;
  }
  JsonObject root = doc.as<JsonObject>();
  if(!doc.containsKey("solution"))
  {
    console.error.println("[OFFICE] JSON File does not contain key ""solution""");
    solution = Hmi::LED_RESULT_NONE;
    return false;
  }
  switch(doc["solution"].as<const char*>()[0])
  {
    case 'a': case 'A': solution = Hmi::LED_RESULT_A; break;
    case 'b': case 'B': solution = Hmi::LED_RESULT_B; break;
    case 'c': case 'C': solution = Hmi::LED_RESULT_C; break;
    default:  solution = Hmi::LED_RESULT_NONE; break;
  }
  file.close();
  console.log.printf("[OFFICE] Correct Solution: %c\n", 'A' + solution - 1);
  console.ok.println("[OFFICE] Office Configuration loaded and parsed successfully");
  return true;
}