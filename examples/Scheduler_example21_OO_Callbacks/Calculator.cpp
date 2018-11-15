#include "Calculator.h"
#include "SuperSensor.h"

#if defined (ARDUINO_ARCH_AVR)
#include <MemoryFree.h>
#endif

#if defined(__arm__)
extern "C" char* sbrk(int incr);
static int freeMemory() {
  char top = 't';
  return &top - reinterpret_cast<char*>(sbrk(0));
}

#endif
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
#if defined (ARDUINO_ARCH_AVR) || defined(__arm__)
  Serial.print("Free mem = "); Serial.println(freeMemory()); Serial.println();
#endif
}

void Calculator::reportDistance(long aD) {
  if (distance < aD) distance = aD;
}

