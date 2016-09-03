//This is the place to declare every single function
//and global variable that is going to be reused between cpp files.


//We are going to use the TaskScheduler, but only the declarations part.
//Remember to put customization macros before the #include:
#define _TASK_SLEEP_ON_IDLE_RUN
#include <TaskSchedulerDeclarations.h>

//Let the runner object be a global, single instance shared between object files.
extern Scheduler runner;
extern Task t2; //the t2 is defined in file2, but we need to access it from file1.

//This function needs to be shared (between file2 and file1).
void t3Callback();
