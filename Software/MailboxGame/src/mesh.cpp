#include "mesh.h"
#include "console.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <ArduinoJson.h>

#define log   DISABLE_MODULE_LEVEL


static Mesh* meshPtr = nullptr;

Mesh::Mesh()
{
  meshPtr = this;
}

void Mesh::begin(int id)
{
  deviceId = id;
  randomReference = esp_random() % 0xFFFF;  // Use True RNG to generate unique lifecycle reference IDs

  if(initialized)
  {
    end();
    initialized = false;
    delay(1500 / TASK_MESH_FREQ);           // Make shure, that the update task is not running anymore
  }
  initialized = true;
  WiFi.mode(WIFI_STA);                      // Set device in STA mode to begin with
  WiFi.setTxPower(WIFI_POWER_19_5dBm);      // Set Max RF-Power
  WiFi.disconnect();
  if (esp_now_init() == ESP_OK)             // Startup ESP Now
  {
    console.ok.println("[MESH] ESPNow Init Success");
    esp_now_register_recv_cb(receiveCallback);
    esp_now_register_send_cb(sentCallback);
  }
  else
  {
    console.error.println("[MESH] ESPNow Init Failed");
    restart();
  }

  xTaskCreate(update, "task_mesh", 2048, this, 1, NULL);
}

void Mesh::end(void)
{
  if(initialized)
  {
    esp_now_deinit();
  }
  initialized = false;
}

void Mesh::restart(bool immediate)
{
  end();
  if(!immediate)
  {
    delay(RESTART_DELAY);
  }
  ESP.restart();
}

void Mesh::getNodeIds(int8_t* ids, uint8_t maxLen)
{
  uint8_t count = 0;
  for(int i = 0; i < MAX_NODES_NUM; i++)
  {
    if(activeNodes[i] && count < maxLen)
    {
      ids[count++] = message[i].id;
    }
  }
}

void Mesh::update(void *pvParameter)
{
  Mesh* ref = (Mesh*)pvParameter;
  while(ref->initialized)
  {
    TickType_t task_last_tick = xTaskGetTickCount();

    if(ref->deviceId < MAX_NODES_NUM)
    {
      ref->message[ref->deviceId].id = ref->deviceId;
      ref->message[ref->deviceId].timestamp = millis();
      ref->message[ref->deviceId].lifetimeRef = ref->randomReference;
      ref->message[ref->deviceId].origin = true;
      ref->nodeTimeOffset[ref->deviceId] = 0;
    }
    else
    {
      console.warning.printf("[MESH] Own Node ID (%d) is out of index (Max. %d)\n", ref->deviceId, MAX_NODES_NUM);
    }

    Message messagesToSend[MAX_NODES_NUM];
    int messagesToSendCount = 0; 
    for(int i = 0; i < MAX_NODES_NUM; i++)
    {
      ref->activeNodes[i] = false;              // Default is every node marked as inactive
      if(ref->message[i].id != -1)              // Check if node ID already exists in local buffer
      {
        int32_t timeDiff = millis() - (ref->message[i].timestamp + ref->nodeTimeOffset[i]);
        if(timeDiff < NETWORK_TIMEOUT)
        {
          ref->activeNodes[i] = true;           // Node is active since data has recently been received
          memcpy(&messagesToSend[messagesToSendCount], &ref->message[i], sizeof(Message));
          messagesToSend[messagesToSendCount].origin = (i == ref->deviceId);  // Clear origin flag for relayed messages
          messagesToSendCount++;
        }
        console.log.printf("[MESH] System Time: %d, Node %d (LifeTimeRef: %04X): ", millis(), ref->message[i].id, ref->message[i].lifetimeRef);
        if(i == ref->deviceId)                  // Check if own node id is present
        {
          console.log.printf("Own Node\n");
        }
        else
        {
          console.log.printf("Node Timestamp = %d -> nodeTimeOffset = %d -> timeDiff = %d\n", ref->message[i].timestamp, ref->nodeTimeOffset[i], timeDiff);
        }   
      }
      ref->origin[i] = ref->message[i].origin;  // Save origin state for IPA access
      if(ref->activeNodes[i])                   // Only clear flag if data of specific node has been received
      {
        ref->message[i].origin = false;         // Clear flag, the collect all messages over one periode and watch for origin messages
      }
    }
    ref->nodeCount = messagesToSendCount;

    if(ref->nodeCount > 0)
    {
      console.log.print("[MESH] Send node IDs: [");
      for(int i = 0; i < ref->nodeCount; i++)
      {
        console.log.printf("%d", messagesToSend[i].id);
        if(i < ref->nodeCount - 1) console.log.print(", ");
      }
      console.log.println("]");

      static uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
      static esp_now_peer_info_t peerInfo = {};
      memcpy(&peerInfo.peer_addr, broadcastAddress, 6);
      if (!esp_now_is_peer_exist(broadcastAddress))
      {
        esp_now_add_peer(&peerInfo);
      }
      esp_now_send(broadcastAddress, (const uint8_t*)messagesToSend, ref->nodeCount * sizeof(Message));
    }
    else
    {
      console.warning.println("[MESH] No node data to send (not even own node data - this should no happen)");
    }

    vTaskDelayUntil(&task_last_tick, (const TickType_t) 1000 / TASK_MESH_FREQ);
  }
  vTaskDelete(NULL);
}

