#include "SuperSensor.h"

SuperSensor::SuperSensor(int aPin) {
  iPin = aPin;
}

SuperSensor::~SuperSensor() {
  iValue = -1;
}

void SuperSensor::begin() {
  iDelay = random(300, 1500);
  iValue = -1;
}

void SuperSensor::stop() {
  //nothing to do
}

long SuperSensor::trigger() {
  iStart = millis();
  return iDelay;
}

bool SuperSensor::measurementReady() {
  if ( millis() - iStart > iDelay ) {
    iValue = random(501);
    return true;
  }
  return false;
}

long SuperSensor::value() {
  return iValue;
}

