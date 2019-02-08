// Test the same as example#2:
// Initially only tasks 1 and 2 are enabled
// Task1 runs every 2 seconds 10 times and then stops
// Task2 runs every 3 seconds indefinitely
// Task1 enables Task3 at its first run
// Task3 run every 5 seconds
// loop() runs every 1 second (a default scheduler delay, if no shorter tasks' interval is detected)
// Task1 disables Task3 on its last iteration and changed Task2 to run every 1/2 seconds
// Because Task2 interval is shorter than Scheduler default tick, loop() executes ecery 1/2 seconds now
// At the end Task2 is the only task running every 1/2 seconds


//Header that declares all shared objects between .cpp files
#include "header.hpp"

#include <Arduino.h> //for Serial and delay

Scheduler runner; //Let the scheduler live here, in the main file, ok?


//Pretend, that the t2 task is a special task,
//that needs to live in file2 object file.
void t2Callback() {
    Serial.print("t2: ");
    Serial.println(millis());
}
Task t2(3000, TASK_FOREVER, &t2Callback, &runner, true);

//Lets define t3Callback here. We are going to use it in file1
//for Task 1.
void t3Callback() {
    Serial.print("t3: ");
    Serial.println(millis());
}






void setup () {
  Serial.begin(115200);
  delay(5000);
  Serial.println("Scheduler TEST (multi-tab)");

  runner.startNow();  // set point-in-time for scheduling start
}


void loop () {
  runner.execute();

//  Serial.println("Loop ticks at: ");
//  Serial.println(millis());
}
