/*
Cooperative multitasking library for Arduino
Copyright (c) 2015-2019 Anatoli Arkhipenko

Changelog:
v1.0.0:
    2015-02-24 - Initial release
    2015-02-28 - added delay() and disableOnLastIteration() methods
    2015-03-25 - changed scheduler execute() method for a more precise delay calculation:
                 1. Do not delay if any of the tasks ran (making request for immediate execution redundant)
                 2. Delay is invoked only if none of the tasks ran
                 3. Delay is based on the min anticipated wait until next task _AND_ the runtime of execute method itself.
    2015-05-11 - added  restart() and restartDelayed() methods to restart tasks which are on hold after running all iterations
    2015-05-19 - completely removed  delay from the scheduler since there are no power saving there. using 1 ms sleep instead

v1.4.1:
    2015-09-15 - more careful placement of AVR-specific includes for sleep method (compatibility with DUE)
                 sleep on idle run is no longer a default and should be explicitly compiled with
                _TASK_SLEEP_ON_IDLE_RUN defined

v1.5.0:
    2015-09-20 - access to currently executing task (for callback methods)
    2015-09-20 - pass scheduler as a parameter to the task constructor to append the task to the end of the chain
    2015-09-20 - option to create a task already enabled

v1.5.1:
    2015-09-21 - bug fix: incorrect handling of active tasks via set() and setIterations().
                 Thanks to Hannes Morgenstern for catching this one

v1.6.0:
    2015-09-22 - revert back to having all tasks disable on last iteration.
    2015-09-22 - deprecated disableOnLastIteration method as a result
    2015-09-22 - created a separate branch 'disable-on-last-iteration' for this
    2015-10-01 - made version numbers semver compliant (documentation only)

v1.7.0:
   2015-10-08 - introduced callback run counter - callback methods can branch on the iteration number.
   2015-10-11 - enableIfNot() - enable a task only if it is not already enabled. Returns true if was already enabled,
                false if was disabled.
   2015-10-11 - disable() returns previous enable state (true if was enabled, false if was already disabled)
   2015-10-11 - introduced callback methods "on enable" and "on disable". On enable runs every time enable is called,
                on disable runs only if task was enabled
   2015-10-12 - new Task method: forceNextIteration() - makes next iteration happen immediately during the next pass
                regardless how much time is left

v1.8.0:
   2015-10-13 - support for status request objects allowing tasks waiting on requests
   2015-10-13 - moved to a single header file to allow compilation control via #defines from the main sketch

v1.8.1:
   2015-10-22 - implement Task id and control points to support identification of failure points for watchdog timer logging

v1.8.2:
   2015-10-27 - implement Local Task Storage Pointer (allow use of same callback code for different tasks)
   2015-10-27 - bug: currentTask() method returns incorrect Task reference if called within OnEnable and OnDisable methods
   2015-10-27 - protection against infinite loop in OnEnable (if enable() methods are called within OnEnable)
   2015-10-29 - new currentLts() method in the scheduler class returns current task's LTS pointer in one call

v1.8.3:
   2015-11-05 - support for task activation on a status request with arbitrary interval and number of iterations
               (0 and 1 are still default values)
   2015-11-05 - implement waitForDelayed() method to allow task activation on the status request completion
                delayed for one current interval
   2015-11-09 - added callback methods prototypes to all examples for Arduino IDE 1.6.6 compatibility
   2015-11-14 - added several constants to be used as task parameters for readability (e.g, TASK_FOREVER, TASK_SECOND, etc.)
   2015-11-14 - significant optimization of the scheduler's execute loop, including millis() rollover fix option

v1.8.4:
   2015-11-15 - bug fix: Task alignment with millis() for scheduling purposes should be done after OnEnable, not before.
                Especially since OnEnable method can change the interval
   2015-11-16 - further optimizations of the task scheduler execute loop

v1.8.5:
   2015-11-23 - bug fix: incorrect calculation of next task invocation in case callback changed the interval
   2015-11-23 - bug fix: Task::set() method calls setInterval() explicitly, therefore delaying the task in the same manner

v1.9.0:
   2015-11-24 - packed three byte-long status variables into bit array structure data type - saving 2 bytes per each task instance

v1.9.2:
   2015-11-28 - _TASK_ROLLOVER_FIX is deprecated (not necessary)
   2015-12-16 - bug fixes: automatic millis rollover support for delay methods
   2015-12-17 - new method for _TASK_TIMECRITICAL option: getStartDelay()

v2.0.0:
   2015-12-22 - _TASK_PRIORITY - support for layered task prioritization

v2.0.1:
   2016-01-02 - bug fix: issue#11 Xtensa compiler (esp8266): Declaration of constructor does not match implementation

v2.0.2:
   2016-01-05 - bug fix: time constants wrapped inside compile option
   2016-01-05 - support for ESP8266 wifi power saving mode for _TASK_SLEEP_ON_IDLE_RUN compile option

v2.1.0:
   2016-02-01 - support for microsecond resolution
   2016-02-02 - added Scheduler baseline start time reset method: startNow()

v2.2.0:
   2016-11-17 - all methods made 'inline' to support inclusion of TaskSchedule.h file into other header files

v2.2.1:
   2016-11-30 - inlined constructors. Added "yield()" and "yieldOnce()" functions to easily break down and chain
                back together long running callback methods
   2016-12-16 - added "getCount()" to StatusRequest objects, made every task StatusRequest enabled.
                Internal StatusRequest objects are accessible via "getInternalStatusRequest()" method.

v2.3.0:
   2017-02-24 - new timeUntilNextIteration() method within Scheduler class - inquire when a particlar task is
                scheduled to run next time

v2.4.0:
   2017-04-27 - added destructor to the Task class to ensure tasks are disables and taken off the execution chain
                upon destruction. (Contributed by Edwin van Leeuwen [BlackEdder - https://github.com/BlackEdder)

v2.5.0:
   2017-04-27 - ESP8266 ONLY: added optional support for std::functions via _TASK_STD_FUNCTION compilation option
                (Contributed by Edwin van Leeuwen [BlackEdder - https://github.com/BlackEdder)
   2017-08-30 - add _TASK_DEBUG making all methods and variables public FOR DEBUGGING PURPOSES ONLY!
                Use at your own risk!
   2017-08-30 - bug fix: Scheduler::addTask() checks if task is already part of an execution chain (github issue #37)
   2017-08-30 - support for multi-tab sketches (Contributed by Adam Ryczkowski - https://github.com/adamryczkowski)

v2.5.1:
   2018-01-06 - support for IDLE sleep on Teensy boards (tested on Teensy 3.5)

v2.5.2:
   2018-01-09 - _TASK_INLINE compilation directive making all methods declared "inline" (issue #42)

v2.6.0:
   2018-01-30 - _TASK_TIMEOUT compilation directive: Task overall timeout functionality
   2018-01-30 - ESP32 support (experimental)
                (Contributed by Marco Tombesi: https://github.com/baggior)

v2.6.1:
   2018-02-13 - Bug: support for task self-destruction in the OnDisable method
                Example 19: dynamic tasks creation and destruction
   2018-03-14 - Bug: high level scheduler ignored if lower level chain is empty
                Example 20: use of local task storage to work with task-specific class objects

v3.0.0:
   2018-03-15 - Major Release: Support for dynamic callback methods binding via compilation parameter _TASK_OO_CALLBACKS

v3.0.1:
   2018-11-09 - bug: task deleted from the execution chain cannot be added back (github issue #67)

v3.0.2:
   2018-11-11 - bug: default constructor is ambiguous when Status Request objects are enabled (github issue #65 & #68)

v3.0.3:
   2019-06-13 - feature: custom sleep callback method: setSleepMethod() - ability to dynamically control idle sleep for various microcontrollers
              - feature: support for MSP430 and MSP432 boards (pull request #75: big thanks to Guillaume Pirou, https://github.com/elominp)
              - officially discontinued support for offile documentation in favor of updating the Wiki pages

v3.1.0:
   2020-01-07 - feature: added 4 cpu load monitoring methods for _TASK_TIMECRITICAL compilation option

v3.1.1:
   2020-01-09 - update: more precise CPU load measuring. Ability to define idle sleep threshold for ESP chips

v3.1.2:
   2020-01-17 - bug fix: corrected external forward definitions of millis() and micros

v3.1.3:
   2020-01-30 - bug fix: _TASK_DEFINE_MILLIS to force forward definition of millis and micros. Not defined by default. 
   2020-02-16 - bug fix: add 'virtual' to the Task destructor definition (issue #86)

v3.1.4:
   2020-02-22 - bug: get rid of unnecessary compiler warnings
   2020-02-22 - feature: access to the task chain with _TASK_EXPOSE_CHAIN compile option

v3.1.5:
   2020-05-08 - feature: implemented light sleep for esp32

v3.1.6:
   2020-05-12 - bug fix: deleteTask and addTask should check task ownership first (Issue #97)

v3.1.7:
   2020-07-07 - warning fix: unused parameter 'aRecursive' (Issue #99)

v3.2.0:
   2020-08-16 - feature: scheduling options

v3.2.1:
   2020-10-04 - feature: Task.abort method. Stop task execution without calling OnDisable(). 

v3.2.2:
   2020-12-14 - feature: enable and restart methods return true if task enabled 
                feature: Task.cancel() method - disable task with a cancel flag (could be used for alt. path
                         processing in the onDisable method.
                feature: Task.cancelled() method - indicates that task was disabled with a cancel() method.

v3.2.3:
   2021-01-01 - feature: discontinued use of 'register' keyword. Depricated in C++ 11 
                feature: add STM32 as a platform supporting _TASK_STD_FUNCTION. (PR #105)

v3.3.0:
   2021-05-11 - feature: Timeout() methods for StatusRequest objects 

v3.4.0:
   2021-07-14 - feature: ability to Enable/Disable and Pause/Resume scheduling 
              - feature: optional use of external millis/micros methods 

v3.5.0:
   2021-11-01 - feature: adjust(long aInterval) method - adjust execution schedule: 
                + aInterval - shift schedule forward (later)
                - aInterval - shift schedule backwards (earlier)

v3.6.0:
   2021-11-01 - feature: _TASK_THREAD_SAFE compile option for multi-core systems or running under RTOS 


*/