void Mesh::sentCallback(const uint8_t *macAddr, esp_now_send_status_t status)
{
  if(status != ESP_NOW_SEND_SUCCESS)
  {
    console.error.println("[MESH] Could not send data (ESP-NOW Error) -> Reset");
    delay(100);
    meshPtr->restart();
  }
}

void Mesh::receiveCallback(const uint8_t *macAddr, const uint8_t *data, int dataLen)
{
  static int noReplyCount = 0;
  int msgLen = min(ESP_NOW_MAX_DATA_LEN, dataLen);
  uint8_t buffer[ESP_NOW_MAX_DATA_LEN];
  memcpy(buffer, data, msgLen);
  int nodeCount = msgLen / sizeof(Message);
  bool receivedOwnId = false;
  console.log.printf("[MESH] Received Node Count: %d\n", nodeCount);
  for(int i = 0; i < nodeCount; i++)
  {
    Message* msg = (Message*) &buffer[i * sizeof(Message)];
    if(msg->id != meshPtr->deviceId)     // Look only for other nodes (exclude relayed personal data)
    {
      if(msg->id < MAX_NODES_NUM)
      {
        if(msg->timestamp > meshPtr->message[msg->id].timestamp || msg->lifetimeRef != meshPtr->message[msg->id].lifetimeRef)   // Check if the received message is fresh or if its already buffered
        {
          bool origin = meshPtr->message[msg->id].origin;
          meshPtr->nodeTimeOffset[msg->id] = millis() - msg->timestamp;
          memcpy(&meshPtr->message[msg->id], msg, sizeof(Message));
          meshPtr->message[msg->id].origin |= origin;     // Ignore relayed messages if message from source has been received
        }
      }
      else
      {
        console.warning.printf("[MESH] Received Node ID (%d) is out of index (Max. %d)\n", msg->id, MAX_NODES_NUM);
      }
    }
    else
    {
      receivedOwnId = true;
    }
  }
  if(receivedOwnId || nodeCount == 0)   // Only panic if other node data has received, but not own data has been relayed back
  {
    noReplyCount = 0;
  }
  else
  {
    noReplyCount++;
    if(noReplyCount > NO_REPLY_THRESHOLD)
    {
      console.warning.println("[MESH] Didn't receive own node data (is this node even transmitting?) -> Reset");
      delay(100);
      meshPtr->restart();
    }
  }
}
