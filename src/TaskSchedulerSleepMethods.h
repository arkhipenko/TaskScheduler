// Cooperative multitasking library for Arduino
// Copyright (c) 2015-2019 Anatoli Arkhipenko

#ifndef _TASKSCHEDULERSLEEPMETHODS_H_
#define _TASKSCHEDULERSLEEPMETHODS_H_

		
#if defined( ARDUINO_ARCH_AVR ) // Could be used only for AVR-based boards.

#include <avr/sleep.h>
#include <avr/power.h>

void SleepMethod( unsigned long aDuration ) {
  set_sleep_mode(SLEEP_MODE_IDLE);
  sleep_enable();
  /* Now enter sleep mode. */
  sleep_mode();

  /* The program will continue from here after the timer timeout ~1 ms */
  sleep_disable(); /* First thing to do is disable sleep. */
}
// ARDUINO_ARCH_AVR

#else
void SleepMethod( unsigned long aDuration ) {
}

#endif //  SLEEP METHODS

#endif // _TASKSCHEDULERSLEEPMETHODS_H_