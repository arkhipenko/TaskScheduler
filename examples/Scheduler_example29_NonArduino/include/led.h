#ifndef _TS27_LED_H
#define _TS27_LED_H

// LED_BUILTIN  13
#if defined( ARDUINO_ARCH_ESP32 )
#define LED_BUILTIN  23 // esp32 dev2 kit does not have LED
#endif

void LEDOff();
void LEDOn();

#endif //   _TS27_LED_H