/** 
 *  TaskScheduler Test sketch - use of task's Local Task Storage pointer
 *  Test case: 
 *    Overall test runs for 5 seconds
 *    A number of calculator tasks run every one second, and update their respective variables using Local Task Storage pointer
 *    All calculator tasks use the same callback code, which obtains reference to appropriate variables via LTS pointer
 *    Calculaotr tasks perform simple calculation (as an example):
 *      adding task id number to itself
 *      multiplying task id number by 10
 *      
 *  Upon completion of the overall test, all results are printed out.
 *  Test could be repeated with various number of calculator tasks. 
 *  All that needs to change is data definitions - code is completely agnostic of number of tasks
 */
 
#define _TASK_SLEEP_ON_IDLE_RUN  // Compile with support for entering IDLE SLEEP state for 1 ms if not tasks are scheduled to run
#define _TASK_WDT_IDS           // Compile with support for Task IDs and Watchdog timer
#define _TASK_LTS_POINTER       // Compile with support for Local Task Storage pointer
#include <TaskScheduler.h>

// Overall number of calculator tasks:
#define NO_TASKS  3

Scheduler ts;

// Callback methods prototypes
void Calculate(); bool CalcOn();
bool WrapperOn(); void WrapperOff(); 

// Tasks
// Calculator tasks.
// Note that all three tasks use the same callback methods
// They will be updating specific variables based on the
// Locat Task Storage pointers 
Task t1(TASK_SECOND, TASK_FOREVER, &Calculate, &ts, false, &CalcOn); 
Task t2(TASK_SECOND, TASK_FOREVER, &Calculate, &ts, false, &CalcOn); 
Task t3(TASK_SECOND, TASK_FOREVER, &Calculate, &ts, false, &CalcOn);
// add more calc tasks here if necessary

Task tWrapper(5*TASK_SECOND, TASK_ONCE, NULL, &ts, false, &WrapperOn, &WrapperOff); 

// The below structure is an object referenced by LTS pointer
typedef struct {
  unsigned int id;
  long         sum;
  long         product;
} task_var;

// These are actual structures which hold tasks specific values
task_var v1;
task_var v2; 
task_var v3; 

// Arrays below allow indexed access to specific tasks and tasks variables
Task     *tasks[] = { &t1, &t2, &t3 };
task_var *vars[]  = { &v1, &v2, &v3 };


/**
 * This method is called when a wrapper task is enabled
 * The purpose is to supply LTS pointers to all the tasks
 */
bool WrapperOn() {

  for (int i=0; i < NO_TASKS; i++) {
    Task& T = *tasks[i];
    T.setLtsPointer( vars[i] );
    T.enableDelayed();
  }
  
  return true;  // Signal that Task could be enabled
}

/**
 * This method is called when Wrapper task is disabled (after first and only iteration is executed)
 * For each of the calculor tasks the results are printed out. 
 */
void WrapperOff() {
  Serial.println("Finished processing");
  
  ts.disableAll();
  
  for (int i=0; i < NO_TASKS; i++) {
    Serial.print("ID: "); Serial.println(vars[i]->id);
    Serial.print("Sum: "); Serial.println(vars[i]->sum);
    Serial.print("Product: "); Serial.println(vars[i]->product);
    Serial.println();
  }
}


/**
 * This method is executed when each calculator task is enabled
 * The purpose is to initiate all local variables
 */
bool CalcOn() {
  Task& T = ts.currentTask();
  task_var& var = *((task_var*) T.getLtsPointer());
  
// Initialize local variables
  var.id = T.getId();
  var.sum = 0;
  var.product = var.id;  
  
  return true;
}


/**
 * This method performs simple calculations on task's local variables
 */
void Calculate() {
  Task& T = ts.currentTask();
// Another way to get to LTS pointer:  
  task_var& var = *((task_var*) ts.currentLts());


  Serial.print("Calculating for task: ");
  Serial.print(T.getId());
  Serial.print("; Task id per LTS is: ");
  Serial.println( var.id );
  
  var.sum += T.getId();
  var.product = var.product * 10;
  
}


/**
 * Standard Arduino setup and loop methods
 */
void setup() {
  Serial.begin(115200);

  randomSeed(analogRead(0)+analogRead(5));
  
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW); 
 
  Serial.println("Local Task Storage pointer test");

  tWrapper.enableDelayed();
}

void loop() {
  ts.execute();
}