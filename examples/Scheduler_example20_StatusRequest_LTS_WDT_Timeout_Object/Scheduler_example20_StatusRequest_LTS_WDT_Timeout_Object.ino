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
*/

// #define _TASK_TIMECRITICAL      // Enable monitoring scheduling overruns
#define _TASK_SLEEP_ON_IDLE_RUN // Enable 1 ms SLEEP_IDLE powerdowns between tasks if no callback methods were invoked during the pass 
#define _TASK_STATUS_REQUEST    // Compile with support for StatusRequest functionality - triggering tasks on status change events in addition to time only
#define _TASK_WDT_IDS           // Compile with support for wdt control points and task ids
#define _TASK_LTS_POINTER       // Compile with support for local task storage pointer
#define _TASK_PRIORITY          // Support for layered scheduling priority
// #define _TASK_MICRO_RES         // Support for microsecond resolution
// #define _TASK_STD_FUNCTION      // Support for std::function (ESP8266 and ESP32 ONLY)
#define _TASK_DEBUG             // Make all methods and variables public for debug purposes
#define _TASK_INLINE       // Make all methods "inline" - needed to support some multi-tab, multi-file implementations
#define _TASK_TIMEOUT           // Support for overall task timeout 

#include <TaskScheduler.h>
#include "SuperSensor.h"

StatusRequest measure;

Scheduler ts, hts;

// Callback methods prototypes
void CycleCallback();
void CalcCallback();
bool CalcEnable();
void CalcDisable();
void SCallback();
bool SEnable();
void SDisable();

// Tasks

Task tCycle(10000, TASK_FOREVER, &CycleCallback, &hts);
Task tCalculate(TASK_IMMEDIATE , TASK_ONCE, &CalcCallback, &hts, false, &CalcEnable, &CalcDisable);

int numberSensors;
long distance;
int pins[] = { 1, 9, 3, 7, 5, 6, 4, 8, 2, 10 };

void CycleCallback() {
  Serial.println();Serial.println();Serial.println();
  Serial.print(millis()); Serial.print(":\t");
  Serial.println("CycleCallback: Initiating measurement cycle every 10 seconds");
  Serial.print("Number of sensors=");
  
  numberSensors = random(1, 11); // 1 to 10 sensors, randomly
  distance = 0;
  Serial.println(numberSensors);

  measure.setWaiting(numberSensors); // Set the StatusRequest to wait for 3 signals.
  tCalculate.waitFor(&measure);
  tCalculate.setTimeout(1000 * TASK_MILLISECOND);
}

bool CalcEnable() {
  Serial.print(millis()); Serial.print(":\t");
  Serial.println("CalcEnable: OnEnable");
  Serial.println("Activating sensors");


  for (int i = 0; i < numberSensors; i++) {
    Task *t = new Task(TASK_MILLISECOND, TASK_FOREVER, &SCallback, &ts, false, &SEnable, &SDisable);
    SuperSensor *s = new SuperSensor( pins[i] );
    t->setLtsPointer( (void*) s);
    t->setId(i + 1);

    s->begin();

    t->restartDelayed();
  }

  return true;
}

void CalcDisable() {
  if (tCalculate.timedOut()) {
    measure.signalComplete(-1);  // signal error
    Serial.print(millis()); Serial.print(":\t");
    Serial.println("MeasureCallback: ***** Timeout *****");
  }
  ts.disableAll(false); // only disable tasks in the ts scheduler
}


void CalcCallback() {
  Serial.print(millis()); Serial.print(":\t");
  Serial.println("CalcCallback: calculating");
  if ( measure.getStatus() >= 0) {  // only calculate if statusrequest ended successfully
    Serial.print("CalcCallback: Max distance="); Serial.println(distance);
    Serial.println();
  }
}


/** Simulation code for all sensors
    -------------------------------
*/
bool SEnable() {
  Task &t = ts.currentTask();
  int i = t.getId();

  Serial.print(millis()); Serial.print(":\t");
  Serial.print("SEnable: TaskID=");
  Serial.println(i);
  Serial.print("Triggering sensor. Delay=");


  // Another way to update the distances with one codebase - use LTS pointers
  SuperSensor *s = (SuperSensor*) t.getLtsPointer();

  long dly = s->trigger();


  Serial.println( dly );
  return true;
}

void SCallback() {
  Task &t = ts.currentTask();

  SuperSensor *s = (SuperSensor*) t.getLtsPointer();
  if ( s->measurementReady() ) {
    int i = t.getId();
    Serial.print(millis()); Serial.print(":\t");
    Serial.print("SCallback: TaskID=");
    Serial.println(i);
    Serial.print("Emulating measurement. d=");

    long d = s->value();
    if ( d > distance ) distance = d;

    Serial.println(d);

    measure.signal();
    t.disable();
  }
}

void SDisable() {
  Task &t = ts.currentTask();
  int i = t.getId();

  Serial.print(millis()); Serial.print(":\t");
  Serial.print("SDisable: TaskID=");
  Serial.println(i);

  SuperSensor *s = (SuperSensor*) ts.currentLts();
  s->stop();

  delete s;
  delete &t;
}

/** Main Arduino code
  Not much is left here - everything is taken care of by the framework
*/
void setup() {

  Serial.begin(115200);
  Serial.println("TaskScheduler StatusRequest Sensor Emulation Test. Complex Test.");
  randomSeed(analogRead(A0) + millis());

  ts.setHighPriorityScheduler(&hts);

  tCalculate.setTimeout(1 * TASK_SECOND);
  tCycle.enable();
}

void loop() {
  ts.execute();
}
