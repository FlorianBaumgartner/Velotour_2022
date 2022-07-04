#include <Arduino.h>
#include "utils.h"
#include <TFT_eSPI.h>

#include "configuration.h"
#include <painlessMesh.h>

#define   MESH_PREFIX     "ESPMESH"
#define   MESH_PASSWORD   "password"
#define   MESH_PORT       5555

#define USER_BTN          0
#define LED               33
#define BLINK_INTERVAL    500

Utils utils;
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite framebuf = TFT_eSprite(&tft);


Scheduler userScheduler; // to control your personal task
painlessMesh  mesh;

void sendMessage(); // Prototype so PlatformIO doesn't complain
Task taskSendMessage(TASK_SECOND * 1 , TASK_FOREVER, &sendMessage);

void sendMessage()
{
  String msg = "Hi from node ";
  msg += mesh.getNodeId();
  msg += " (Serial Number: " + String(utils.getSerialNumber()) + ")";
  mesh.sendBroadcast(msg);
  taskSendMessage.setInterval(TASK_SECOND * 1);
}

// Needed for painless library
void receivedCallback( uint32_t from, String &msg )
{
  Serial.printf("startHere: Received from %u msg=%s\n", from, msg.c_str());
}

void newConnectionCallback(uint32_t nodeId)
{
    Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
}

void changedConnectionCallback()
{
  Serial.printf("Changed connections\n");
}

void nodeTimeAdjustedCallback(int32_t offset)
{
    Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(),offset);
}

void setup()
{
  pinMode(LED, OUTPUT);
  pinMode(USER_BTN, INPUT_PULLUP);
  utils.begin("TEST");

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  framebuf.createSprite(TFT_WIDTH, TFT_HEIGHT);
  framebuf.fillSprite(TFT_BLACK);
  framebuf.pushSprite(0, 0);
  digitalWrite(LED, 1);  // Backlight on

  mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); // all types on
  //mesh.setDebugMsgTypes( ERROR | STARTUP);  // set before init() so that you can see startup messages

  mesh.init(MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT, utils.getSerialNumber()[0] == '0'? WIFI_AP : WIFI_STA);
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

  userScheduler.addTask(taskSendMessage);
  taskSendMessage.enable();
}

void loop()
{
  static bool btn = true;
  if(btn && !digitalRead(USER_BTN))
  {
    Serial.println("Start Mesh");

  }
  btn = digitalRead(USER_BTN);

  static int t = 0;
  if(millis() - t > 1000)
  {
    t = millis();
    Serial.printf("Time: %d\n", millis());

    framebuf.fillSprite(TFT_BLACK);
    framebuf.setCursor(0, 0, 2);
    framebuf.setTextColor(TFT_WHITE);
    framebuf.printf("Serial Number: %s\n", utils.getSerialNumber());
    framebuf.printf("Time: %d\n", millis());
    framebuf.pushSprite(0, 0);
  }

  //digitalWrite(LED, (millis() / BLINK_INTERVAL) & 1);
  mesh.update();
  delay(1);
}