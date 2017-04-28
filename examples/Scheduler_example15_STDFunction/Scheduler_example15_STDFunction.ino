/** 
 * TaskScheduler Test sketch - Showing how to use std::function
 * to get acces to variables from within the task callback function 
 */
#define _TASK_STD_FUNCTION   // Compile with support for std::function 
#include <TaskScheduler.h>

Scheduler ts;

class Calculator {
public:
    size_t cumSum = 0; // cumulative sum
    Calculator(size_t b) {
        // Pass the this pointer, so that we get access to this->cumSum
        // Also pass a copy of b
        calculateTask.set(TASK_SECOND, TASK_FOREVER, [this, b]() {
            Serial.printf("cumSum = %u + %u\n", cumSum, b);
            cumSum += b;
            Serial.printf("Resulting cumulative sum: %u\n", cumSum);
        });
        ts.addTask(calculateTask);
        calculateTask.enable();
    }

    Task calculateTask;
};

Calculator calc1(2);
Calculator calc2(4);
Calculator calc3(8);

// Disable tasks after 10 seconds
Task tWrapper(10*TASK_SECOND, TASK_ONCE, []() {
    ts.disableAll();
}, &ts); 

/**
 * Standard Arduino setup and loop methods
 */
void setup() {
  Serial.begin(115200);
  Serial.println("std::function test");
  tWrapper.enableDelayed();
}

void loop() {
  ts.execute();
}
