#ifndef MESH_H
#define MESH_H

#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>

#define MAX_NODES_NUM             10          // [#]
#define NETWORK_TIMEOUT           5000        // [ms]

#pragma pack(push, 1)
struct Message
{
  int8_t id = -1;
  uint16_t lifetimeRef = -1;
  int32_t timestamp = 0;
  int32_t payload = -1;
};
#pragma pack(pop)

class Mesh
{
  public:
    Mesh();
    void begin(int id = 0);
    void broadcast(const String &message);
    

    void setLedState(bool state);
    bool getLedState(void);    

  private:
    static void update(void *pvParameter);
    static void sentCallback(const uint8_t *macAddr, esp_now_send_status_t status);
    static void receiveCallback(const uint8_t *macAddr, const uint8_t *data, int dataLen);

    uint16_t randomReference = 0;
    int8_t deviceId = 0;
    Message message[MAX_NODES_NUM];               // Must be static, due to callback functions are static (C-Sytle)
    int32_t nodeTimeOffset[MAX_NODES_NUM];        // Offset of system timestamp and node timestamp (positive = system is ahead in time, negative = node is ahead in time)
};

#endif