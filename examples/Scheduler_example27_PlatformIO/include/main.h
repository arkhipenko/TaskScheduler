#ifndef _TS27_MAIN_H
#define _TS27_MAIN_H

#include <Arduino.h>

#ifdef _DEBUG_
#define _PP(a) Serial.print(a);
#define _PL(a) Serial.println(a);
#else
#define _PP(a)
#define _PL(a)
#endif

#define PERIOD1 500
#define DURATION 10000

#define PERIOD2 400

#define PERIOD3 300

#define PERIOD4 200

#define PERIOD5 600

#define PERIOD6 300

#endif