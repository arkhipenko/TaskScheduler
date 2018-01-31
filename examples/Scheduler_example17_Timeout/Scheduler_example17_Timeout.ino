/*
    This eaxmple illustrates the use of overall Task timeout functionality:

    Task 1 - runs every 1 seconds and times out in 10 seconds
    Task 2 - runs every 5 seconds and resets the timeout every run, so runs continuosly even though the timeout is set to 10 seconds
*/


// #define _TASK_TIMECRITICAL      // Enable monitoring scheduling overruns
#define _TASK_SLEEP_ON_IDLE_RUN // Enable 1 ms SLEEP_IDLE powerdowns between tasks if no callback methods were invoked during the pass 
//#define _TASK_STATUS_REQUEST    // Compile with support for StatusRequest functionality - triggering tasks on status change events in addition to time only
// #define _TASK_WDT_IDS           // Compile with support for wdt control points and task ids
// #define _TASK_LTS_POINTER       // Compile with support for local task storage pointer
// #define _TASK_PRIORITY          // Support for layered scheduling priority
// #define _TASK_MICRO_RES         // Support for microsecond resolution
// #define _TASK_STD_FUNCTION      // Support for std::function (ESP8266 ONLY)
// #define _TASK_DEBUG             // Make all methods and variables public for debug purposes
// #define _TASK_INLINE         // Make all methods "inline" - needed to support some multi-tab, multi-file implementations
#define _TASK_TIMEOUT

#include <TaskScheduler.h>

Scheduler ts;

void task1Callback();
void task1OnDisable();
void task2Callback();
void task2OnDisable();

Task t1(1 * TASK_SECOND, TASK_FOREVER, &task1Callback, &ts, false, NULL, &task1OnDisable);
Task t2(5 * TASK_SECOND, TASK_FOREVER, &task2Callback, &ts, false, NULL, &task2OnDisable);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  Serial.println("TaskScheduler Timeout example");
  Serial.println("=============================");

  t1.setTimeout(10 * TASK_SECOND);
  t2.setTimeout(10 * TASK_SECOND);

  ts.enableAll();
}

void loop() {
  // put your main code here, to run repeatedly:
  ts.execute();
}


void task1Callback() {
  Serial.print("Task 1:\t");
  Serial.print(millis());
  Serial.print(": t/out=");
  Serial.print(t1.getTimeout());
  Serial.print("\tms until t/out=");
  Serial.println( t1.untilTimeout());

}
void task1OnDisable() {
  if (t1.timedOut()) {
    Serial.println("Task 1 has timed out. Restarting");
    t1.setInterval(1 * TASK_SECOND);
    t1.setIterations(15);
    t1.setTimeout(TASK_NOTIMEOUT);
    t1.enable(); 
  }
  else {
    Serial.println("Task 1 has been disabled");
  }
}

void task2Callback() {
  Serial.print("Task 2:\t");
  Serial.print(millis());
  Serial.print(": t/out=");
  Serial.print(t2.getTimeout());
  Serial.print("\tms until t/out=");
  Serial.println( t2.untilTimeout());
  t2.resetTimeout();
}
void task2OnDisable() {
  if (t2.timedOut()) {
    Serial.println("Task 2 has timed out");
  }
  else {
    Serial.println("Task 2 has been disabled");
  }
}