#include <Arduino.h>

#ifdef _TASK_DEFINE_MILLIS
extern "C" {
    unsigned long micros(void);
    unsigned long millis(void);
}
#endif

#include "TaskSchedulerDeclarations.h"

#ifndef _TASKSCHEDULER_H_
#define _TASKSCHEDULER_H_

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
// #define _TASK_THREAD_SAFE        // Enable additional checking for thread safety

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
    static unsigned int __task_id_counter = 0; // global task ID counter for assiging task IDs automatically.
#endif  // _TASK_WDT_IDS

#ifdef _TASK_PRIORITY
    Scheduler* iCurrentScheduler;
#endif // _TASK_PRIORITY


// ------------------ TaskScheduler implementation --------------------

#ifndef _TASK_EXTERNAL_TIME
static uint32_t _task_millis() {return millis();}
static uint32_t _task_micros() {return micros();}
#endif  //  _TASK_EXTERNAL_TIME

/** Constructor, uses default values for the parameters
 * so could be called with no parameters.
 */
#ifdef _TASK_OO_CALLBACKS
Task::Task( unsigned long aInterval, long aIterations, Scheduler* aScheduler, bool aEnable ) {
    reset();
    set(aInterval, aIterations);
#else
Task::Task( unsigned long aInterval, long aIterations, TaskCallback aCallback, Scheduler* aScheduler, bool aEnable, TaskOnEnable aOnEnable, TaskOnDisable aOnDisable ) {
    reset();
    set(aInterval, aIterations, aCallback, aOnEnable, aOnDisable);
#endif

    if (aScheduler) aScheduler->addTask(*this);

#ifdef _TASK_WDT_IDS
    iTaskID = ++__task_id_counter;
#endif  // _TASK_WDT_IDS

    if (aEnable) enable();
}

/** Destructor.
 *  Makes sure the task disabled and deleted out of the chain
 *  prior to being deleted.
 */
Task::~Task() {
    disable();
    if (iScheduler)
        iScheduler->deleteTask(*this);
}


#ifdef _TASK_STATUS_REQUEST

/** Constructor with reduced parameter list for tasks created for
 *  StatusRequest only triggering (always immediate and only 1 iteration)
 */


#ifdef _TASK_OO_CALLBACKS
Task::Task( Scheduler* aScheduler ) {
    reset();
    set(TASK_IMMEDIATE, TASK_ONCE);
#else
Task::Task( TaskCallback aCallback, Scheduler* aScheduler, TaskOnEnable aOnEnable, TaskOnDisable aOnDisable ) {
    reset();
    set(TASK_IMMEDIATE, TASK_ONCE, aCallback, aOnEnable, aOnDisable);
#endif // _TASK_OO_CALLBACKS

    if (aScheduler) aScheduler->addTask(*this);

#ifdef _TASK_WDT_IDS
    iTaskID = ++__task_id_counter;
#endif  // _TASK_WDT_IDS
}


StatusRequest::StatusRequest()
{
    iCount = 0;
    iStatus = 0;
}

void StatusRequest::setWaiting(unsigned int aCount) { 
  iCount = aCount; 
  iStatus = 0; 
#ifdef _TASK_TIMEOUT
  iStarttime = _TASK_TIME_FUNCTION();
#endif  //  #ifdef _TASK_TIMEOUT
}

bool StatusRequest::pending() { return (iCount != 0); }
bool StatusRequest::completed() { return (iCount == 0); }
int StatusRequest::getStatus() { return iStatus; }
int StatusRequest::getCount() { return iCount; }
StatusRequest* Task::getStatusRequest() { return iStatusRequest; }
StatusRequest* Task::getInternalStatusRequest() { return &iMyStatusRequest; }

/** Signals completion of the StatusRequest by one of the participating events
 *  @param: aStatus - if provided, sets the return code of the StatusRequest: negative = error, 0 (default) = OK, positive = OK with a specific status code
 *  Negative status will complete Status Request fully (since an error occured).
 *  @return: true, if StatusRequest is complete, false otherwise (still waiting for other events)
 */
bool StatusRequest::signal(int aStatus) {
    if ( iCount) {  // do not update the status request if it was already completed
        if (iCount > 0)  --iCount;
        if ( (iStatus = aStatus) < 0 ) iCount = 0;   // if an error is reported, the status is requested to be completed immediately
    }
    return (iCount == 0);
}

void StatusRequest::signalComplete(int aStatus) {
    if (iCount) { // do not update the status request if it was already completed
        iCount = 0;
        iStatus = aStatus;
    }
}

/** Sets a Task to wait until a particular event completes
 *  @param: aStatusRequest - a pointer for the StatusRequest to wait for.
 *  If aStatusRequest is NULL, request for waiting is ignored, and the waiting task is not enabled.
 */
bool Task::waitFor(StatusRequest* aStatusRequest, unsigned long aInterval, long aIterations) {
#ifdef _TASK_THREAD_SAFE
    iMutex++;
#endif  // _TASK_THREAD_SAFE

    iStatusRequest = aStatusRequest;
    if ( iStatusRequest != NULL ) { // assign internal StatusRequest var and check if it is not NULL
        setIterations(aIterations);
        setInterval(aInterval);
        iStatus.waiting = _TASK_SR_NODELAY;  // no delay
        
#ifdef _TASK_THREAD_SAFE
        iMutex--;
#endif  // _TASK_THREAD_SAFE
        
        return enable();
    }
#ifdef _TASK_THREAD_SAFE
    iMutex--;
#endif  // _TASK_THREAD_SAFE

    return false;
}

bool Task::waitForDelayed(StatusRequest* aStatusRequest, unsigned long aInterval, long aIterations) {
#ifdef _TASK_THREAD_SAFE
    iMutex++;
#endif  // _TASK_THREAD_SAFE

    iStatusRequest = aStatusRequest;
    if ( iStatusRequest != NULL ) { // assign internal StatusRequest var and check if it is not NULL
        setIterations(aIterations);
        if ( aInterval ) setInterval(aInterval);  // For the dealyed version only set the interval if it was not a zero
        iStatus.waiting = _TASK_SR_DELAY;  // with delay equal to the current interval
#ifdef _TASK_THREAD_SAFE
        iMutex--;
#endif  // _TASK_THREAD_SAFE
        return enable();
    }
#ifdef _TASK_THREAD_SAFE
    iMutex--;
#endif  // _TASK_THREAD_SAFE

    return false;
}

#ifdef _TASK_TIMEOUT
void StatusRequest::resetTimeout() {
    iStarttime = _TASK_TIME_FUNCTION();
}

long StatusRequest::untilTimeout() {
    if ( iTimeout ) {
        return ( (long) (iStarttime + iTimeout) - (long) _TASK_TIME_FUNCTION() );
    }
    return -1;
}
#endif  // _TASK_TIMEOUT
#endif  // _TASK_STATUS_REQUEST

bool Task::isEnabled() { return iStatus.enabled; }

unsigned long Task::getInterval() { return iInterval; }

long Task::getIterations() { return iIterations; }

unsigned long Task::getRunCounter() { return iRunCounter; }

#ifdef _TASK_OO_CALLBACKS

// bool Task::Callback() { return true; }
bool Task::OnEnable() { return true; }
void Task::OnDisable() { }

#else

void Task::setCallback(TaskCallback aCallback) { iCallback = aCallback; }
void Task::setOnEnable(TaskOnEnable aCallback) { iOnEnable = aCallback; }
void Task::setOnDisable(TaskOnDisable aCallback) { iOnDisable = aCallback; }

#endif // _TASK_OO_CALLBACKS


/** Resets (initializes) the task/
 * Task is not enabled and is taken out
 * out of the execution chain as a result
 */
void Task::reset() {
  
#ifdef _TASK_THREAD_SAFE
    iMutex = 1;
#endif  // _TASK_THREAD_SAFE

    iStatus.enabled = false;
    iStatus.inonenable = false;
    iStatus.canceled = false;
    iPreviousMillis = 0;
    iInterval = iDelay = 0;
    iPrev = NULL;
    iNext = NULL;
    iScheduler = NULL;
    iRunCounter = 0;

#ifdef _TASK_SCHEDULING_OPTIONS
    iOption = TASK_SCHEDULE;
#endif  // _TASK_SCHEDULING_OPTIONS

#ifdef _TASK_TIMECRITICAL
    iOverrun = 0;
    iStartDelay = 0;
#endif  // _TASK_TIMECRITICAL

#ifdef _TASK_WDT_IDS
    iControlPoint = 0;
#endif  // _TASK_WDT_IDS

#ifdef _TASK_LTS_POINTER
    iLTS = NULL;
#endif  // _TASK_LTS_POINTER

#ifdef _TASK_STATUS_REQUEST
    iStatusRequest = NULL;
    iStatus.waiting = 0;
    iMyStatusRequest.signalComplete();
#endif  // _TASK_STATUS_REQUEST

#ifdef  _TASK_TIMEOUT
    iTimeout = 0;
    iStarttime = 0;
    iStatus.timeout = false;
#endif  // _TASK_TIMEOUT

#ifdef _TASK_THREAD_SAFE
    iMutex = 0;
#endif  // _TASK_THREAD_SAFE
}

/** Explicitly set Task execution parameters
 * @param aInterval - execution interval in ms
 * @param aIterations - number of iterations, use -1 for no limit
 * @param aCallback - pointer to the callback method which executes the task actions
 * @param aOnEnable - pointer to the callback method which is called on enable()
 * @param aOnDisable - pointer to the callback method which is called on disable()
 */

#ifdef _TASK_OO_CALLBACKS
void Task::set(unsigned long aInterval, long aIterations) {
#else
void Task::set(unsigned long aInterval, long aIterations, TaskCallback aCallback, TaskOnEnable aOnEnable, TaskOnDisable aOnDisable) {
#ifdef _TASK_THREAD_SAFE
    iMutex++;
#endif  // _TASK_THREAD_SAFE

    iCallback = aCallback;
    iOnEnable = aOnEnable;
    iOnDisable = aOnDisable;

#ifdef _TASK_THREAD_SAFE
    iMutex--;
#endif  // _TASK_THREAD_SAFE

#endif // _TASK_OO_CALLBACKS

#ifdef _TASK_THREAD_SAFE
    iMutex++;
#endif  // _TASK_THREAD_SAFE

    setInterval(aInterval);
    iSetIterations = iIterations = aIterations;
#ifdef _TASK_THREAD_SAFE
    iMutex--;
#endif  // _TASK_THREAD_SAFE

}

/** Sets number of iterations for the task
 * if task is enabled, schedule for immediate execution
 * @param aIterations - number of iterations, use -1 for no limit
 */
void Task::setIterations(long aIterations) {
#ifdef _TASK_THREAD_SAFE
    iMutex++;
#endif  // _TASK_THREAD_SAFE

    iSetIterations = iIterations = aIterations;

#ifdef _TASK_THREAD_SAFE
    iMutex--;
#endif  // _TASK_THREAD_SAFE
}

#ifndef _TASK_OO_CALLBACKS

/** Prepare task for next step iteration following yielding of control to the scheduler
 * @param aCallback - pointer to the callback method for the next step
 */
void Task::yield (TaskCallback aCallback) {
#ifdef _TASK_THREAD_SAFE
    iMutex++;
#endif  // _TASK_THREAD_SAFE

    iCallback = aCallback;
    forceNextIteration();

    // The next 2 lines adjust runcounter and number of iterations
    // as if it is the same run of the callback, just split between
    // a series of callback methods
    iRunCounter--;
    if ( iIterations >= 0 ) iIterations++;

#ifdef _TASK_THREAD_SAFE
    iMutex--;
#endif  // _TASK_THREAD_SAFE
}

/** Prepare task for next step iteration following yielding of control to the scheduler
 * @param aCallback - pointer to the callback method for the next step
 */
void Task::yieldOnce (TaskCallback aCallback) {
#ifdef _TASK_THREAD_SAFE
    iMutex++;
#endif  // _TASK_THREAD_SAFE

    yield(aCallback);
    iIterations = 1;

#ifdef _TASK_THREAD_SAFE
    iMutex--;
#endif  // _TASK_THREAD_SAFE
}
#endif // _TASK_OO_CALLBACKS


/** Enables the task
 *  schedules it for execution as soon as possible,
 *  and resets the RunCounter back to zero
 */
bool Task::enable() {
    if (iScheduler) { // activation without active scheduler does not make sense

#ifdef _TASK_THREAD_SAFE
        iMutex++;
#endif  // _TASK_THREAD_SAFE

        iRunCounter = 0;
        iStatus.canceled = false;

#ifdef _TASK_OO_CALLBACKS
        if ( !iStatus.inonenable ) {
            Task *current = iScheduler->iCurrent;
            iScheduler->iCurrent = this;
            iStatus.inonenable = true;      // Protection against potential infinite loop
            iStatus.enabled = OnEnable();
            iStatus.inonenable = false;     // Protection against potential infinite loop
            iScheduler->iCurrent = current;
        }
#else
        if ( iOnEnable && !iStatus.inonenable ) {
            Task *current = iScheduler->iCurrent;
            iScheduler->iCurrent = this;
            iStatus.inonenable = true;      // Protection against potential infinite loop
            iStatus.enabled = iOnEnable();
            iStatus.inonenable = false;     // Protection against potential infinite loop
            iScheduler->iCurrent = current;
        }
        else {
            iStatus.enabled = true;
        }
#endif // _TASK_OO_CALLBACKS

        iPreviousMillis = _TASK_TIME_FUNCTION() - (iDelay = iInterval);

#ifdef _TASK_TIMEOUT
            resetTimeout();
#endif // _TASK_TIMEOUT

        if ( iStatus.enabled ) {
#ifdef _TASK_STATUS_REQUEST
            iMyStatusRequest.setWaiting();
#endif // _TASK_STATUS_REQUEST
        }

#ifdef _TASK_THREAD_SAFE
        iMutex--;
#endif  // _TASK_THREAD_SAFE

        return iStatus.enabled;
    }
    return false;
}

/** Enables the task only if it was not enabled already
 * Returns previous state (true if was already enabled, false if was not)
 */
bool Task::enableIfNot() {
#ifdef _TASK_THREAD_SAFE
    iMutex++;
#endif  // _TASK_THREAD_SAFE

    bool previousEnabled = iStatus.enabled;
    if ( !previousEnabled ) enable();

#ifdef _TASK_THREAD_SAFE
    iMutex--;
#endif  // _TASK_THREAD_SAFE

    return (previousEnabled);
}

/** Enables the task
 * and schedules it for execution after a delay = aInterval
 */
bool Task::enableDelayed(unsigned long aDelay) {
#ifdef _TASK_THREAD_SAFE
    iMutex++;
#endif  // _TASK_THREAD_SAFE

    enable();
    delay(aDelay);

#ifdef _TASK_THREAD_SAFE
    iMutex--;
#endif  // _TASK_THREAD_SAFE

    return iStatus.enabled;
}

#ifdef _TASK_TIMEOUT
void Task::setTimeout(unsigned long aTimeout, bool aReset) {
#ifdef _TASK_THREAD_SAFE
    iMutex++;
#endif  // _TASK_THREAD_SAFE

    iTimeout = aTimeout;
    if (aReset) resetTimeout();

#ifdef _TASK_THREAD_SAFE
    iMutex--;
#endif  // _TASK_THREAD_SAFE
}

void Task::resetTimeout() {
#ifdef _TASK_THREAD_SAFE
    iMutex++;
#endif  // _TASK_THREAD_SAFE

    iStarttime = _TASK_TIME_FUNCTION();
    iStatus.timeout = false;

#ifdef _TASK_THREAD_SAFE
    iMutex--;
#endif  // _TASK_THREAD_SAFE
}

unsigned long Task::getTimeout() {
    return iTimeout;
}

long Task::untilTimeout() {
    if ( iTimeout ) {
        return ( (long) (iStarttime + iTimeout) - (long) _TASK_TIME_FUNCTION() );
    }
    return -1;
}

bool Task::timedOut() {
    return iStatus.timeout;
}

#endif // _TASK_TIMEOUT



/** Delays Task for execution after a delay = aInterval (if task is enabled).
 * leaves task enabled or disabled
 * if aDelay is zero, delays for the original scheduling interval from now
 */
void Task::delay(unsigned long aDelay) {
#ifdef _TASK_THREAD_SAFE
    iMutex++;
#endif  // _TASK_THREAD_SAFE

    iDelay = aDelay ? aDelay : iInterval;
    iPreviousMillis = _TASK_TIME_FUNCTION(); 

#ifdef _TASK_THREAD_SAFE
    iMutex--;
#endif  // _TASK_THREAD_SAFE
}

/** Adjusts Task execution with aInterval (if task is enabled).
 */
void Task::adjust(long aInterval) {
    if ( aInterval == 0 ) return;  //  nothing to do for a zero

#ifdef _TASK_THREAD_SAFE
    iMutex++;
#endif  // _TASK_THREAD_SAFE

    if ( aInterval < 0 ) {
      iPreviousMillis += aInterval;
    }
    else {
      iDelay += aInterval;  //  we have to adjust delay because adjusting iPreviousMillis might push
                            //  it into the future beyond current millis() and cause premature trigger
    }
#ifdef _TASK_THREAD_SAFE
    iMutex--;
#endif  // _TASK_THREAD_SAFE
}


/** Schedules next iteration of Task for execution immediately (if enabled)
 * leaves task enabled or disabled
 * Task's original schedule is shifted, and all subsequent iterations will continue from this point in time
 */
void Task::forceNextIteration() {
#ifdef _TASK_THREAD_SAFE
    iMutex++;
#endif  // _TASK_THREAD_SAFE

    iPreviousMillis = _TASK_TIME_FUNCTION() - (iDelay = iInterval);

#ifdef _TASK_THREAD_SAFE
    iMutex--;
#endif  // _TASK_THREAD_SAFE
}

/** Sets the execution interval.
 * Task execution is delayed for aInterval
 * Use  enable() to schedule execution ASAP
 * @param aInterval - new execution interval
 */
void Task::setInterval (unsigned long aInterval) {
#ifdef _TASK_THREAD_SAFE
    iMutex++;
#endif  // _TASK_THREAD_SAFE

    iInterval = aInterval;
    delay(); // iDelay will be updated by the delay() function

#ifdef _TASK_THREAD_SAFE
    iMutex--;
#endif  // _TASK_THREAD_SAFE
}

/** Disables task
 * Task will no longer be executed by the scheduler
 * Returns status of the task before disable was called (i.e., if the task was already disabled)
 */

bool Task::disable() {
    bool previousEnabled = iStatus.enabled;
    iStatus.enabled = false;
    iStatus.inonenable = false;

#ifdef _TASK_OO_CALLBACKS
    if (previousEnabled) {
#else
    if (previousEnabled && iOnDisable) {
#endif // _TASK_OO_CALLBACKS

        Task *current = iScheduler->iCurrent;
        iScheduler->iCurrent = this;
#ifdef _TASK_OO_CALLBACKS
        OnDisable();
#else
        iOnDisable();
#endif // _TASK_OO_CALLBACKS

        iScheduler->iCurrent = current;
    }
#ifdef _TASK_STATUS_REQUEST
    iMyStatusRequest.signalComplete();
#endif
    return (previousEnabled);
}

/** Aborts task execution
 * Task will no longer be executed by the scheduler AND ondisable method will not be called
 */
void Task::abort() {
    iStatus.enabled = false;
    iStatus.inonenable = false;
    iStatus.canceled = true;
}


/** Cancels task execution
 * Task will no longer be executed by the scheduler. Ondisable method will be called after 'canceled' flag is set
 */
void Task::cancel() {
    iStatus.canceled = true;
    disable();
}

bool Task::canceled() {
    return iStatus.canceled;
}

/** Restarts task
 * Task will run number of iterations again
 */

bool Task::restart() {
    iIterations = iSetIterations;
    return enable();
}

/** Restarts task delayed
 * Task will run number of iterations again
 */
bool Task::restartDelayed(unsigned long aDelay) {
    iIterations = iSetIterations;
    return enableDelayed(aDelay);
}

bool Task::isFirstIteration() { return (iRunCounter <= 1); }

bool Task::isLastIteration() { return (iIterations == 0); }

#ifdef _TASK_TIMECRITICAL

long Task::getOverrun() { return iOverrun; }
long Task::getStartDelay() { return iStartDelay; }

#endif  // _TASK_TIMECRITICAL


#ifdef _TASK_WDT_IDS

void Task::setId(unsigned int aID) { iTaskID = aID; }
unsigned int Task::getId() { return iTaskID; }
void Task::setControlPoint(unsigned int aPoint) { iControlPoint = aPoint; }
unsigned int Task::getControlPoint() { return iControlPoint; }

#endif  // _TASK_WDT_IDS

#ifdef _TASK_LTS_POINTER

void  Task::setLtsPointer(void *aPtr) { iLTS = aPtr; }
void* Task::getLtsPointer() { return iLTS; }

#endif  // _TASK_LTS_POINTER

// ------------------ Scheduler implementation --------------------

/** Default constructor.
 * Creates a scheduler with an empty execution chain.
 */
Scheduler::Scheduler() {
    init();
#ifdef _TASK_SLEEP_ON_IDLE_RUN
    setSleepMethod(&SleepMethod);
#endif // _TASK_SLEEP_ON_IDLE_RUN
}

/*
Scheduler::~Scheduler() {
#ifdef _TASK_SLEEP_ON_IDLE_RUN
#endif // _TASK_SLEEP_ON_IDLE_RUN
}
*/

/** Initializes all internal varaibles
 */
void Scheduler::init() {
    iEnabled = false;
    
    iFirst = NULL;
    iLast = NULL;
    iCurrent = NULL;

    iPaused = false;

#ifdef _TASK_PRIORITY
    iHighPriority = NULL;
#endif  // _TASK_PRIORITY

#ifdef _TASK_SLEEP_ON_IDLE_RUN
    allowSleep(true);
#endif  // _TASK_SLEEP_ON_IDLE_RUN

#ifdef _TASK_TIMECRITICAL
    cpuLoadReset();
#endif  // _TASK_TIMECRITICAL

    iEnabled = true;  
}

/** Appends task aTask to the tail of the execution chain.
 * @param &aTask - reference to the Task to be appended.
 * @note Task can only be part of the chain once.
 */
 void Scheduler::addTask(Task& aTask) {
// If task already belongs to a scheduler, we should not be adding
// it to this scheduler. It should be deleted from the other scheduler first. 
    if (aTask.iScheduler != NULL)
        return;

    iEnabled = false;

    aTask.iScheduler = this;
// First task situation:
    if (iFirst == NULL) {
        iFirst = &aTask;
        aTask.iPrev = NULL;
    }
    else {
// This task gets linked back to the previous last one
        aTask.iPrev = iLast;
        iLast->iNext = &aTask;
    }
// "Previous" last task gets linked to this one - as this one becomes the last one
    aTask.iNext = NULL;
    iLast = &aTask;

    iEnabled = true;
}

/** Deletes specific Task from the execution chain
 * @param &aTask - reference to the task to be deleted from the chain
 */
void Scheduler::deleteTask(Task& aTask) {
// Can only delete own tasks
    if (aTask.iScheduler != this) 
        return;
    
    iEnabled = false;

    aTask.iScheduler = NULL;
    if (aTask.iPrev == NULL) {
        if (aTask.iNext == NULL) {
            iFirst = NULL;
            iLast = NULL;
            iEnabled = true;
            return;
        }
        else {
            aTask.iNext->iPrev = NULL;
            iFirst = aTask.iNext;
            aTask.iNext = NULL;
            iEnabled = true;
            return;
        }
    }

    if (aTask.iNext == NULL) {
        aTask.iPrev->iNext = NULL;
        iLast = aTask.iPrev;
        aTask.iPrev = NULL;
        iEnabled = true;
        return;
    }

    aTask.iPrev->iNext = aTask.iNext;
    aTask.iNext->iPrev = aTask.iPrev;
    aTask.iPrev = NULL;
    aTask.iNext = NULL;
    
    iEnabled = true;
}

/** Disables all tasks in the execution chain
 * Convenient for error situations, when the only
 * task remaining active is an error processing task
 * @param aRecursive - if true, tasks of the higher priority chains are disabled as well recursively
 */
#ifdef _TASK_PRIORITY
void Scheduler::disableAll(bool aRecursive) {
#else
void Scheduler::disableAll() {
#endif

    iEnabled = false;
    
    Task    *current = iFirst;
    while (current) {
        current->disable();
        current = current->iNext;
    }

#ifdef _TASK_PRIORITY
    if (aRecursive && iHighPriority) iHighPriority->disableAll(true);
#endif  // _TASK_PRIORITY

    iEnabled = true;
}


/** Enables all the tasks in the execution chain
 * @param aRecursive - if true, tasks of the higher priority chains are enabled as well recursively
 */
#ifdef _TASK_PRIORITY
void Scheduler::enableAll(bool aRecursive) {
#else
void Scheduler::enableAll() {
#endif    

    iEnabled = false;
    
    Task    *current = iFirst;
    while (current) {
        current->enable();
        current = current->iNext;
    }

#ifdef _TASK_PRIORITY
    if (aRecursive && iHighPriority) iHighPriority->enableAll(true);
#endif  // _TASK_PRIORITY

    iEnabled = true;
}

/** Sets scheduler for the higher priority tasks (support for layered task priority)
 * @param aScheduler - pointer to a scheduler for the higher priority tasks
 */
#ifdef _TASK_PRIORITY
void Scheduler::setHighPriorityScheduler(Scheduler* aScheduler) {
    if (aScheduler != this) iHighPriority = aScheduler;  // Setting yourself as a higher priority one will create infinite recursive call

#ifdef _TASK_SLEEP_ON_IDLE_RUN
    if (iHighPriority) {
        iHighPriority->allowSleep(false);       // Higher priority schedulers should not do power management
    }
#endif  // _TASK_SLEEP_ON_IDLE_RUN

};
#endif  // _TASK_PRIORITY


#ifdef _TASK_SLEEP_ON_IDLE_RUN
void Scheduler::allowSleep(bool aState) {
    iAllowSleep = aState;
}
#endif  // _TASK_SLEEP_ON_IDLE_RUN


#ifdef _TASK_PRIORITY
void Scheduler::startNow( bool aRecursive ) {
#else
void Scheduler::startNow() {
#endif
    unsigned long t = _TASK_TIME_FUNCTION();

    iEnabled = false;
    
    iCurrent = iFirst;
    while (iCurrent) {
        if ( iCurrent->iStatus.enabled ) iCurrent->iPreviousMillis = t - iCurrent->iDelay;
        iCurrent = iCurrent->iNext;
    }

#ifdef _TASK_PRIORITY
    if (aRecursive && iHighPriority) iHighPriority->startNow( true );
#endif  // _TASK_PRIORITY

    iEnabled = true;
}

/** Returns number millis or micros until next scheduled iteration of a given task
 *
 * @param aTask - reference to task which next iteration is in question
 */
long Scheduler::timeUntilNextIteration(Task& aTask) {

#ifdef _TASK_STATUS_REQUEST

    StatusRequest *s = aTask.getStatusRequest();
    if ( s != NULL && s->pending() )
        return (-1);    // cannot be determined
#endif
    if ( !aTask.isEnabled() )
        return (-1);    // cannot be determined

    long d = (long) aTask.iDelay - ( (long) (_TASK_TIME_FUNCTION() - aTask.iPreviousMillis) );

    if ( d < 0 )
        return (0); // Task will run as soon as possible
    return ( d );
}


Task& Scheduler::currentTask() { return *iCurrent; }      // DEPRICATED. Use the next one instead
Task* Scheduler::getCurrentTask() { return iCurrent; }

#ifdef _TASK_LTS_POINTER
void* Scheduler::currentLts() { return iCurrent->iLTS; }
#endif  // _TASK_LTS_POINTER

#ifdef _TASK_TIMECRITICAL
bool Scheduler::isOverrun() { return (iCurrent->iOverrun < 0); }

void Scheduler::cpuLoadReset() {
    iCPUStart = micros();
    iCPUCycle = 0;
    iCPUIdle = 0;
}


unsigned long Scheduler::getCpuLoadTotal() {
    return (micros() - iCPUStart);
}
#endif  // _TASK_TIMECRITICAL

    
#ifdef _TASK_SLEEP_ON_IDLE_RUN
void  Scheduler::setSleepMethod( SleepCallback aCallback ) {
    if ( aCallback != NULL ) {
        iSleepScheduler = this;
        iSleepMethod = aCallback;
    }
}
#endif  // _TASK_SLEEP_ON_IDLE_RUN


/** Makes one pass through the execution chain.
 * Tasks are executed in the order they were added to the chain
 * There is no concept of priority
 * Different pseudo "priority" could be achieved
 * by running task more frequently
 */

bool Scheduler::execute() {
  
    bool     idleRun = true;
    unsigned long m, i;  // millis, interval;

#ifdef _TASK_SLEEP_ON_IDLE_RUN
    unsigned long tFinish;
    unsigned long tStart = micros();
#endif  // _TASK_SLEEP_ON_IDLE_RUN

#ifdef _TASK_TIMECRITICAL
    unsigned long tPassStart;
    unsigned long tTaskStart, tTaskFinish;

#ifdef _TASK_SLEEP_ON_IDLE_RUN
    unsigned long tIdleStart = 0;
#endif  // _TASK_SLEEP_ON_IDLE_RUN

#endif  // _TASK_TIMECRITICAL

    Task *nextTask;     // support for deleting the task in the onDisable method
    iCurrent = iFirst;

#ifdef _TASK_PRIORITY
    // If lower priority scheduler does not have a single task in the chain
    // the higher priority scheduler still has to have a chance to run
        if (!iCurrent && iHighPriority) iHighPriority->execute();
        iCurrentScheduler = this;
#endif  // _TASK_PRIORITY

    //  each scheduled is enabled/disabled individually, so check iEnabled only
    //  after the higher priority scheduler has been invoked.
    if ( !iEnabled ) return true; //  consider this to be an idle run

    while (!iPaused && iCurrent) {

#ifdef _TASK_TIMECRITICAL
        tPassStart = micros();
        tTaskStart = tTaskFinish = 0; 
#endif  // _TASK_TIMECRITICAL

#ifdef _TASK_PRIORITY
    // If scheduler for higher priority tasks is set, it's entire chain is executed on every pass of the base scheduler
        if (iHighPriority) idleRun = iHighPriority->execute() && idleRun;
        iCurrentScheduler = this;
#endif  // _TASK_PRIORITY
        nextTask = iCurrent->iNext;
        do {
            if ( iCurrent->iStatus.enabled ) {

#ifdef _TASK_THREAD_SAFE
            //  this task is in the scheduling state and should not be invoked
            //  as there could be incosistent settings until scheduling is done
            if ( iCurrent->iMutex ) break;
#endif  // _TASK_THREAD_SAFE

#ifdef _TASK_WDT_IDS
    // For each task the control points are initialized to avoid confusion because of carry-over:
                iCurrent->iControlPoint = 0;
#endif  // _TASK_WDT_IDS

    // Disable task on last iteration:
                if (iCurrent->iIterations == 0) {
                    iCurrent->disable();
                    break;
                }
                m = _TASK_TIME_FUNCTION();
                i = iCurrent->iInterval;

#ifdef _TASK_TIMEOUT
    // Disable task on a timeout
                if ( iCurrent->iTimeout && (m - iCurrent->iStarttime > iCurrent->iTimeout) ) {
                    iCurrent->iStatus.timeout = true;
                    iCurrent->disable();
                    break;
                }
#endif // _TASK_TIMEOUT

#ifdef  _TASK_STATUS_REQUEST
    // If StatusRequest object was provided, and still pending, and task is waiting, this task should not run
    // Otherwise, continue with execution as usual.  Tasks waiting to StatusRequest need to be rescheduled according to
    // how they were placed into waiting state (waitFor or waitForDelayed)
                if ( iCurrent->iStatus.waiting ) {
#ifdef _TASK_TIMEOUT
                    StatusRequest *sr = iCurrent->iStatusRequest;
                    if ( sr->iTimeout && (m - sr->iStarttime > sr->iTimeout) ) {
                      sr->signalComplete(TASK_SR_TIMEOUT);
                    }
#endif // _TASK_TIMEOUT
                    if ( (iCurrent->iStatusRequest)->pending() ) break;
                    if (iCurrent->iStatus.waiting == _TASK_SR_NODELAY) {
                        iCurrent->iPreviousMillis = m - (iCurrent->iDelay = i);
                    }
                    else {
                        iCurrent->iPreviousMillis = m;
                    }
                    iCurrent->iStatus.waiting = 0;
                }
#endif  // _TASK_STATUS_REQUEST

                if ( m - iCurrent->iPreviousMillis < iCurrent->iDelay ) break;

                if ( iCurrent->iIterations > 0 ) iCurrent->iIterations--;  // do not decrement (-1) being a signal of never-ending task
                iCurrent->iRunCounter++;
#ifdef _TASK_SCHEDULING_OPTIONS
                switch (iCurrent->iOption) {
                  case TASK_INTERVAL:
                    iCurrent->iPreviousMillis = m;
                    break;
                    
                  case TASK_SCHEDULE_NC:
                    iCurrent->iPreviousMillis += iCurrent->iDelay; 
                    {
                        long ov = (long) ( iCurrent->iPreviousMillis + i - m );
                        if ( ov < 0 ) {
                            long ii = i ? i : 1;
                            iCurrent->iPreviousMillis += ((m - iCurrent->iPreviousMillis) / ii) * ii;
                        }
                    }
                    break;
                    
                  default:
                    iCurrent->iPreviousMillis += iCurrent->iDelay;
                }
#else
                iCurrent->iPreviousMillis += iCurrent->iDelay;
#endif  // _TASK_SCHEDULING_OPTIONS

#ifdef _TASK_TIMECRITICAL
    // Updated_previous+current interval should put us into the future, so iOverrun should be positive or zero.
    // If negative - the task is behind (next execution time is already in the past)
                unsigned long p = iCurrent->iPreviousMillis;
                iCurrent->iOverrun = (long) ( p + i - m );
                iCurrent->iStartDelay = (long) ( m - p );
#endif  // _TASK_TIMECRITICAL

                iCurrent->iDelay = i;
                
#ifdef _TASK_TIMECRITICAL
                tTaskStart = micros();
#endif  // _TASK_TIMECRITICAL

#ifdef _TASK_OO_CALLBACKS
                idleRun = !iCurrent->Callback();
#else
                if ( iCurrent->iCallback ) {
                    iCurrent->iCallback();
                    idleRun = false;
                }
#endif // _TASK_OO_CALLBACKS

#ifdef _TASK_TIMECRITICAL
                tTaskFinish = micros();
#endif  // _TASK_TIMECRITICAL

            }
        } while (0);    //guaranteed single run - allows use of "break" to exit

        iCurrent = nextTask;
        
        
#ifdef _TASK_TIMECRITICAL
        iCPUCycle += ( (micros() - tPassStart) - (tTaskFinish - tTaskStart) );
#endif  // _TASK_TIMECRITICAL
        
#if defined (ARDUINO_ARCH_ESP8266) || defined (ARDUINO_ARCH_ESP32)
        yield();
#endif  // ARDUINO_ARCH_ESPxx
    }


#ifdef _TASK_SLEEP_ON_IDLE_RUN
    tFinish = micros(); // Scheduling pass end time in microseconds.

    if (idleRun && iAllowSleep) {
        if ( iSleepScheduler == this ) { // only one scheduler should make the MC go to sleep. 
            if ( iSleepMethod != NULL ) {
                
#ifdef _TASK_TIMECRITICAL
                tIdleStart = micros();
#endif  // _TASK_TIMECRITICAL

                (*iSleepMethod)( tFinish-tStart );
                
#ifdef _TASK_TIMECRITICAL
                iCPUIdle += (micros() - tIdleStart);
#endif  // _TASK_TIMECRITICAL
            }
        }
    }
    


#endif  // _TASK_SLEEP_ON_IDLE_RUN

    return (idleRun);
}



#endif /* _TASKSCHEDULER_H_ */
