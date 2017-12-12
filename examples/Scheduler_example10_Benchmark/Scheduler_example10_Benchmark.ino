
/**
 * This is a test to benchmark TaskScheduler execution.
 * 
 * This test executes 1,000,000 cycles of a task with empty callback method
 * Compiled with different options, you can assess the impact of each on the size of the Task object
 * and the execution overhead of the main execution pass route.
 *
 * Sample execution times (in milliseconds per 1M iterations) are provided below.
 * The test board is Arduino UNO 16MHz processor.
 *
 
TaskScheduler 2.1.0:
No modifiers
Duration=19869

with SLEEP
Duration=20058

with status request:
Duration=20058

with time critical:
Duration=27289


TaskScheduler 1.9.0:
No modifiers
Duration=15656

with SLEEP
Duration=16285

with status request:
Duration=16600

with rollover fix:
Duration=18109


TaskScheduler 1.8.5:
Duration=15719

with SLEEP
Duration=16348

with status request:
Duration=18360

with rollover fix:
Duration=18423

 */


//#define _TASK_TIMECRITICAL     // Enable monitoring scheduling overruns
//#define _TASK_STATUS_REQUEST     // Compile with support for StatusRequest functionality - triggering tasks on status change events in addition to time only
//#define _TASK_WDT_IDS          // Compile with support for wdt control points and task ids
//#define _TASK_LTS_POINTER      // Compile with support for local task storage pointer
//#define _TASK_SLEEP_ON_IDLE_RUN
//#define _TASK_MICRO_RES
#include <TaskScheduler.h>

Scheduler ts;

// Callback methods prototypes
bool tOn(); void tOff();
void callback();

// Tasks
Task t(TASK_IMMEDIATE, 1000000, &callback, &ts, false, &tOn, &tOff);

unsigned long c1, c2;

bool tOn() {
  c1 = millis();
  c2 = 0;
  
  return true;
}

void tOff() {
  c2 = millis();
  Serial.println("done.");
  Serial.print("Tstart =");Serial.println(c1);
  Serial.print("Tfinish=");Serial.println(c2);
  Serial.print("Duration=");Serial.println(c2-c1);
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.print("Start...");

  t.enable();
}

void callback() {
  
}


void loop() {
  // put your main code here, to run repeatedly:
  ts.execute();
}
