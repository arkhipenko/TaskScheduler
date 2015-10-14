#define _TASK_SLEEP_ON_IDLE_RUN
#include <TaskScheduler.h>

#define LEDPIN	13

Scheduler ts;

Task tWrapper(30000L, -1, &WrapperCallback, &ts, true);
Task tBlink(5000, 1, NULL, &ts, false, &BlinkOnEnable, &BlinkOnDisable);
Task tLED(0, -1, NULL, &ts, false, NULL, &LEDOff);

void WrapperCallback() {
	tBlink.restartDelayed(); // LED blinking is initiated
							 //every 30 seconds for 5 seconds
}


// Upon being enabled, tBlink will define the parameters
// and enable LED blinking task, which actually controls
// the hardware (LED in this example)
bool BlinkOnEnable() {
	tLED.setInterval( 500 + random(501) );
	tLED.setCallback( &LEDOn);
	tLED.enable();

	return true; // Task should be enabled
}

// tBlink does not really need a callback function
// since it just waits for 5 seconds for the first
// and only iteration to occur. Once the iteration
// takes place, tBlink is disabled by the Scheduler,
// thus executing its OnDisable method below.

void BlinkOnDisable() {
	tLED.disable();
}

void LEDOn () {
	digitalWrite(LEDPIN, HIGH);
	tLED.setCallback( &LEDOff);
}

void LEDOff () {
	digitalWrite(LEDPIN, LOW);
	tLED.setCallback( &LEDOn);
}

// Note that LEDOff method serves as OnDisable method
// to make sure the LED is turned off when the tBlink
// task finishes (or disabled ahead of time)

void setup() {
// put your setup code here, to run once:
}

void loop() {
// put your main code here, to run repeatedly:
	ts.execute();
}