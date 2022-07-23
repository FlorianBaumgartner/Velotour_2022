#include "office.h"
#include "console.h"

#include "freertos/task.h"


bool Office::begin(void)
{
  
  xTaskCreate(update, "task_office", 2048, this, 1, NULL);
  return true;
}


void Office::update(void* pvParameter)
{
  Office* ref = (Office*)pvParameter;

  while(true)
  {
    TickType_t task_last_tick = xTaskGetTickCount();


    vTaskDelayUntil(&task_last_tick, (const TickType_t) 1000 / TASK_OFFICE_FREQ);
  }
  vTaskDelete(NULL);
}