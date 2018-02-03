/**
    This is example 5 rewritten with Timeout, LTS and WDT functioanlity:
    - 1 second timeout is set for the main calculation task
    - LTS is used to address individual array elements for each sensor sinlce the callback code is shared
    - WDT is used to set the Task ID and use that as an index for array of distances (alternative to LTS)

    Original description:
    ====================
      This test emulates querying 3 sensors once every 10 seconds, each could respond with a different delay
      (ultrasonic sensors for instance) and printing a min value of the three when all three have reported their values.
      The overall timeout of 1 second is setup as well.
      An error message needs to be printed if a timeout occurred instead of a value.

      Example5:
       Sketch uses 6066 bytes (18%) of program storage space. Maximum is 32256 bytes.
       Global variables use 1039 bytes (50%) of dynamic memory, leaving 1009 bytes for local variables. Maximum is 2048 bytes.
      Example 18:
       Sketch uses 5142 bytes (15%) of program storage space. Maximum is 32256 bytes.
       Global variables use 878 bytes (42%) of dynamic memory, leaving 1170 bytes for local variables. Maximum is 2048 bytes.
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

StatusRequest measure;

Scheduler ts, hts;

// Callback methods prototypes
void CycleCallback();
void CalcCallback();
bool CalcEnable();
void CalcDisable();
void SCallback(); bool SEnable();

// Tasks
Task tSensor1(0, TASK_ONCE, &SCallback, &ts, false, &SEnable);  // task ID = 1
Task tSensor2(0, TASK_ONCE, &SCallback, &ts, false, &SEnable); // task ID = 2
Task tSensor3(0, TASK_ONCE, &SCallback, &ts, false, &SEnable); // task ID = 3

Task tCycle(10000, TASK_FOREVER, &CycleCallback, &hts);
Task tCalculate(TASK_IMMEDIATE , TASK_ONCE, &CalcCallback, &hts, false, &CalcEnable, &CalcDisable);

#define NO_OF_SENSORS 3
long distance, d[NO_OF_SENSORS + 1], d_lts[NO_OF_SENSORS]; // d[] will be populated via task ID used as array indexes, d_lts will be addressed via LTS pointers

void CycleCallback() {
  Serial.println();
  Serial.print(millis()); Serial.print(":\t");
  Serial.println("CycleCallback: Initiating measurement cycle every 10 seconds");

  distance = 0;
  measure.setWaiting(NO_OF_SENSORS); // Set the StatusRequest to wait for 3 signals.
  tCalculate.waitFor(&measure);
}

bool CalcEnable() {
  Serial.print(millis()); Serial.print(":\t");
  Serial.println("CalcEnable: OnEnable");
  Serial.println("Activating sensors and setting timeout");

  tSensor1.restartDelayed();
  tSensor2.restartDelayed();
  tSensor3.restartDelayed();

  return true;
}

void CalcDisable() {
  if (tCalculate.timedOut()) {
    measure.signalComplete(-1);  // signal error
    Serial.print(millis()); Serial.print(":\t");
    Serial.println("MeasureCallback: ***** Timeout *****");
    //    tSensor1.disable();
    //    tSensor2.disable();
    //    tSensor3.disable();
  }
}


void CalcCallback() {
  Serial.print(millis()); Serial.print(":\t");
  Serial.println("CalcCallback: calculating");
  distance = -1;
  if ( measure.getStatus() >= 0) {  // only calculate if statusrequest ended successfully
    distance = d[1] < d[2] ? d[1] : d[2];
    distance = d[3] < distance ? d[3] : distance;
    Serial.print("CalcCallback: Min distance="); Serial.println(distance);
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

  t.setInterval( random(1200) );  // Simulating sensor delay, which could go over 1 second and cause timeout
  // One way to update the 3 distances with one codebase - use task id as an index
  d[i] = 0;

  // Another way to update the 3 distances with one codebase - use LTS pointers
  int *pd = (int*) t.getLtsPointer();
  *pd = 0;

  Serial.println( t.getInterval() );
  return true;
}

void SCallback() {
  Task &t = ts.currentTask();
  int i = t.getId();

  Serial.print(millis()); Serial.print(":\t");
  Serial.print("SCallback: TaskID=");
  Serial.println(i);
  Serial.print("Emulating measurement. d=");

  d[i] = random(501); // pick a value from 0 to 500 "centimeters" simulating a measurement
  int *pd = (int*) t.getLtsPointer();
  *pd = d[i];

  measure.signal();

  Serial.print(d[i]);
  Serial.print("\t");
  Serial.println(*pd);
}

/** Main Arduino code
    Not much is left here - everything is taken care of by the framework
*/
void setup() {

  Serial.begin(115200);
  Serial.println("TaskScheduler StatusRequest Sensor Emulation Test. Complex Test.");
  randomSeed(analogRead(A0) + millis());

  tSensor1.setLtsPointer(&d_lts[0]);
  tSensor2.setLtsPointer(&d_lts[1]);
  tSensor3.setLtsPointer(&d_lts[2]);

  ts.setHighPriorityScheduler(&hts);

  tCalculate.setTimeout(1 * TASK_SECOND);
  tCycle.enable();
}

void loop() {

  ts.execute();

}
