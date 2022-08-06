/******************************************************************************
* file    office.cpp
*******************************************************************************
* brief   Main class for handling all office related tasks
*******************************************************************************
* author  Florian Baumgartner
* version 1.0
* date    2022-08-02
*******************************************************************************
* MIT License
*
* Copyright (c) 2022 Crelin - Florian Baumgartner
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell          
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
******************************************************************************/

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
  console.ok.println("[OFFICE] Initialization successfull!");
  return true;
}


void Office::update(void* pvParameter)
{
  Office* ref = (Office*)pvParameter;
  uint32_t printTimer = 0;
  uint32_t stateTimer = -1;
  State state = STATE_READY;
  bool allCorrect = false;

  while(true)
  {
    TickType_t task_last_tick = xTaskGetTickCount();

    bool check = true;
    ref->returnPayload = 0x00000000;                // Divided into 2-Bit node status segments: 00 = ignored node, 01 = disconnected, 10 = wrong code, 11 = correct code
    for(int i = 0; i < MAX_NODES_NUM; i++)
    {
      if(ref->mailboxStatus[i] != Hmi::NODE_IGNORED)  // Only care about valid mailboxes, unlisted IDs are ignored
      {
        ref->mailboxStatus[i] = Hmi::NODE_DISCONNECTED;
        if(ref->mesh.getNodeState(i))               // Check if node is present in network
        {
          ref->mailboxStatus[i] = ref->mesh.getNodePayload(i) == -1? Hmi::NODE_CONNECTED : Hmi::NODE_ACTIVE;
        }

        bool nodeCorrect = ref->mesh.getNodePayload(i) == ref->mailboxCompareCode[i];   // Check if received card number matches with reference
        if(ref->mailboxStatus[i] == Hmi::NODE_ACTIVE)
        {
          ref->returnPayload |= (nodeCorrect? 0x03 : 0x02) << i * 2;
        }
        else
        {
          ref->returnPayload |= 0x01 << i * 2;                      // Shows that node has not yet joined the network
        }
        check &= nodeCorrect;
      }
      if(i == 0 && SHOW_OWN_NODE_STATUS)
      {
        ref->hmi.setNodeStatus(0, Hmi::NODE_ACTIVE);                // Light up LED 0 to show own ID (office)
      }
      else
      {
        ref->hmi.setNodeStatus(i, ref->mailboxStatus[i]);           // Update Network status LEDs
      }
    }
    allCorrect |= check;                                            // Once all cards are correct, keep state even if removed
    ref->returnPayload |= allCorrect? 0x80000000 : 0x00000000;      // MSB is set in payload if all cards are correct -> game finished
    ref->mesh.setPayload(ref->returnPayload);                       // Send game info back to all mailboxes

    if((millis() - printTimer > PRINT_INTERVAL) && millis() > 10000 && console && state == STATE_READY)
    {
      printTimer = millis();
      ref->printInfo();
    }

    switch(state)
    {
      case STATE_READY:
        if(ref->hmi.getMode() == Hmi::LED_MODE_NONE)
        {
          ref->hmi.setMode(Hmi::LED_MODE_NODE_STATUS);
        }
        if(allCorrect)
        {
          ref->hmi.setResultIndicator(ref->solution);
          ref->hmi.setMode(Hmi::LED_MODE_SUCCESS);
          ref->hmi.playSound(Hmi::BUZZER_SUCCESS);
          console.log.println("[OFFICE] All cards are correct -> Go to win state");
          state = STATE_WIN;
          stateTimer = millis();
        }
        break;
      case STATE_WIN:
        if(ref->sys.getButtonState() || (millis() - stateTimer > SUCCESS_STATE_TIMEOUT * 1000))
        {
          ref->hmi.setResultIndicator(Hmi::LED_RESULT_NONE);
          ref->hmi.setStatusIndicator(Hmi::LED_STATUS_OFF);
          ref->hmi.setMode(Hmi::LED_MODE_POWER_OFF);
          ref->hmi.playSound(Hmi::BUZZER_POWER_OFF);
          console.log.println("[OFFICE] Enter Powerdown sequence (command received or timeout occured)");
          state = STATE_POWERDOWN;
        }
        break;
      case STATE_POWERDOWN:
        if(ref->hmi.getMode() == Hmi::LED_MODE_NONE && !utils.isConnected())
        {
          ref->mesh.end();
          ref->hmi.end();
          ref->sys.powerDown();
        }
        break;
      default:
        console.error.printf("[OFFICE] Undefined State: %d\n", state);
        break;
    }
    
    vTaskDelayUntil(&task_last_tick, (const TickType_t) 1000 / TASK_OFFICE_FREQ);
  }
  vTaskDelete(NULL);
}

void Office::printInfo(bool forceUpdate)
{ 
  bool update = forceUpdate;
  static Hmi::NodeStatus mailboxStatusOld[MAX_NODES_NUM];
  static uint32_t payloadOld[MAX_NODES_NUM];
  for(int i = 0; i < MAX_NODES_NUM; i++)
  {
    update |= (mailboxStatus[i] != mailboxStatusOld[i]);
    update |= (mesh.getNodePayload(i) != payloadOld[i]);
    mailboxStatusOld[i] = mailboxStatus[i];
    payloadOld[i] = mesh.getNodePayload(i);
  }
  if(update)
  {
    console.print("\n[OFFICE] ");
    console.printTimestamp();
    console.print("\n[OFFICE] **********");
    for(int i = 1; i < MAX_NODES_NUM; i++)
    {
      console.print("***********");
    }
    for(int i = 1; i < MAX_NODES_NUM; i++)
    {
      if(i == 1) console.print("\n[OFFICE] Node:    |");
      console.printf(" %8d |", i);
    }
    for(int i = 1; i < MAX_NODES_NUM; i++)
    {
      if(i == 1) console.print("\n[OFFICE] Status:  |");
      switch(mailboxStatus[i])
      {
        case Hmi::NODE_IGNORED:       console.print("  IGNORED");               break;
        case Hmi::NODE_DISCONNECTED:  console[COLOR_RED].print("  DISCONN");    break;
        case Hmi::NODE_CONNECTED:     console[COLOR_YELLOW].print("  CONNECT"); break;
        case Hmi::NODE_ACTIVE:        console[COLOR_GREEN].print("   ACTIVE");  break;
        default:                      console[COLOR_RED].print("  UNKNOWN");    break;
      } 
      console.print(" |");
    }
    for(int i = 1; i < MAX_NODES_NUM; i++)
    {
      if(i == 1) console.print("\n[OFFICE] Compare: |");
      console.printf(" %08X |", mailboxCompareCode[i]);
    }
    for(int i = 1; i < MAX_NODES_NUM; i++)
    {
      if(i == 1) console.print("\n[OFFICE] Payload: |");
      if(mesh.getNodePayload(i) != -1)
      {
        console[mesh.getNodePayload(i) == mailboxCompareCode[i]? COLOR_GREEN : COLOR_RED].printf(" %08X", mesh.getNodePayload(i));
        console.print(" |");
      }
      else console.print(" -------- |");
    }
    console.print("\n[OFFICE] **********");
    for(int i = 1; i < MAX_NODES_NUM; i++)
    {
      console.print("***********");
    }
    console.println();
  }
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
    mailboxStatus[i] = Hmi::NODE_IGNORED;
    mailboxCompareCode[i] = -1;
  }
  for (JsonPair kv : root)
  {
    int i = strtol(kv.key().c_str(), NULL, 0);
    mailboxStatus[i] = Hmi::NODE_DISCONNECTED;
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
