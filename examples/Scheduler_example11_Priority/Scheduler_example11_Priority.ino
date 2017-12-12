/**
 * This is a test of TaskScheduler layered priority funtionality
 * 
 * Current test employs two priority layers:
 * Base scheduler runs tasks t1, t2 and t3
 * High priority scheduler runs tasks t4 and t5
 * 
 * Sequence of task scheduling (not execution!) is:
 * 4, 5, 1, 4, 5, 2, 4, 5, 3 = one base scheduler pass
 * 
 * Scheduling overhead (at 20 micros per one pass) is: (B + B * H) * T = (3 + 3 * 2) * 18 = 162 micros
 * where
 *  B - number of tasks in the base scheduler's chain
 *  H - number of tasks in the high priority scheduler's chain
 *  T - scheduling overhead for 1 pass (~15-18 microseconds) 
 *  
 *  Actual task execution order:

Scheduler Priority Test
Task: 40:  0 Start delay = 0
Task: 50: 10  Start delay = 10
Task: 1:  21  Start delay = 21
Task: 2:  31  Start delay = 31
Task: 3:  41  Start delay = 41

Task: 40: 500 Start delay = 0
Task: 40: 1000  Start delay = 0
Task: 50: 1010  Start delay = 10
Task: 1:  1021  Start delay = 20
Task: 40: 1500  Start delay = 0
Task: 40: 2000  Start delay = 0
Task: 50: 2011  Start delay = 11
Task: 1:  2022  Start delay = 21
Task: 2:  2032  Start delay = 32
Task: 40: 2500  Start delay = 0
Task: 40: 3000  Start delay = 0
Task: 50: 3010  Start delay = 10
Task: 1:  3021  Start delay = 20
Task: 3:  3032  Start delay = 32

Task: 40: 3500  Start delay = 0
Task: 40: 4000  Start delay = 0
Task: 50: 4011  Start delay = 11
Task: 1:  4022  Start delay = 21
Task: 2:  4032  Start delay = 32
Task: 40: 4500  Start delay = 0
Task: 40: 5000  Start delay = 0
Task: 50: 5010  Start delay = 10
Task: 1:  5021  Start delay = 20
Task: 40: 5500  Start delay = 0
Task: 40: 6000  Start delay = 0
Task: 50: 6010  Start delay = 10
Task: 1:  6022  Start delay = 21
Task: 2:  6032  Start delay = 32
Task: 3:  6043  Start delay = 42

 */
 
#define _TASK_SLEEP_ON_IDLE_RUN
#define _TASK_PRIORITY
#define _TASK_WDT_IDS
#define _TASK_TIMECRITICAL
#include <TaskScheduler.h>

Scheduler r, hpr;

// Callback methods prototypes
void tCallback();

// Tasks
Task t1(1000, TASK_FOREVER, &tCallback, &r);  //adding task to the chain on creation
Task t2(2000, TASK_FOREVER, &tCallback, &r);
Task t3(3000, TASK_FOREVER, &tCallback, &r);

Task t4(500, TASK_FOREVER, &tCallback, &hpr);  //adding task to the chain on creation
Task t5(1000, TASK_FOREVER, &tCallback, &hpr);  //adding task to the chain on creation

void tCallback() {
  Scheduler &s = Scheduler::currentScheduler();
  Task &t = s.currentTask();
  
  Serial.print("Task: "); Serial.print(t.getId());Serial.print(":\t");
  Serial.print(millis()); Serial.print("\tStart delay = "); Serial.println(t.getStartDelay());
  delay(10);

  if (t.getId() == 3) Serial.println();
}

void setup () {
  Serial.begin(115200);
  Serial.println("Scheduler Priority Test");

  t4.setId(40);
  t5.setId(50);

  r.setHighPriorityScheduler(&hpr); 
  r.enableAll(true); // this will recursively enable the higher priority tasks as well
}


void loop () {
  
  r.execute();
  
}
