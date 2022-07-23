#ifndef MESH_H
#define MESH_H

#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>

#define TASK_MESH_FREQ            1           // Frequency [Hz]
#define MAX_NODES_NUM             10          // [#]
#define NETWORK_TIMEOUT           5000        // [ms]
#define RESTART_DELAY             10          // [ms]
#define NO_REPLY_THRESHOLD        5           // [#]

#pragma pack(push, 1)
struct Message
{
  int8_t id = -1;
  int8_t origin = 0;           // Identifies if this message is sent by origin source or has been relayed
  uint16_t lifetimeRef = 0;
  int32_t timestamp = 0;
  int32_t payload = -1;
};
#pragma pack(pop)

class Mesh
{
  public:
    Mesh();
    void begin(int id = 0);
    void end(void);
    void restart(bool immediate = false);
    inline void setPayload(uint32_t data) {message[deviceId].payload = data;};
    inline int8_t getPersonalId(void) {return deviceId;};
    inline uint32_t getNodePayload(int8_t id) {return message[id % MAX_NODES_NUM].payload;};
    inline bool getNodeOrigin(int8_t id) {return origin[id % MAX_NODES_NUM] != 0;};
    inline uint8_t getNodeCount(void) {return nodeCount;};
    void getNodeIds(int8_t* ids, uint8_t maxLen = MAX_NODES_NUM);
    
  private:
    static void update(void *pvParameter);
    static void sentCallback(const uint8_t *macAddr, esp_now_send_status_t status);
    static void receiveCallback(const uint8_t *macAddr, const uint8_t *data, int dataLen);

    volatile bool initialized = false;
    uint16_t randomReference = 0;
    int8_t deviceId = 0;
    uint8_t nodeCount = 0;
    int8_t origin[MAX_NODES_NUM];                 // Indicates which node is directly connected (1 = direct, 0 = relayed)
    Message message[MAX_NODES_NUM];               // Must be static, due to callback functions are static (C-Sytle)
    int32_t nodeTimeOffset[MAX_NODES_NUM];        // Offset of system timestamp and node timestamp (positive = system is ahead in time, negative = node is ahead in time)
    bool activeNodes[MAX_NODES_NUM];
};

#endif