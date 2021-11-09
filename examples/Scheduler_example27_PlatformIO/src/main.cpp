/*
   Every example set must have a LED blink example
   For this one the idea is to have as many ways to blink the LED
   as I can think of. So, here we go.

   Tested on:
   - Arduino Nano
   - ESP8266
   - ESP32
   - STM32 Maple Mini
*/

#include <TaskSchedulerDeclarations.h>

#include "main.h"
#include "led.h"

// Scheduler
Scheduler ts;

/*
   Approach 1: LED is driven by the boolean variable; false = OFF, true = ON
*/
void blink1CB();
Task tBlink1 ( PERIOD1 * TASK_MILLISECOND, DURATION / PERIOD1, &blink1CB, &ts, true );

/*
   Approach 2: two callback methods: one turns ON, another turns OFF
*/
void blink2CB_ON();
void blink2CB_OFF();
Task tBlink2 ( PERIOD2 * TASK_MILLISECOND, DURATION / PERIOD2, &blink2CB_ON, &ts, false );

/*
   Approach 3: Use RunCounter
*/
void blink3CB();
Task tBlink3 (PERIOD3 * TASK_MILLISECOND, DURATION / PERIOD3, &blink3CB, &ts, false);

/*
   Approach 4: Use status request objects to pass control from one task to the other
*/
bool blink41OE();
void blink41();
void blink42();
void blink42OD();
Task tBlink4On  ( PERIOD4 * TASK_MILLISECOND, TASK_ONCE, blink41, &ts, false, &blink41OE );
Task tBlink4Off ( PERIOD4 * TASK_MILLISECOND, TASK_ONCE, blink42, &ts, false, NULL, &blink42OD );


/*
   Approach 5: Two interleaving tasks
*/
bool blink51OE();
void blink51();
void blink52();
void blink52OD();
Task tBlink5On  ( 600 * TASK_MILLISECOND, DURATION / PERIOD5, &blink51, &ts, false, &blink51OE );
Task tBlink5Off ( 600 * TASK_MILLISECOND, DURATION / PERIOD5, &blink52, &ts, false, NULL, &blink52OD );


/*
   Approach 6: RunCounter-based with random intervals
*/
void blink6CB();
bool blink6OE();
void blink6OD();
Task tBlink6 ( PERIOD6 * TASK_MILLISECOND, DURATION / PERIOD6, &blink6CB, &ts, false, &blink6OE, &blink6OD );

void setup() {
  // put your setup code here, to run once:
#if defined(_DEBUG_) || defined(_TEST_)
  Serial.begin(115200);
  delay(TASK_SECOND);
  _PL("TaskScheduler Blink example");
  _PL("Blinking for 10 seconds using various techniques\n");
  delay(2 * TASK_SECOND);
#endif
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  ts.execute();
}

// === 1 =======================================
bool LED_state = false;
void blink1CB() {
  if ( tBlink1.isFirstIteration() ) {
    _PP(millis());
    _PL(": Blink1 - simple flag driven");
    LED_state = false;
  }

  if ( LED_state ) {
    LEDOff();
    LED_state = false;
  }
  else {
    LEDOn();
    LED_state = true;
  }

  if ( tBlink1.isLastIteration() ) {
    tBlink2.restartDelayed( 2 * TASK_SECOND );
    LEDOff();
  }
}


// === 2 ======================================
void blink2CB_ON() {
  if ( tBlink2.isFirstIteration() ) {
    _PP(millis());
    _PL(": Blink2 - 2 callback methods");
  }

  LEDOn();
  tBlink2.setCallback( &blink2CB_OFF );

  if ( tBlink2.isLastIteration() ) {
    tBlink3.restartDelayed( 2 * TASK_SECOND );
    LEDOff();
  }
}


void blink2CB_OFF() {

  LEDOff();
  tBlink2.setCallback( &blink2CB_ON );

  if ( tBlink2.isLastIteration() ) {
    tBlink3.restartDelayed( 2 * TASK_SECOND );
    LEDOff();
  }
}


// === 3 =====================================
void blink3CB() {
  if ( tBlink3.isFirstIteration() ) {
    _PP(millis());
    _PL(": Blink3 - Run Counter driven");
  }

  if ( tBlink3.getRunCounter() & 1 ) {
    LEDOn();
  }
  else {
    LEDOff();
  }

  if ( tBlink3.isLastIteration() ) {
    tBlink4On.setOnEnable( &blink41OE );
    tBlink4On.restartDelayed( 2 * TASK_SECOND );
    LEDOff();
  }
}


// === 4 =============================================
int counter = 0;
bool blink41OE() {
  _PP(millis());
  _PL(": Blink4 - Internal status request based");
  counter = 0;
  tBlink4On.setOnEnable( NULL );
  return true;
}

void blink41() {
  //  _PP(millis());
  //  _PL(": blink41");
  LEDOn();
  StatusRequest* r = tBlink4On.getInternalStatusRequest();
  tBlink4Off.waitForDelayed( r );
  counter++;
}

void blink42() {
  //  _PP(millis());
  //  _PL(": blink42");
  LEDOff();
  StatusRequest* r = tBlink4Off.getInternalStatusRequest();
  tBlink4On.waitForDelayed( r );
  counter++;
}


void blink42OD() {
  if ( counter >= DURATION / PERIOD4 ) {
    tBlink4On.disable();
    tBlink4Off.disable();

    tBlink5On.setOnEnable( &blink51OE );
    tBlink5On.restartDelayed( 2 * TASK_SECOND );
    tBlink5Off.restartDelayed( 2 * TASK_SECOND + PERIOD5 / 2 );
    LEDOff();
  }
}


// === 5 ==========================================
bool blink51OE() {
  _PP(millis());
  _PL(": Blink5 - Two interleaving tasks");
  tBlink5On.setOnEnable( NULL );
  return true;
}
void blink51() {
  //  _PP(millis());
  //  _PL(": blink51");
  LEDOn();
}
void blink52() {
  //  _PP(millis());
  //  _PL(": blink52");
  LEDOff();
}
void blink52OD() {
  tBlink6.restartDelayed( 2 * TASK_SECOND );
  LEDOff();
}


// === 6 ============================================
long interval6 = 0;
bool blink6OE() {
  _PP(millis());
  _PP(": Blink6 - RunCounter + Random ON interval = ");
  interval6 = random( 100, 901 );
  tBlink6.setInterval( interval6 );
  _PL( interval6 );
  tBlink6.delay( 2 * TASK_SECOND );

  return true;
}

void blink6CB() {
  if ( tBlink6.getRunCounter() & 1 ) {
    LEDOn();
    tBlink6.setInterval( interval6 );
  }
  else {
    LEDOff();
    tBlink6.setInterval( TASK_SECOND - interval6 );
  }
}

void blink6OD() {
  tBlink1.restartDelayed( 2 * TASK_SECOND );
  LEDOff();
}
