// Cooperative multitasking library for Arduino
// Copyright (c) 2015-2019 Anatoli Arkhipenko
//
// Changelog:
// v1.0.0:
//     2015-02-24 - Initial release
//     2015-02-28 - added delay() and disableOnLastIteration() methods
//     2015-03-25 - changed scheduler execute() method for a more precise delay calculation:
//                  1. Do not delay if any of the tasks ran (making request for immediate execution redundant)
//                  2. Delay is invoked only if none of the tasks ran
//                  3. Delay is based on the min anticipated wait until next task _AND_ the runtime of execute method itself.
//     2015-05-11 - added  restart() and restartDelayed() methods to restart tasks which are on hold after running all iterations
//     2015-05-19 - completely removed  delay from the scheduler since there are no power saving there. using 1 ms sleep instead
//
// v1.4.1:
//     2015-09-15 - more careful placement of AVR-specific includes for sleep method (compatibility with DUE)
//                  sleep on idle run is no longer a default and should be explicitly compiled with
//                 _TASK_SLEEP_ON_IDLE_RUN defined
//
// v1.5.0:
//     2015-09-20 - access to currently executing task (for callback methods)
//     2015-09-20 - pass scheduler as a parameter to the task constructor to append the task to the end of the chain
//     2015-09-20 - option to create a task already enabled
//
// v1.5.1:
//     2015-09-21 - bug fix: incorrect handling of active tasks via set() and setIterations().
//                  Thanks to Hannes Morgenstern for catching this one
//
// v1.6.0:
//     2015-09-22 - revert back to having all tasks disable on last iteration.
//     2015-09-22 - deprecated disableOnLastIteration method as a result
//     2015-09-22 - created a separate branch 'disable-on-last-iteration' for this
//     2015-10-01 - made version numbers semver compliant (documentation only)
//
// v1.7.0:
//    2015-10-08 - introduced callback run counter - callback methods can branch on the iteration number.
//    2015-10-11 - enableIfNot() - enable a task only if it is not already enabled. Returns true if was already enabled,
//                 false if was disabled.
//    2015-10-11 - disable() returns previous enable state (true if was enabled, false if was already disabled)
//    2015-10-11 - introduced callback methods "on enable" and "on disable". On enable runs every time enable is called,
//                 on disable runs only if task was enabled
//    2015-10-12 - new Task method: forceNextIteration() - makes next iteration happen immediately during the next pass
//                 regardless how much time is left
//
// v1.8.0:
//    2015-10-13 - support for status request objects allowing tasks waiting on requests
//    2015-10-13 - moved to a single header file to allow compilation control via #defines from the main sketch
//
// v1.8.1:
//    2015-10-22 - implement Task id and control points to support identification of failure points for watchdog timer logging
//
// v1.8.2:
//    2015-10-27 - implement Local Task Storage Pointer (allow use of same callback code for different tasks)
//    2015-10-27 - bug: currentTask() method returns incorrect Task reference if called within OnEnable and OnDisable methods
//    2015-10-27 - protection against infinite loop in OnEnable (if enable() methods are called within OnEnable)
//    2015-10-29 - new currentLts() method in the scheduler class returns current task's LTS pointer in one call
//
// v1.8.3:
//    2015-11-05 - support for task activation on a status request with arbitrary interval and number of iterations
//                (0 and 1 are still default values)
//    2015-11-05 - implement waitForDelayed() method to allow task activation on the status request completion
//                 delayed for one current interval
//    2015-11-09 - added callback methods prototypes to all examples for Arduino IDE 1.6.6 compatibility
//    2015-11-14 - added several constants to be used as task parameters for readability (e.g, TASK_FOREVER, TASK_SECOND, etc.)
//    2015-11-14 - significant optimization of the scheduler's execute loop, including millis() rollover fix option
//
// v1.8.4:
//    2015-11-15 - bug fix: Task alignment with millis() for scheduling purposes should be done after OnEnable, not before.
//                 Especially since OnEnable method can change the interval
//    2015-11-16 - further optimizations of the task scheduler execute loop
//
// v1.8.5:
//    2015-11-23 - bug fix: incorrect calculation of next task invocation in case callback changed the interval
//    2015-11-23 - bug fix: Task::set() method calls setInterval() explicitly, therefore delaying the task in the same manner
//
// v1.9.0:
//    2015-11-24 - packed three byte-long status variables into bit array structure data type - saving 2 bytes per each task instance
//
// v1.9.2:
//    2015-11-28 - _TASK_ROLLOVER_FIX is deprecated (not necessary)
//    2015-12-16 - bug fixes: automatic millis rollover support for delay methods
//    2015-12-17 - new method for _TASK_TIMECRITICAL option: getStartDelay()
//
// v2.0.0:
//    2015-12-22 - _TASK_PRIORITY - support for layered task prioritization
//
// v2.0.1:
//    2016-01-02 - bug fix: issue#11 Xtensa compiler (esp8266): Declaration of constructor does not match implementation
//
// v2.0.2:
//    2016-01-05 - bug fix: time constants wrapped inside compile option
//    2016-01-05 - support for ESP8266 wifi power saving mode for _TASK_SLEEP_ON_IDLE_RUN compile option
//
// v2.1.0:
//    2016-02-01 - support for microsecond resolution
//    2016-02-02 - added Scheduler baseline start time reset method: startNow()
//
// v2.2.0:
//    2016-11-17 - all methods made 'inline' to support inclusion of TaskSchedule.h file into other header files
//
// v2.2.1:
//    2016-11-30 - inlined constructors. Added "yield()" and "yieldOnce()" functions to easily break down and chain
//                 back together long running callback methods
//    2016-12-16 - added "getCount()" to StatusRequest objects, made every task StatusRequest enabled.
//                 Internal StatusRequest objects are accessible via "getInternalStatusRequest()" method.
//
// v2.3.0:
//    2017-02-24 - new timeUntilNextIteration() method within Scheduler class - inquire when a particlar task is
//                 scheduled to run next time
//
// v2.4.0:
//    2017-04-27 - added destructor to the Task class to ensure tasks are disables and taken off the execution chain
//                 upon destruction. (Contributed by Edwin van Leeuwen [BlackEdder - https://github.com/BlackEdder)
//
// v2.5.0:
//    2017-04-27 - ESP8266 ONLY: added optional support for std::functions via _TASK_STD_FUNCTION compilation option
//                 (Contributed by Edwin van Leeuwen [BlackEdder - https://github.com/BlackEdder)
//    2017-08-30 - add _TASK_DEBUG making all methods and variables public FOR DEBUGGING PURPOSES ONLY!
//                 Use at your own risk!
//    2017-08-30 - bug fix: Scheduler::addTask() checks if task is already part of an execution chain (github issue #37)
//    2017-08-30 - support for multi-tab sketches (Contributed by Adam Ryczkowski - https://github.com/adamryczkowski)
//
// v2.5.1:
//    2018-01-06 - support for IDLE sleep on Teensy boards (tested on Teensy 3.5)
//
// v2.5.2:
//    2018-01-09 - _TASK_INLINE compilation directive making all methods declared "inline" (issue #42)
//
// v2.6.0:
//    2018-01-30 - _TASK_TIMEOUT compilation directive: Task overall timeout functionality
//    2018-01-30 - ESP32 support (experimental)
//                 (Contributed by Marco Tombesi: https://github.com/baggior)
//
// v2.6.1:
//    2018-02-13 - Bug: support for task self-destruction in the OnDisable method
//                 Example 19: dynamic tasks creation and destruction
//    2018-03-14 - Bug: high level scheduler ignored if lower level chain is empty
//                 Example 20: use of local task storage to work with task-specific class objects
//
// v3.0.0:
//    2018-03-15 - Major Release: Support for dynamic callback methods binding via compilation parameter _TASK_OO_CALLBACKS
//
// v3.0.1:
//    2018-11-09 - bug: task deleted from the execution chain cannot be added back (github issue #67)
//
// v3.0.2:
//    2018-11-11 - bug: default constructor is ambiguous when Status Request objects are enabled (github issue #65 & #68)
//
// v3.0.3:
//    2019-06-13 - feature: custom sleep callback method: setSleepMethod() - ability to dynamically control idle sleep for various microcontrollers
//               - feature: support for MSP430 and MSP432 boards (pull request #75: big thanks to Guillaume Pirou, https://github.com/elominp)
//               - officially discontinued support for offile documentation in favor of updating the Wiki pages
//
// v3.1.0:
//    2020-01-07 - feature: added 4 cpu load monitoring methods for _TASK_TIMECRITICAL compilation option
//
// v3.1.1:
//    2020-01-09 - update: more precise CPU load measuring. Ability to define idle sleep threshold for ESP chips
//
// v3.1.2:
//    2020-01-17 - bug fix: corrected external forward definitions of millis() and micros
// 
// v3.1.3:
//    2020-01-30 - bug fix: _TASK_DEFINE_MILLIS to force forward definition of millis and micros. Not defined by default. 
//    2020-02-16 - bug fix: add 'virtual' to the Task destructor definition (issue #86)
//
// v3.1.4:
//    2020-02-22 - bug: get rid of unnecessary compiler warnings
//    2020-02-22 - feature: access to the task chain with _TASK_EXPOSE_CHAIN compile option
//
// v3.1.5:
//    2020-05-08 - feature: implemented light sleep for esp32
//
// v3.1.6:
//    2020-05-12 - bug fix: deleteTask and addTask should check task ownership first (Issue #97)
//
// v3.1.7:
//    2020-07-07 - warning fix: unused parameter 'aRecursive' (Issue #99)
//
// v3.2.0:
//    2020-08-16 - feature: scheduling options
//
// v3.2.1:
//    2020-10-04 - feature: Task.abort method. Stop task execution without calling OnDisable(). 
//
// v3.2.2:
//    2020-12-14 - feature: enable and restart methods return true if task enabled 
//                 feature: Task.cancel() method - disable task with a cancel flag (could be used for alt. path
//                          processing in the onDisable method.
//                 feature: Task.cancelled() method - indicates that task was disabled with a cancel() method.
//
// v3.2.3:
//    2021-01-01 - feature: discontinued use of 'register' keyword. Deprecated in C++ 11 
//                 feature: add STM32 as a platform supporting _TASK_STD_FUNCTION. (PR #105)
//
// v3.3.0:
//    2021-05-11 - feature: Timeout() methods for StatusRequest objects 
//
// v3.4.0:
//    2021-07-14 - feature: ability to Enable/Disable and Pause/Resume scheduling 
//               - feature: optional use of external millis/micros methods 


