#include "office.h"
#include "console.h"
#include "utils.h"
#include "freertos/task.h"
#include <ArduinoJson.h>


bool Office::begin(void)
{
  loadMailboxFile("office.json");

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

    for(int i = 0; i < MAX_NODES_NUM; i++)
    {
      if(ref->mailboxStatus[i] != MAILBOX_IGNORED)  // Update connection status of mailboxes
      {
        ref->mailboxStatus[i] = MAILBOX_DISCONNECTED;
        if(ref->mesh.getNodeState(i))       // Check if node is present in network
        {
          ref->mailboxStatus[i] = ref->mesh.getNodePayload(i) == -1? MAILBOX_CONNECTED : MAILBOX_ACTIVE;
        }
      }

    }


    if(millis() - printTimer > PRINT_INTERVAL)
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
  console.ok.println("[OFFICE] Configuration loaded and parsed successfully");
  return true;
}