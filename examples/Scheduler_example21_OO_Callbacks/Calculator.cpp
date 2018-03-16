#include "Calculator.h"
#include "SuperSensor.h"
#include <MemoryFree.h>

Calculator::Calculator( Scheduler* aS, Scheduler* aSensors) : Task(aS) {
  iS = aSensors;
  setTimeout(1000 * TASK_MILLISECOND);
}

bool Calculator::Callback() {
  Serial.print(millis()); Serial.print(":\t");
  Serial.println("CalcCallback: calculating");
  if ( getStatusRequest()->getStatus() >= 0) {  // only calculate if statusrequest ended successfully
    Serial.print("CalcCallback: Max distance="); Serial.println(distance);
  }
  return false;
}

extern int pins[];

bool Calculator::OnEnable() {
  Serial.print(millis()); Serial.print(":\t");
  Serial.println("CalcEnable: OnEnable");
  Serial.println("Activating sensors");

  StatusRequest* sr = getStatusRequest();
  iNS = sr->getCount();

  distance = 0;
  for (int i = 0; i < iNS; i++) {
    SuperSensor *s = new SuperSensor( iS, pins[i], this, sr);
    s->setId(i + 1);
    s->begin();
    s->restartDelayed();
  }

  return true;
}

void Calculator::OnDisable() {
  if ( timedOut() ) {
    getStatusRequest()->signalComplete(-1);  // signal error
    Serial.print(millis()); Serial.print(":\t");
    Serial.println("MeasureCallback: ***** Timeout *****");
  }
  iS->disableAll(false); // only disable tasks in the ts scheduler
  Serial.print("Free mem = "); Serial.println(freeMemory()); Serial.println();
}

void Calculator::reportDistance(long aD) {
  if (distance < aD) distance = aD;
}

