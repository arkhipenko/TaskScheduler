#ifndef _TICKER_H
#define _TICKER_H

#include "Arduino.h"

#define _TASK_SLEEP_ON_IDLE_RUN // Enable 1 ms SLEEP_IDLE powerdowns between tasks if no callback methods were invoked during the pass 
#define _TASK_STATUS_REQUEST    // Compile with support for StatusRequest functionality - triggering tasks on status change events in addition to time only
#define _TASK_WDT_IDS           // Compile with support for wdt control points and task ids
#define _TASK_PRIORITY          // Support for layered scheduling priority
#define _TASK_TIMEOUT           // Support for overall task timeout 
#define _TASK_OO_CALLBACKS

#include <TaskSchedulerDeclarations.h>

class Ticker : public Task {
  public:
    Ticker(Scheduler* aS, Task* aCalc, StatusRequest* aM);
    ~Ticker() {};

    bool Callback();

  private:
    Task  *iCalc;
    StatusRequest* iMeasure;
};


#endif
