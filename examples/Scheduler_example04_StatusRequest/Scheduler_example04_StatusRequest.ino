/** This test demonstrates interaction between three simple tasks via StatusRequest object.
 *  Task T1 runs every 5 seconds and signals completion of a status request st.
 *  Tasks T2 and T3 are waiting on the same request (st)
 *  Task T3 does not renew its interest in status request st, so it is only invoked once (first iteration)
 *  Task T2 is invoked every time st completes, because it renews its interest in status of status request object st every iteration of T1
 */
 
#define _TASK_SLEEP_ON_IDLE_RUN
#define _TASK_STATUS_REQUEST
#include <TaskScheduler.h>

StatusRequest st;

Scheduler ts; 

// Callback methods prototypes
void Callback1();
void Disable1();
void Callback2();
void Callback3();
void PrepareStatus();

// Tasks
Task t1(5000, TASK_ONCE, &Callback1, &ts, true, NULL, &Disable1);
Task t2(&Callback2, &ts);
Task t3(&Callback3, &ts);

/** T1 callback
 *  T1 just signals completion of st every 5 seconds
 */
void Callback1() {
  Serial.println("T1: Signaling completion of ST");
  st.signalComplete();
}

/** T1 On Disable callback
 *  This callback renews the status request and restarts T1 delayed to run again in 5 seconds
 */
void Disable1() {
  PrepareStatus();
  t1.restartDelayed();
}

/** T2 callback
 *  Invoked when status request st completes
 */
void Callback2() {
  Serial.println("T2: Invoked due to completion of ST");  
}


/** T3 callback
 *  Invoked when status request st completes.
 *  This is only run once since T3 does not renew its interest in the status request st after first iteration
 */
 void Callback3() {
  Serial.println("T3: Invoked due to completion of ST");  
  
}

/** Prepare Status request st for another iteration
 *  
 */
void PrepareStatus() {
  st.setWaiting();         // set the statusrequest object for waiting 
  t2.waitFor(&st);  // request tasks 1 & 2 to wait on the object st
}


/** Main Arduino code
 *  Not much to do here. Just init Serial and set the initial status request
 */
void setup() {

  Serial.begin(115200);
  delay(1000);
  Serial.println("TaskScheduler: Status Request Test 1. Simple Test.");  

  ts.startNow();
  PrepareStatus();
  t3.waitFor(&st);

  t1.delay();
}

void loop() {

  ts.execute();

}