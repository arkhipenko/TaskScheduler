/** 
 *  TaskScheduler Test sketch - use of task IDs and watchdog timer to identify hung tasks
 *  THIS SKETCH RUNS ON AVR BOARDS ONLY
 *  Test case: 
 *    Watchdog timer is set to 2 seconds (interrupt + reset)
 *    A hearbeat task (resetting the watchdog timer) is scheduled with 500 ms interval
 *    A number of tasks are running every 1 second and "rolling the dice" 0..19.  If 5, task is made to enter infinite loop
 *    Device should reset in 2 seconds after a task enters infinite loop
 *    A task id and a control point number are saved to EEPROM prior to device reset, and are displayed after reboot.
 *    In real life, device might chose to NOT activate certain tasks which failed previously (failed sensors for instance)
 */

#define _TASK_SLEEP_ON_IDLE_RUN
#define _TASK_WDT_IDS
#include <TaskScheduler.h>

#include <EEPROM.h>
#include <avr/wdt.h>

Scheduler ts;

// Callback methods prototypes
void TaskCB(); 
void HB(); bool HBOn(); void HBOff();

// Three tasks emulating accidental infinite loop
Task tTask1(TASK_SECOND, TASK_FOREVER, &TaskCB, &ts, true);
Task tTask2(TASK_SECOND, TASK_FOREVER, &TaskCB, &ts, true);
Task tTask3(TASK_SECOND, TASK_FOREVER, &TaskCB, &ts, true);

// Heartbeat task - resetting the watchdog timer periodically
// Initiates WDT on enable, and deactivates it on disable
Task tHB(500, TASK_FOREVER, &HB, &ts, false, &HBOn, &HBOff);

/**
 * Emulating task callback function
 *   Prints task id and randomly "hangs" in two places.
 *   Control points are stored on the task prior to section which might hang,
 *   making this information available to the WDT interrupt handler
 */
void TaskCB() {
  Task& T = ts.currentTask();
  
  Serial.print("Task #:"); 
  Serial.print(T.getId()); 
  Serial.print(" current iteration = "); 
  Serial.println(T.getRunCounter());

// Hang if random number between 0 and 19 is 5 (5% probability)
  T.setControlPoint(10);
  if (random(20) == 5) for(;;);
  
// Hang if random number between 0 and 99 is more that 95 (5% probability)
  T.setControlPoint(95);
  if (random(100) > 94) for(;;);
}

/**
 * This On Enable method sets up the WDT
 * for interrupt and reset after 2 seconds
 */
bool HBOn() {
  
  //disable interrupts
  cli();
  //reset watchdog
  wdt_reset();
  //set up WDT interrupt
  WDTCSR = (1<<WDCE)|(1<<WDE);
  //Start watchdog timer with aDelay prescaller
  WDTCSR = (1<<WDIE)|(1<<WDE)|(WDTO_2S & 0x2F);
//  WDTCSR = (1<<WDIE)|(WDTO_2S & 0x2F);  // interrupt only without reset
  //Enable global interrupts
  sei();
}

/**
 * This On Disable method disables WDT
 */
void HBOff() {
  wdt_disable();
}

/**
 * This is a periodic reset of WDT
 */
void HB() {
  wdt_reset();

}

/**
 * Watchdog timeout ISR
 * 
 */
ISR(WDT_vect)
{
  Task& T = ts.currentTask();

  digitalWrite(13, HIGH);
  EEPROM.write(0, (byte)T.getId());
  EEPROM.write(1, (byte)T.getControlPoint());
  digitalWrite(13, LOW);
}

/**
 * Standard arduino setup routine
 */
void setup() {
  
  Serial.begin(115200);

  randomSeed(analogRead(0)+analogRead(5));
  
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW); 
  
 
  Serial.println("WDT heartbeat test");
  Serial.print("Last task before reset="); Serial.println(EEPROM.read(0));
  Serial.print("Last control point before reset="); Serial.println(EEPROM.read(1));
  
  delay(2000);

  tHB.enableDelayed();
}

/**
 * Not much is left for the loop()
 */
void loop() {
  ts.execute();
}