/*
    TaskScheduler Example #25

    Create 10 random Tasks. Prints the entire chain.
    Then executes chain once printing the remaining chain from currently active task
    Deletes all tasks afterwards
*/

// #define _TASK_TIMECRITICAL      // Enable monitoring scheduling overruns
#define _TASK_SLEEP_ON_IDLE_RUN // Enable 1 ms SLEEP_IDLE powerdowns between tasks if no callback methods were invoked during the pass
// #define _TASK_STATUS_REQUEST    // Compile with support for StatusRequest functionality - triggering tasks on status change events in addition to time only
#define _TASK_WDT_IDS           // Compile with support for wdt control points and task ids
// #define _TASK_LTS_POINTER       // Compile with support for local task storage pointer
// #define _TASK_PRIORITY          // Support for layered scheduling priority
// #define _TASK_MICRO_RES         // Support for microsecond resolution
// #define _TASK_STD_FUNCTION      // Support for std::function (ESP8266 and ESP32 ONLY)
// #define _TASK_DEBUG             // Make all methods and variables public for debug purposes
// #define _TASK_INLINE            // Make all methods "inline" - needed to support some multi-tab, multi-file implementations
// #define _TASK_TIMEOUT           // Support for overall task timeout
// #define _TASK_OO_CALLBACKS      // Support for dynamic callback method binding
// #define _TASK_DEFINE_MILLIS     // Force forward declaration of millis() and micros() "C" style
#define _TASK_EXPOSE_CHAIN      // Methods to access tasks in the task chain

#include <TaskScheduler.h>
#include <QueueArray.h>

// Debug and Test options
#define _DEBUG_
//#define _TEST_

#ifdef _DEBUG_
#define _PP(a) Serial.print(a);
#define _PL(a) Serial.println(a);
#else
#define _PP(a)
#define _PL(a)
#endif


// Scheduler
Scheduler ts;

void taskCallback();
void cleanUp();

Task tManager(6000, TASK_ONCE, &cleanUp, &ts, false);
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

int noOfTasks = 10;
QueueArray <Task*> toDelete;

void setup() {
  // put your setup code here, to run once:
#if defined(_DEBUG_) || defined(_TEST_)
  Serial.begin(115200);
  delay(2000);
  _PL("Scheduler Example: Expose Scheduler Task Chain"); _PL();
#endif

  _PL("Generating a random chain of tasks");
  for (int i = 0; i < noOfTasks; i++) {
    long p = random(100, 5001); // from 100 ms to 5 seconds
    long j = random(1, 11); // from 1 to 10 iterations)
    Task *t = new Task(p, j, &taskCallback, &ts, false);
    _PP(F("Generated a new task:\t")); _PP(t->getId()); _PP(F("\tInt, Iter = \t"));
    _PP(p); Serial.print(", "); _PL(j);
    if ( random(1, 100) > 50 ) {
      t->enable();
    }
    else {
      t->enableDelayed();
    }
    toDelete.push(t);
  }
  _PL();
  _PL("Printing the entire chain");
  Task* f = ts.getFirstTask();
  Task* l = ts.getLastTask();
  while (f) {
    _PP("Task #"); _PL(f->getId());
    f = f->getNextTask();
  }
  _PL();
  tManager.enableDelayed();
}


void loop() {
  ts.execute();
}


void taskCallback() {
  _PP(millis());
  _PP(": taskCallback() of task #");
  Task* t = ts.getCurrentTask();
  t->disable();
  _PL(t->getId());
  Task* l = ts.getLastTask();
  while (t) {
    _PP("#"); _PP(t->getId());
    if (t->getNextTask() != NULL) _PP("->");
    t = t->getNextTask();
  }
  _PL(); _PL();
}

void cleanUp() {
    _PL("Deleting tasks:");
  do {
    Task* t = toDelete.pop();
    _PP("Deleting task #"); _PL(t->getId());
    delete t;
  } while (!toDelete.isEmpty());

  for (;;) ;
}
