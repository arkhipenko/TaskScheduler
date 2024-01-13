/**
    TaskScheduler Test sketch - test of Task destructor
    Test case:
      Main task runs every 100 milliseconds 100 times and in 50% cases generates a task object
      which runs 1 to 10 times with 100 ms to 5 s interval, and then destroyed.
      Self-destruct feature takes care of the task deletion after tasks complete
      Last 5 tasks are deleted with disableAll() scheduler method as a test

      This sketch uses the following libraries:
       - FreeMemory library: https://github.com/McNeight/MemoryFree
       - QueueArray library: https://playground.arduino.cc/Code/QueueArray/
*/

#define _TASK_WDT_IDS // To enable task unique IDs
#define _TASK_SLEEP_ON_IDLE_RUN // Compile with support for entering IDLE SLEEP state for 1 ms if not tasks are scheduled to run
#define _TASK_LTS_POINTER       // Compile with support for Local Task Storage pointer
#define _TASK_SELF_DESTRUCT     // Enable tasks to "self-destruct" after disable
#include <TaskScheduler.h>

int freeMemory();

#if defined (ARDUINO_ARCH_AVR)
#include <MemoryFree.h>
#elif defined(__arm__)
extern "C" char* sbrk(int incr);
static int freeMemory() {
  char top = 't';
  return &top - reinterpret_cast<char*>(sbrk(0));
}
#elif defined (ARDUINO_ARCH_ESP8266) || defined (ARDUINO_ARCH_ESP32)
int freeMemory() { return ESP.getFreeHeap();}
#else
//  Supply your own freeMemory method
int freeMemory() { return 0;}
#endif

Scheduler ts;

// Callback methods prototypes
void MainLoop();
void GC();

// Statis task
Task tMain(100 * TASK_MILLISECOND, 100, &MainLoop, &ts, true);

void Iteration();
bool OnEnable();
void OnDisable();

int noOfTasks = 0;
long maxIterations = 0;

void MainLoop() {
  Serial.print(millis()); Serial.print("\t");
  Serial.print("MainLoop run: ");
  int i = tMain.getRunCounter();
  Serial.print(i); Serial.print(F(".\t"));

  if ( random(0, 101) > 50 ) {  // generate a new task only in 50% of cases
    // Generating another task
    long p = random(100, 5001); // from 100 ms to 5 seconds
    long j = random(1, 11); // from 1 to 10 iterations)
    maxIterations = ( j > maxIterations ) ? j : maxIterations;
    Task *t = new Task(p, j, Iteration, &ts, false, OnEnable, OnDisable, true); // enable self-destruct

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
  
  if (i >= maxIterations) {
    t.disable();
    maxIterations *= 2; // self-disable exactly one task
  }
}

bool OnEnable() {
  // to-do: think of something to put in here.
  return  true;
}

void OnDisable() {
  Task *t = &ts.currentTask();
  unsigned int tid = t->getId();

  Serial.print(millis()); Serial.print("\t");
  Serial.print("Task N"); Serial.print(tid); Serial.println(F("\tfinished"));
  Serial.print(F("\tNo of tasks=")); Serial.println(--noOfTasks);
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

void loop() {
  ts.execute();
  if ( millis() > 5000 && noOfTasks <= 5 ) {
      ts.disableAll();
      Serial.print(F("\tFree mem=")); Serial.println(freeMemory());
      while(1);
  }
}
