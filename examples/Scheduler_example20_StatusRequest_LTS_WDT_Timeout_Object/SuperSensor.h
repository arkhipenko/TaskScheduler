#ifndef _SUPER_SENSOR_H
#define _SUPER_SENSOR_H


#include "Arduino.h"
#include <TaskSchedulerDeclarations.h>

class SuperSensor {
  public:
    SuperSensor(int aPin);
    ~SuperSensor();
    void begin();
    void stop();
    long trigger();
    bool measurementReady();
    long value();

  private: 
    long  iDelay;
    long  iValue;
    int   iPin;
    unsigned long iStart;
};

#endif // _SUPER_SENSOR_H
