/*
   An example of using scheduler's custom sleep callback method

   An empty loop is executed for 10 seconds with a 10 ms. interval
   The first time it is excuted with an empty sleep callback, and
   the second time with a 1 ms delay in the custom callback

  RESULTS:

  Arduino Nano:
=================================
  Testing empty sleep callback...
  cEmptyCallback=1001
  cEmptyTotal=423866
  Testing 1 ms delayed sleep callback...
  cDelayCallback=1001
  cDelayTotal=10669


  ESP8266 at 80MHz
=================================
  Testing empty sleep callback...
  cEmptyCallback=1001
  cEmptyTotal=278101
  Testing 1 ms delayed sleep callback...
  cDelayCallback=1001
  cDelayTotal=10493


  ESP8266 at 160MHz
=================================
  Testing empty sleep callback...
  cEmptyCallback=1001
  cEmptyTotal=546041
  Testing 1 ms delayed sleep callback...
  cDelayCallback=1001
  cDelayTotal=10746


  Maple Mini STM32 board at 70MHz -O3 code optimization
==================================
  Testing empty sleep callback...
  cEmptyCallback=1001
  cEmptyTotal=2689973
  Testing 1 ms delayed sleep callback...
  cDelayCallback=1001
  cDelayTotal=10958


  esp32 at 240MHz
==================================
  Testing empty sleep callback...
  cEmptyCallback=1001
  cEmptyTotal=492851
  Testing 1 ms delayed sleep callback...
  cDelayCallback=1001
  cDelayTotal=11002


*/

#define _TASK_SLEEP_ON_IDLE_RUN
#include <TaskScheduler.h>

Scheduler ts;

// Callback methods prototypes
void Count();
bool tEmptyOn();
void tEmptyOff();
bool tDelayOn();
void tDelayOff();

// Sleep methods prototypes
void sEmpty(unsigned long aT);
void sDelay(unsigned long aT);

// Tasks
Task tCount ( 10, TASK_FOREVER, &Count, &ts, false );
Task tEmpty ( 10000, TASK_ONCE, NULL, &ts, false, &tEmptyOn, &tEmptyOff );
Task tDelay ( 10000, TASK_ONCE, NULL, &ts, false, &tDelayOn, &tDelayOff );


volatile unsigned long cEmptyCallback, cEmptyTotal, cDelayCallback, cDelayTotal;
volatile unsigned long *cCB, *cTL;

void setup() {
  Serial.begin(115200);
  delay(5000);
  Serial.println("Start counting...");
  ts.setSleepMethod( &sEmpty );
  tEmpty.restartDelayed();
}


void sEmpty(unsigned long aT) {
}

void sDelay(unsigned long aT) {
  delay(1);
}

bool tEmptyOn() {
  Serial.println("Testing empty sleep callback...");
  cCB = &cEmptyCallback;
  cTL = &cEmptyTotal;

  *cCB = 0;
  *cTL = 0;

  tCount.restart();

  return true;
}

void tEmptyOff() {
  tCount.disable();

  Serial.print("cEmptyCallback="); Serial.println(*cCB);
  Serial.print("cEmptyTotal="); Serial.println(*cTL);

  ts.setSleepMethod( &sDelay );
  tDelay.restartDelayed();
}


bool tDelayOn() {
  Serial.println("Testing 1 ms delayed sleep callback...");
  cCB = &cDelayCallback;
  cTL = &cDelayTotal;

  *cCB = 0;
  *cTL = 0;

  tCount.restart();

  return true;
}

void tDelayOff() {
  tCount.disable();

  Serial.print("cDelayCallback="); Serial.println(*cCB);
  Serial.print("cDelayTotal="); Serial.println(*cTL);
}

void Count() {
  (*cCB)++;
}



void loop() {
  // put your main code here, to run repeatedly:
  ts.execute();
  (*cTL)++;
}
