#include <Arduino.h>
#include "header.hpp"


//Declare the functions we want to use before we are ready to define them
void t1Callback();


// Tasks
Task t1(2000, 10, &t1Callback, &runner, true);  //adding task to the chain on creation
Task t3(5000, TASK_FOREVER, &t3Callback);


void t1Callback() {
    Serial.print("t1: ");
    Serial.println(millis());

    if (t1.isFirstIteration()) {
      runner.addTask(t3);
      t3.enable();
      Serial.println("t1: enabled t3 and added to the chain");
    }

    if (t1.isLastIteration()) {
      t3.disable();
      runner.deleteTask(t3);
      t2.setInterval(500);
      Serial.println("t1: disable t3 and delete it from the chain. t2 interval set to 500");
    }
}

