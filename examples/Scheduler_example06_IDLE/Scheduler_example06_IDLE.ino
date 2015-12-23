
/**
 * This is a test to prove that processor really goes into IDLE sleep.
 * For this setup:
 * 
 *
 
Task c(10, -1, &Count, &ts);
Task t(10000, 1, NULL, &ts, true, &tOn, &tOff);

The result are:

1): With #define _TASK_SLEEP_ON_IDLE_RUN enabled
Start
c1=10771  (v1.9.0: same)
c2=1001


and

2): With #define _TASK_SLEEP_ON_IDLE_RUN disabled (commented out)
Start
c1=529783  (v1.9.0: 551947)
c2=1001

C1 is scenario 2) is much higher than in scenario 1) because processor is put to sleep for 1), but not for 2)

 */


/**
 * Compile and run once with _TASK_SLEEP_ON_IDLE_RUN enabled, then with _TASK_SLEEP_ON_IDLE_RUN disabled.
 * Compare the results.
 */
 
//#define _TASK_SLEEP_ON_IDLE_RUN
#include <TaskScheduler.h>

Scheduler ts;

// Callback methods prototypes
void Count();
bool tOn(); void tOff();

// Tasks
Task c(10, TASK_FOREVER, &Count, &ts);
Task t(10000, TASK_ONCE, NULL, &ts, true, &tOn, &tOff);


volatile unsigned long c1, c2;
bool tOn() {
  c1 = 0;
  c2 = 0;
  c.enable(); 
  
  return true;
}

void tOff() {
  c.disable();
  Serial.print("c1=");Serial.println(c1);
  Serial.print("c2=");Serial.println(c2);
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Start");
  t.delay();
 // ts.allowSleep(false);
}

void Count() {
  c2++;
}


void loop() {
  // put your main code here, to run repeatedly:
  ts.execute();
  c1++;
}