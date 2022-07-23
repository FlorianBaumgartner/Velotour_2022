#ifndef OFFICE_H
#define OFFICE_H

#include <Arduino.h>
#include "system.h"
#include "hmi.h"
#include "mesh.h"

#define TASK_OFFICE_FREQ          10            // [Hz]

class Office
{
  public:
    Office(System& system, Hmi& hmi, Mesh& mesh): system(system), hmi(hmi), mesh(mesh) {}
    bool begin(void);

  private:
    System& system;
    Hmi& hmi;
    Mesh& mesh;

    static void update(void* pvParameter);
};

#endif