#ifndef _TASKSCHEDULER_H_
#define _TASKSCHEDULER_H_

#include <Arduino.h>

#ifdef _TASK_DEFINE_MILLIS
extern "C" {
    unsigned long micros(void);
    unsigned long millis(void);
}
#endif

#include "TaskSchedulerDeclarations.h"

// ----------------------------------------
// The following "defines" control library functionality at compile time,
// and should be used in the main sketch depending on the functionality required
//
// #define _TASK_TIMECRITICAL       // Enable monitoring scheduling overruns
// #define _TASK_SLEEP_ON_IDLE_RUN  // Enable 1 ms SLEEP_IDLE powerdowns between runs if no callback methods were invoked during the pass
// #define _TASK_STATUS_REQUEST     // Compile with support for StatusRequest functionality - triggering tasks on status change events in addition to time only
// #define _TASK_WDT_IDS            // Compile with support for wdt control points and task ids
// #define _TASK_LTS_POINTER        // Compile with support for local task storage pointer
// #define _TASK_PRIORITY           // Support for layered scheduling priority
// #define _TASK_MICRO_RES          // Support for microsecond resolution
// #define _TASK_STD_FUNCTION       // Support for std::function (ESP8266 ONLY)
// #define _TASK_DEBUG              // Make all methods and variables public for debug purposes
// #define _TASK_INLINE             // Make all methods "inline" - needed to support some multi-tab, multi-file implementations
// #define _TASK_TIMEOUT            // Support for overall task timeout
// #define _TASK_OO_CALLBACKS       // Support for callbacks via inheritance
// #define _TASK_EXPOSE_CHAIN       // Methods to access tasks in the task chain
// #define _TASK_SCHEDULING_OPTIONS // Support for multiple scheduling options
// #define _TASK_DEFINE_MILLIS      // Force forward declaration of millis() and micros() "C" style
// #define _TASK_EXTERNAL_TIME      // Custom millis() and micros() methods

 #ifdef _TASK_MICRO_RES

 #undef _TASK_SLEEP_ON_IDLE_RUN     // SLEEP_ON_IDLE has only millisecond resolution
 #define _TASK_TIME_FUNCTION() _task_micros()

 #else
 #define _TASK_TIME_FUNCTION() _task_millis()

 #endif  // _TASK_MICRO_RES


#ifdef _TASK_SLEEP_ON_IDLE_RUN
#include "TaskSchedulerSleepMethods.h"

  Scheduler* iSleepScheduler;
  SleepCallback iSleepMethod;

#endif  // _TASK_SLEEP_ON_IDLE_RUN


#if !defined (ARDUINO_ARCH_ESP8266) && !defined (ARDUINO_ARCH_ESP32) && !defined (ARDUINO_ARCH_STM32)
#ifdef _TASK_STD_FUNCTION
    #error Support for std::function only for ESP8266 or ESP32 architecture
#undef _TASK_STD_FUNCTION
#endif // _TASK_STD_FUNCTION
#endif // ARDUINO_ARCH_ESP8266

#ifdef _TASK_WDT_IDS
    static unsigned int __task_id_counter = 0; // global task ID counter for assigning task IDs automatically.
#endif  // _TASK_WDT_IDS

#ifdef _TASK_PRIORITY
    Scheduler* iCurrentScheduler;
#endif // _TASK_PRIORITY


#endif /* _TASKSCHEDULER_H_ */
