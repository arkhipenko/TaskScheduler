#include "Ticker.h"

Ticker::Ticker(Scheduler* aS, Task* aCalc, StatusRequest* aM) : Task(10000, TASK_FOREVER, aS, false) {
  iCalc = aCalc;
  iMeasure = aM;
}

bool Ticker::Callback() {
  Serial.println(); Serial.println(); Serial.println();
  Serial.print(millis()); Serial.print(":\t");
  Serial.println("CycleCallback: Initiating measurement cycle every 10 seconds");

  int numberSensors = random(1, 11); // 1 to 10 sensors, randomly
  Serial.print("Number of sensors=");
  Serial.println(numberSensors);

  iMeasure->setWaiting(numberSensors); // Set the StatusRequest to wait for 1 to 10 signals.
  iCalc->waitFor(iMeasure);
}

