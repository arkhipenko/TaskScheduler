#ifndef _CALCULATOR_H
#define _CALCULATOR_H

#include "Arduino.h"

#define _TASK_SLEEP_ON_IDLE_RUN // Enable 1 ms SLEEP_IDLE powerdowns between tasks if no callback methods were invoked during the pass 
#define _TASK_STATUS_REQUEST    // Compile with support for StatusRequest functionality - triggering tasks on status change events in addition to time only
#define _TASK_WDT_IDS           // Compile with support for wdt control points and task ids
#define _TASK_PRIORITY          // Support for layered scheduling priority
#define _TASK_TIMEOUT           // Support for overall task timeout 
#define _TASK_OO_CALLBACKS      // Support for dynamic callback method binding

#include <TaskSchedulerDeclarations.h>

class Calculator : public Task {
  public:
    Calculator( Scheduler* aS, Scheduler* aSensors);

    void reportDistance(long aD);

    bool Callback();
    bool OnEnable();
    void OnDisable();

  private:
    Scheduler*  iS;

    long distance;
    int iNS;

};

#endif // _CALCULATOR_H

