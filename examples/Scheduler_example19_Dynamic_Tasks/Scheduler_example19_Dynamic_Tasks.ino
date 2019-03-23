/**
    TaskScheduler Test sketch - test of Task destructor
    Test case:
      Main task runs every 100 milliseconds 100 times and in 50% cases generates a task object
      which runs 1 to 10 times with 100 ms to 5 s interval, and then destroyed.
      Garbage collection deletes all the tasks which have finished (enabled in their respective
      OnDisable methods)

      This sketch uses the following libraries:
       - FreeMemory library: https://github.com/McNeight/MemoryFree
       - QueueArray library: https://playground.arduino.cc/Code/QueueArray/
*/

#define _TASK_WDT_IDS // To enable task unique IDs
#define _TASK_SLEEP_ON_IDLE_RUN  // Compile with support for entering IDLE SLEEP state for 1 ms if not tasks are scheduled to run
#define _TASK_LTS_POINTER       // Compile with support for Local Task Storage pointer
#include <TaskScheduler.h>
#include <QueueArray.h>

#if defined (ARDUINO_ARCH_AVR)
#include <MemoryFree.h>
#elif defined(__arm__)
extern "C" char* sbrk(int incr);
static int freeMemory() {
  char top = 't';
  return &top - reinterpret_cast<char*>(sbrk(0));
}
#else
int freeMemory(); // supply your own
#endif

Scheduler ts;

// Callback methods prototypes
void MainLoop();
void GC();

// Statis task
Task tMain(100 * TASK_MILLISECOND, 100, &MainLoop, &ts, true);
Task tGarbageCollection(200 * TASK_MILLISECOND, TASK_FOREVER, &GC, &ts, false);


void Iteration();
bool OnEnable();
void OnDisable();

int noOfTasks = 0;
QueueArray <Task*> toDelete;

void MainLoop() {
  Serial.print(millis()); Serial.print("\t");
  Serial.print("MainLoop run: ");
  int i = tMain.getRunCounter();
  Serial.print(i); Serial.print(F(".\t"));

  if ( random(0, 101) > 50 ) {  // generate a new task only in 50% of cases
    // Generating another task
    long p = random(100, 5001); // from 100 ms to 5 seconds
    long j = random(1, 11); // from 1 to 10 iterations)
    Task *t = new Task(p, j, Iteration, &ts, false, OnEnable, OnDisable);

    Serial.print(F("Generated a new task:\t")); Serial.print(t->getId()); Serial.print(F("\tInt, Iter = \t"));
    Serial.print(p); Serial.print(", "); Serial.print(j);
    Serial.print(F("\tFree mem=")); Serial.print(freeMemory());
    Serial.print(F("\tNo of tasks=")); Serial.println(++noOfTasks);
    t->enable();
  }
  else {
    Serial.println(F("Skipped generating a task"));
  }
}


void Iteration() {
  Task &t = ts.currentTask();

  Serial.print(millis()); Serial.print("\t");
  Serial.print("Task N"); Serial.print(t.getId()); Serial.print(F("\tcurrent iteration: "));
  int i = t.getRunCounter();
  Serial.println(i);
}

bool OnEnable() {
  // to-do: think of something to put in here.
  return  true;
}

void OnDisable() {
  Task *t = &ts.currentTask();
  unsigned int tid = t->getId();
  toDelete.push(t);
  tGarbageCollection.enableIfNot();

  Serial.print(millis()); Serial.print("\t");
  Serial.print("Task N"); Serial.print(tid); Serial.println(F("\tfinished"));
}

/**
   Standard Arduino setup and loop methods
*/
void setup() {
  Serial.begin(115200);

  randomSeed(analogRead(0) + analogRead(5));
  noOfTasks = 0;

  Serial.println(F("Dynamic Task Creation/Destruction Example"));
  Serial.println();

  Serial.print(F("Free mem=")); Serial.print(freeMemory());
  Serial.print(F("\tNo of tasks=")); Serial.println(noOfTasks);
  Serial.println();
}

void GC() {
  if ( toDelete.isEmpty() ) {
    tGarbageCollection.disable();
    return;
  }
  Task *t = toDelete.pop();
  Serial.print(millis()); Serial.print("\t");
  Serial.print("Task N"); Serial.print(t->getId()); Serial.println(F("\tdestroyed"));
  Serial.print("Free mem="); Serial.print(freeMemory());
  Serial.print(F("\tNo of tasks=")); Serial.println(--noOfTasks);
  delete t;
}

void loop() {
  ts.execute();
}


