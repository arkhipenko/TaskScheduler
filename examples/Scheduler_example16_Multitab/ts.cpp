//This is the only .cpp file that gets the #include<TaskScheduler.h>.
//Without it, the linker would not find necessary TaskScheduler's compiled code.
//
//Remember to put customization macros here as well.
//
//And don't import any common headers (here: header.hpp)
//
//Really. This file needs to be short. All stuff is in TaskScheduler.h.

//  #define _TASK_TIMECRITICAL      // Enable monitoring scheduling overruns
#define _TASK_SLEEP_ON_IDLE_RUN // Enable 1 ms SLEEP_IDLE powerdowns between tasks if no callback methods were invoked during the pass 
//  #define _TASK_STATUS_REQUEST    // Compile with support for StatusRequest functionality - triggering tasks on status change events in addition to time only
//  #define _TASK_WDT_IDS           // Compile with support for wdt control points and task ids
//  #define _TASK_LTS_POINTER       // Compile with support for local task storage pointer
//  #define _TASK_PRIORITY          // Support for layered scheduling priority
//  #define _TASK_MICRO_RES         // Support for microsecond resolution
//  #define _TASK_STD_FUNCTION      // Support for std::function (ESP8266 ONLY)
//  #define _TASK_DEBUG             // Make all methods and variables public for debug purposes

#include <TaskScheduler.h>
