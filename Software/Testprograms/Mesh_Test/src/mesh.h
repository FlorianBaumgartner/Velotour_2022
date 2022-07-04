#ifndef MESH_H
#define MESH_H

#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>

class Mesh
{
  public:
    Mesh();
    void begin(int id = 0);
    void broadcast(const String &message);

    void setLedState(bool state);
    bool getLedState(void);    

  private:
    static void update(void);
    static void sentCallback(const uint8_t *macAddr, esp_now_send_status_t status);
    static void receiveCallback(const uint8_t *macAddr, const uint8_t *data, int dataLen);

    int deviceId = 0;
};

#endif