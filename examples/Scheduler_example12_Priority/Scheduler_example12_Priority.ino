/**
 * This is a test of TaskScheduler layered priority funtionality
 * 
 * Current test employs three priority layers:
 * Base scheduler runs tasks t1, t2 and t3
 * High priority scheduler runs tasks t4 and t5
 * Highest priority scheduler runs tasks t6 and t7
 * 
 * Sequence of task scheduling (not execution!) is:
 * 6, 7, 4, 6, 7, 5, 1, 6, 7, 4, 6, 7, 5, 2, 6, 7, 4, 6, 7, 5, 3 = one base scheduler pass
 * 
 * Scheduling overhead (at 20 micros per one pass) is: (B + B * H + B * H * C) * T = (3 + 3 * 2 + 3 * 2 * 2) * 18 = 378 micros
 * where
 *  B - number of tasks in the base scheduler's chain
 *  H - number of tasks in the high priority scheduler's chain
 *  C - number of tasks in the critical priority scheduler's chain
 *  T - scheduling overhead for 1 pass (~15-18 microseconds) 
 *  
 *  Actual task execution order:

Scheduler Priority Test
Task: 600:  0 Start delay = 0
Task: 700:  10  Start delay = 10
Task: 40: 21  Start delay = 21
Task: 50: 31  Start delay = 31
Task: 1:  43  Start delay = 41
Task: 2:  53  Start delay = 53
Task: 3:  63  Start delay = 63

Task: 600:  500 Start delay = 0
Task: 40: 510 Start delay = 10
Task: 600:  1000  Start delay = 0
Task: 700:  1010  Start delay = 10
Task: 40: 1021  Start delay = 21
Task: 50: 1032  Start delay = 32
Task: 1:  1043  Start delay = 43
Task: 600:  1500  Start delay = 0
Task: 40: 1510  Start delay = 10
Task: 600:  2000  Start delay = 0
Task: 700:  2011  Start delay = 11
Task: 40: 2022  Start delay = 22
Task: 50: 2032  Start delay = 32
Task: 1:  2043  Start delay = 43
Task: 2:  2054  Start delay = 54
Task: 600:  2500  Start delay = 0
Task: 40: 2510  Start delay = 10
Task: 600:  3000  Start delay = 0
Task: 700:  3010  Start delay = 10
Task: 40: 3021  Start delay = 21
Task: 50: 3032  Start delay = 32
Task: 1:  3043  Start delay = 43
Task: 3:  3053  Start delay = 53

Task: 600:  3500  Start delay = 0
Task: 40: 3510  Start delay = 10
Task: 600:  4000  Start delay = 0
Task: 700:  4011  Start delay = 11
Task: 40: 4022  Start delay = 22
Task: 50: 4032  Start delay = 32
Task: 1:  4043  Start delay = 43
Task: 2:  4054  Start delay = 54
Task: 600:  4500  Start delay = 0
Task: 40: 4510  Start delay = 10
Task: 600:  5000  Start delay = 0
Task: 700:  5010  Start delay = 10
Task: 40: 5021  Start delay = 21
Task: 50: 5031  Start delay = 31
Task: 1:  5043  Start delay = 43
Task: 600:  5500  Start delay = 0
Task: 40: 5511  Start delay = 11
Task: 600:  6000  Start delay = 0
Task: 700:  6010  Start delay = 10
Task: 40: 6022  Start delay = 22
Task: 50: 6032  Start delay = 32
Task: 1:  6043  Start delay = 43
Task: 2:  6053  Start delay = 53
Task: 3:  6065  Start delay = 65

 */
 
#define _TASK_SLEEP_ON_IDLE_RUN
#define _TASK_PRIORITY
#define _TASK_WDT_IDS
#define _TASK_TIMECRITICAL
#include <TaskScheduler.h>

Scheduler r;
Scheduler hpr;
Scheduler cpr;

// Callback methods prototypes
void tCallback();

// Tasks
Task t1(1000, TASK_FOREVER, &tCallback, &r);  //adding task to the chain on creation
Task t2(2000, TASK_FOREVER, &tCallback, &r);
Task t3(3000, TASK_FOREVER, &tCallback, &r);

Task t4(500, TASK_FOREVER, &tCallback, &hpr);  //adding task to the chain on creation
Task t5(1000, TASK_FOREVER, &tCallback, &hpr);  //adding task to the chain on creation

Task t6(500, TASK_FOREVER, &tCallback, &cpr);  //adding task to the chain on creation
Task t7(1000, TASK_FOREVER, &tCallback, &cpr);  //adding task to the chain on creation

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
  
  t6.setId(600);
  t7.setId(700);

  r.setHighPriorityScheduler(&hpr); 
  hpr.setHighPriorityScheduler(&cpr); 
  r.enableAll(true); // this will recursively enable the higher priority tasks as well
}


void loop () {
  
  r.execute();
  
}
