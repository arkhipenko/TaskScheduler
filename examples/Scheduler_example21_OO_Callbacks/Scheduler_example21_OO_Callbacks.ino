/**
    This is example 5 rewritten with Timeout, LTS, WDT functioanlity + multitab and extra classes
    - 1 second timeout is set for the main calculation task
    - LTS is used to address task-specific sensor class object
    - WDT is used to set the Task ID and use that for identifying the tasks (debug)

    Original description:
    ====================
      This test emulates querying 1 to 10 sensors once every 10 seconds, each could respond with a different delay
      (ultrasonic sensors for instance) and printing a max value of them when all have reported their values.
      The overall timeout of 1 second is setup as well.
      An error message needs to be printed if a timeout occurred instead of a distance value.

      Task and SuperSensor objects are dynamically created and destroyed as needed every 10 seconds

       This sketch uses a FreeMemory library: https://github.com/McNeight/MemoryFree
*/


#define _TASK_SLEEP_ON_IDLE_RUN // Enable 1 ms SLEEP_IDLE powerdowns between tasks if no callback methods were invoked during the pass 
#define _TASK_STATUS_REQUEST    // Compile with support for StatusRequest functionality - triggering tasks on status change events in addition to time only
#define _TASK_WDT_IDS           // Compile with support for wdt control points and task ids
#define _TASK_PRIORITY          // Support for layered scheduling priority
#define _TASK_TIMEOUT           // Support for overall task timeout 
#define _TASK_OO_CALLBACKS

#include <TaskScheduler.h>

#include "SuperSensor.h"
#include "Calculator.h"
#include "Ticker.h"

StatusRequest measure;

Scheduler ts, hts;

// Tasks

Calculator* tCalculate;//(&hts, &ts);
Ticker*     tCycle; //(&hts, (Task*) &tCalculate, &measure);

int pins[] = { 1, 9, 3, 7, 5, 6, 4, 8, 2, 10 };


/** Main Arduino code
  Not much is left here - everything is taken care of by the framework
*/
void setup() {

  Serial.begin(115200);
  Serial.println("TaskScheduler StatusRequest Sensor Emulation Test. Complex Test.");
  randomSeed(analogRead(A0) + millis());

  ts.setHighPriorityScheduler(&hts);

  tCalculate = new Calculator (&hts, &ts);
  tCycle     = new Ticker (&hts, (Task*) &tCalculate, &measure);

  tCalculate->setTimeout(1 * TASK_SECOND);
  tCycle->enable();
}

void loop() {
  ts.execute();
}
