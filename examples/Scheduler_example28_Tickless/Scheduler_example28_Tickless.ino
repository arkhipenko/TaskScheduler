/** 
 *  TaskScheduler Test
 * 
 *  This test illustrates how to use TS's tickless support functionality
 *  Tickless support enables more deterministic sleep by calculating time delay until next task invocation
 *  during every pass. That delay could then be used to put microcontroller in sleep mode continously
 *  instead of in small intervals
 *
 *  Initially only tasks 1 and 2 are enabled
 *  Task1 runs every 2 seconds 10 times and then stops
 *  Task2 runs every 3 seconds indefinitely
 *  Task1 enables Task3 at its first run
 *  Task3 run every 5 seconds
 *  Task1 disables Task3 on its last iteration and changed Task2 to run every 1/2 seconds
 *  At the end Task2 is the only task running every 1/2 seconds
 */

#define _TASK_SLEEP_ON_IDLE_RUN  // Enable 1 ms SLEEP_IDLE powerdowns between runs if no callback methods were invoked during the pass
#define _TASK_TICKLESS           // Enable support for tickless sleep on FreeRTOS
#define _TASK_DO_NOT_YIELD       // Disable yield() method in execute() 
 
#include <TaskScheduler.h>

// Callback methods prototypes
void t1Callback();
void t2Callback();
void t3Callback();

//Tasks
Task t4();
Task t1(2000, 10, &t1Callback);
Task t2(3000, TASK_FOREVER, &t2Callback);
Task t3(5000, TASK_FOREVER, &t3Callback);

Scheduler runner;


void t1Callback() {
    Serial.print("t1: ");
    Serial.println(millis());
    
    if (t1.isFirstIteration()) {
      runner.addTask(t3);
      t3.enable();
      Serial.println("t1: enabled t3 and added to the chain");
    }
    
    if (t1.isLastIteration()) {
      t3.disable();
      runner.deleteTask(t3);
      t2.setInterval(500);
      Serial.println("t1: disable t3 and delete it from the chain. t2 interval set to 500");
    }
}

void t2Callback() {
    Serial.print("t2: ");
    Serial.println(millis());
	
	// comment this line out if you want to test t2's 500 ms explicit delay
	// as-is this delay tests that task in catch up mode will prevent explicit tickless delay
    delay(501);
  
}

void t3Callback() {
    Serial.print("t3: ");
    Serial.println(millis());
  
}

void setup () {
  Serial.begin(115200);
  Serial.println("Scheduler TEST");
  
  runner.init();
  Serial.println("Initialized scheduler");
  
  runner.addTask(t1);
  Serial.println("added t1");
  
  runner.addTask(t2);
  Serial.println("added t2");

  delay(1000);
  
  t1.enable();
  Serial.println("Enabled t1");
  t2.enable();
  Serial.println("Enabled t2");
}

unsigned long nr = 0;
void loop () {
  runner.execute();
  nr = runner.getNextRun();
  if ( nr ) {
    Serial.println("TS stats:");
    Serial.print("\tTotal   tasks: "); Serial.println(runner.getTotalTasks() );
    Serial.print("\tActive  tasks: "); Serial.println(runner.getActiveTasks() );
    Serial.print("\tInvoked tasks: "); Serial.println(runner.getInvokedTasks() );
    Serial.print("Next scheduling pass in "); Serial.print(nr); Serial.println(" ms");
    delay(nr);
  }
}