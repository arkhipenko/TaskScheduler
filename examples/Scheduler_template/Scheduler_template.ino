// #define _TASK_TIMECRITICAL      // Enable monitoring scheduling overruns
// #define _TASK_SLEEP_ON_IDLE_RUN // Enable 1 ms SLEEP_IDLE powerdowns between tasks if no callback methods were invoked during the pass
// #define _TASK_STATUS_REQUEST    // Compile with support for StatusRequest functionality - triggering tasks on status change events in addition to time only
// #define _TASK_WDT_IDS           // Compile with support for wdt control points and task ids
// #define _TASK_LTS_POINTER       // Compile with support for local task storage pointer
// #define _TASK_PRIORITY          // Support for layered scheduling priority
// #define _TASK_MICRO_RES         // Support for microsecond resolution
// #define _TASK_STD_FUNCTION      // Support for std::function (ESP8266 and ESP32 ONLY)
// #define _TASK_DEBUG             // Make all methods and variables public for debug purposes
// #define _TASK_INLINE            // Make all methods "inline" - needed to support some multi-tab, multi-file implementations
// #define _TASK_TIMEOUT           // Support for overall task timeout
// #define _TASK_OO_CALLBACKS      // Support for dynamic callback method binding
#include <TaskScheduler.h>


// Scheduler
Scheduler ts;

void task1Callback();
void task2Callback();

/*
Scheduling defines:
  TASK_MILLISECOND
  TASK_SECOND
  TASK_MINUTE
  TASK_HOUR
  TASK_IMMEDIATE
  TASK_FOREVER
  TASK_ONCE
  TASK_NOTIMEOUT
*/

Task t1 (100 * TASK_MILLISECOND, TASK_FOREVER, &task1Callback, &ts, true);
Task t2 (TASK_IMMEDIATE, 100, &task2Callback, &ts, true);


void setup() {
  // put your setup code here, to run once:

}

void loop() {
  ts.execute();
}


void task1Callback() {

}

void task2Callback() {

}




