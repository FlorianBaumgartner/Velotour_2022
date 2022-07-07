#include "mesh.h"
#include "utils.h"                          // Use for USB Serial
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <ArduinoJson.h>

#define TASK_MESH_FREQ          1           // Frequency [Hz]

volatile static bool demoLed;
static Message* messagePtr = nullptr;
static int32_t* nodeTimeOffsetPtr = nullptr;
static int8_t* deviceIdPtr = nullptr;


Mesh::Mesh()
{
  messagePtr = message;
  nodeTimeOffsetPtr = nodeTimeOffset;
  deviceIdPtr = &deviceId;
}

void Mesh::begin(int id)
{
  deviceId = id;
  randomReference = esp_random() % 0xFFFF;  // Use True RNG to generate unique lifecycle reference IDs

  WiFi.mode(WIFI_STA);                      // Set device in STA mode to begin with
  WiFi.disconnect();
  if (esp_now_init() == ESP_OK)             // Startup ESP Now
  {
    Serial.println("ESPNow Init Success");
    esp_now_register_recv_cb(receiveCallback);
    esp_now_register_send_cb(sentCallback);
  }
  else
  {
    Serial.println("ESPNow Init Failed");
    delay(3000);
    ESP.restart();
  }

  xTaskCreate(update, "task_mesh", 2048, this, 1, NULL);
}

void Mesh::broadcast(const String &message)
{
  uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  esp_now_peer_info_t peerInfo = {};
  memcpy(&peerInfo.peer_addr, broadcastAddress, 6);
  if (!esp_now_is_peer_exist(broadcastAddress))
  {
    esp_now_add_peer(&peerInfo);
  }
  esp_err_t result = esp_now_send(broadcastAddress, (const uint8_t *)message.c_str(), message.length());
  if (result == ESP_OK)
  {
    Serial.println("Broadcast message success");
  }
  else if (result == ESP_ERR_ESPNOW_NOT_INIT)
  {
    Serial.println("ESPNOW not Init.");
  }
  else if (result == ESP_ERR_ESPNOW_ARG)
  {
    Serial.println("Invalid Argument");
  }
  else if (result == ESP_ERR_ESPNOW_INTERNAL)
  {
    Serial.println("Internal Error");
  }
  else if (result == ESP_ERR_ESPNOW_NO_MEM)
  {
    Serial.println("ESP_ERR_ESPNOW_NO_MEM");
  }
  else if (result == ESP_ERR_ESPNOW_NOT_FOUND)
  {
    Serial.println("Peer not found.");
  }
  else
  {
    Serial.println("Unknown error");
  }
}

void Mesh::update(void *pvParameter)
{
  while(true)
  {
    Mesh* ref = (Mesh*)pvParameter;
    TickType_t task_last_tick = xTaskGetTickCount();

    if(ref->deviceId < MAX_NODES_NUM)
    {
      ref->message[ref->deviceId].id = ref->deviceId;
      ref->message[ref->deviceId].timestamp = millis();
      ref->message[ref->deviceId].payload = 0x12345678;     // TODO: Change payload
      ref->message[ref->deviceId].lifetimeRef = ref->randomReference;
      ref->nodeTimeOffset[ref->deviceId] = 0;
    }
    else
    {
      Serial.printf("Own Node ID (%d) is out of index (Max. %d)\n", ref->deviceId, MAX_NODES_NUM);
    }

    Message messagesToSend[MAX_NODES_NUM];
    int messagesToSendCount = 0; 
    for(int i = 0; i < MAX_NODES_NUM; i++)
    {
      if(ref->message[i].id != -1)    // Check if node ID aleady exists in local buffer
      {
        int32_t timeDiff = millis() - (ref->message[i].timestamp + ref->nodeTimeOffset[i]);
        if(timeDiff < NETWORK_TIMEOUT)
        {
          memcpy(&messagesToSend[messagesToSendCount++], &ref->message[i], sizeof(Message));
        }
        Serial.printf("System Time: %d, Node %d (LifeTimeRef: %04X): ", millis(), ref->message[i].id, ref->message[i].lifetimeRef);
        if(i == ref->deviceId)        // Check if own node id is present
        {
          Serial.printf("Own Node\n");
        }
        else
        {
          Serial.printf("Node Timestamp = %d -> nodeTimeOffset = %d -> timeDiff = %d\n", ref->message[i].timestamp, ref->nodeTimeOffset[i], timeDiff);
        }   
      }
    }
    if(messagesToSendCount > 0)
    {
      Serial.print("Send node IDs: [");
      for(int i = 0; i < messagesToSendCount; i++)
      {
        Serial.printf("%d", messagesToSend[i].id);
        if(i < messagesToSendCount - 1) Serial.print(", ");
      }
      Serial.println("]");

      uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
      esp_now_peer_info_t peerInfo = {};
      memcpy(&peerInfo.peer_addr, broadcastAddress, 6);
      if (!esp_now_is_peer_exist(broadcastAddress))
      {
        esp_now_add_peer(&peerInfo);
      }
      esp_err_t result = esp_now_send(broadcastAddress, (const uint8_t*)messagesToSend, messagesToSendCount * sizeof(Message));
      //Serial.println(result == ESP_OK? "Messages sent successfully": "Error occured while sending Messages");
    }
    else
    {
      Serial.println("No node data to send (not even own node data - this should no happen)");
    }

    //ref->nodeTimeOffset[1] = nodeTimeOffsetPtr[1];
    //Serial.printf("Addr should be %08X, but is %08X\n", ref->nodeTimeOffset[1], nodeTimeOffsetPtr[1]);

    vTaskDelayUntil(&task_last_tick, (const TickType_t) 1000 / TASK_MESH_FREQ);
  }
}

void Mesh::setLedState(bool state)
{
  demoLed = state;
}

bool Mesh::getLedState(void)
{
  return demoLed;
}

void Mesh::sentCallback(const uint8_t *macAddr, esp_now_send_status_t status)
{
  if(status != ESP_NOW_SEND_SUCCESS)
  {
    Serial.println("Sending data failed!");
  }
}

void Mesh::receiveCallback(const uint8_t *macAddr, const uint8_t *data, int dataLen)
{
  int msgLen = min(ESP_NOW_MAX_DATA_LEN, dataLen);
  uint8_t buffer[ESP_NOW_MAX_DATA_LEN];
  memcpy(buffer, data, msgLen);
  int nodeCount = msgLen / sizeof(Message);
  Serial.printf("Received Node Count: %d\n", nodeCount);
  for(int i = 0; i < nodeCount; i++)
  {
    Message* msg = (Message*) &buffer[i * sizeof(Message)];
    if(msg->id != *deviceIdPtr)     // Look only for other nodes (exlude repeated personal data)
    {
      if(msg->id < MAX_NODES_NUM)
      {
        if(msg->timestamp > messagePtr[msg->id].timestamp || msg->lifetimeRef != messagePtr[msg->id].lifetimeRef)   // Check if the received message is fresh or if its already buffered
        {
          nodeTimeOffsetPtr[msg->id] = millis() - msg->timestamp;
          memcpy(&messagePtr[msg->id], msg, sizeof(Message));
        }
      }
      else
      {
        Serial.printf("Received Node ID (%d) is out of index (Max. %d)\n", msg->id, MAX_NODES_NUM);
      }
    }
  }
}
