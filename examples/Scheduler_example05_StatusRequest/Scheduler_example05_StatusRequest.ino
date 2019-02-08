/** This test emulates querying 3 sensors once every 10 seconds, each could respond with a different delay
 *    (ultrasonic sensors for instance) and printing a min value of the three when all three have reported their values.
 *    The overall timeout of 1 second is setup as well.
 *    An error message needs to be printed if a timeout occurred instead of a value.
 */


#define _TASK_SLEEP_ON_IDLE_RUN
#define _TASK_STATUS_REQUEST
#include <TaskScheduler.h>

#ifdef ARDUINO_ARCH_STM32F1
#define A0	3
#endif

StatusRequest measure;

Scheduler ts; 

// Callback methods prototypes
void CycleCallback();
void MeasureCallback(); 
bool MeasureEnable();
void MeasureDisable();
void CalcCallback();
void S1Callback(); bool S1Enable();
void S2Callback(); bool S2Enable();
void S3Callback(); bool S3Enable();

// Tasks
Task tCycle(10000, TASK_FOREVER, &CycleCallback, &ts, true);
Task tMeasure(1000, TASK_ONCE, &MeasureCallback, &ts, false, &MeasureEnable, &MeasureDisable);
Task tCalculate(&CalcCallback, &ts);
Task tSensor1(0, TASK_ONCE, &S1Callback, &ts, false, &S1Enable);
Task tSensor2(0, TASK_ONCE, &S2Callback, &ts, false, &S2Enable);
Task tSensor3(0, TASK_ONCE, &S3Callback, &ts, false, &S3Enable);


long distance, d1, d2, d3;

void CycleCallback() {
  Serial.println("CycleCallback: Initiating measurement cycle every 10 seconds");

  tMeasure.restartDelayed();
}



bool MeasureEnable() {
  Serial.println("MeasureEnable: Activating sensors");  

  distance = 0;
  measure.setWaiting(3); // Set the StatusRequest to wait for 3 signals. 
  tCalculate.waitFor(&measure);

  tSensor1.restartDelayed();
  tSensor2.restartDelayed();
  tSensor3.restartDelayed();

  return true;
}

void MeasureCallback() {
  Serial.println("MeasureCallback: Invoked by calculate task or one second later");  

  if (measure.pending()) {
    tCalculate.disable();
    measure.signalComplete(-1);  // signal error
    Serial.println("MeasureCallback: Timeout!");
  }
  else {
    Serial.print("MeasureCallback: Min distance=");Serial.println(distance);
  }
}

void MeasureDisable() {
  Serial.println("MeasureDisable: Cleaning up");  
  
  tSensor1.disable();
  tSensor2.disable();
  tSensor3.disable();
}


void CalcCallback() {
  Serial.println("CalcCallback: calculating");  
  distance = -1;
  if ( measure.getStatus() >= 0) {  // only calculate if statusrequest ended successfully
    distance = d1 < d2 ? d1 : d2;
    distance = d3 < distance ? d3 : distance;
    tMeasure.forceNextIteration();
  }
}


/** Simulation code for sensor 1
 *  ----------------------------
 */
bool S1Enable() {
  Serial.print("S1Enable: Triggering sensor1. Delay=");  

  tSensor1.setInterval( random(1200) );  // Simulating sensor delay, which could go over 1 second and cause timeout
  d1 = 0;
  
  Serial.println( tSensor1.getInterval() );
  return true;
}

void S1Callback() {
  Serial.print("S1Callback: Emulating measurement. d1=");  
  d1 = random(501); // pick a value from 0 to 500 "centimeters" simulating a measurement 
  measure.signal();
  
  Serial.println(d1); 
}


/** Simulation code for sensor 2
 *  ----------------------------
 */
bool S2Enable() {
  Serial.print("S2Enable: Triggering sensor2. Delay=");  

  tSensor2.setInterval( random(1200) );  // Simulating sensor delay, which could go over 1 second and cause timeout
  d2 = 0;

  Serial.println( tSensor2.getInterval() );
  return true;
}

void S2Callback() {
  Serial.print("S2Callback: Emulating measurement. d2=");  
  d2 = random(501); // pick a value from 0 to 500 "centimeters" simulating a measurement
  measure.signal();

  Serial.println(d2);  
}


/** Simulation code for sensor 3
 *  ----------------------------
 */
bool S3Enable() {
  Serial.print("S3Enable: Triggering sensor3. Delay=");  

  tSensor3.setInterval( random(1200) );  // Simulating sensor delay, which could go over 1 second and cause timeout
  d3 = 0;

  Serial.println( tSensor3.getInterval() );
  return true;
}

void S3Callback() {
  Serial.print("S3Callback: Emulating measurement. d3=");  
  d3 = random(501); // pick a value from 0 to 500 "centimeters" simulating a measurement
  measure.signal();
  
  Serial.println(d3); 
}


/** Main Arduino code
 *  Not much is left here - everything is taken care of by the framework
 */
void setup() {

  Serial.begin(115200);
  Serial.println("TaskScheduler StatusRequest Sensor Emulation Test. Complex Test.");  
  
#ifdef ARDUINO_ARCH_STM32F1
  pinMode(A0, INPUT_ANALOG);
#endif
  
  randomSeed(analogRead(A0)+millis());
}

void loop() {
  
  ts.execute();

}
