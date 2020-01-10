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


#elif defined( CORE_TEENSY )
void SleepMethod( unsigned long aDuration ) {
  asm("wfi");
}
//CORE_TEENSY


#elif defined( ARDUINO_ARCH_ESP8266 )

#ifndef _TASK_ESP8266_DLY_THRESHOLD
#define _TASK_ESP8266_DLY_THRESHOLD 200L
#endif
extern "C" {
#include "user_interface.h"
}

void SleepMethod( unsigned long aDuration ) {
// to do: find suitable sleep function for esp8266
      if ( aDuration < _TASK_ESP8266_DLY_THRESHOLD) delay(1);   // ESP8266 implementation of delay() uses timers and yield
}
// ARDUINO_ARCH_ESP8266


#elif defined( ARDUINO_ARCH_ESP32 )

#ifndef _TASK_ESP32_DLY_THRESHOLD
#define _TASK_ESP32_DLY_THRESHOLD 200L
#endif
#warning _TASK_SLEEP_ON_IDLE_RUN for ESP32 cannot use light sleep mode but a standard delay for 1 ms
extern unsigned long tStart, tFinish;

void SleepMethod( unsigned long aDuration ) {
//TODO: find a correct light sleep implementation for ESP32
    // esp_sleep_enable_timer_wakeup(1000); //1 ms
    // int ret= esp_light_sleep_start();
      if ( aDuration < _TASK_ESP32_DLY_THRESHOLD) delay(1);   // ESP8266 implementation of delay() uses timers and yield

}
// ARDUINO_ARCH_ESP32


#elif defined( ARDUINO_ARCH_STM32F1 )

#include <libmaple/pwr.h>
#include <libmaple/scb.h>

void SleepMethod( unsigned long aDuration ) {
	  // Now go into stop mode, wake up on interrupt.
	  // Systick interrupt will run every 1 milliseconds.
	  asm("    wfi");
}
// ARDUINO_ARCH_STM32


#elif defined( ENERGIA_ARCH_MSP432 )

void SleepMethod( unsigned long aDuration ) {
    delay(1);
}
// ENERGIA_ARCH_MSP432


#elif defined( ENERGIA_ARCH_MSP430 )

void SleepMethod( unsigned long aDuration ) {
    sleep(1);
}
// ENERGIA_ARCH_MSP430


#else
void SleepMethod( unsigned long aDuration ) {
}

#endif //  SLEEP METHODS

#endif // _TASKSCHEDULERSLEEPMETHODS_H_