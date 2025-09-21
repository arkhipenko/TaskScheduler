/**
 * @file TaskSchedulerDeclarations.h
 * @brief Cooperative multitasking library for Arduino microcontrollers
 * @author Anatoli Arkhipenko
 * @version 4.0.0
 * @date 2015-2023
 * @copyright Copyright (c) 2015-2025 Anatoli Arkhipenko
 *
 * @details A lightweight implementation of cooperative multitasking (task scheduling) supporting:
 * - Periodic task execution, with dynamic execution period in milliseconds (default) or microseconds
 * - Number of iterations (limited or infinite number of iterations)
 * - Execution of tasks in the predefined sequence
 * - Dynamic change of task execution parameters (frequency, number of iterations, callback methods)
 * - Power saving via entering IDLE sleep mode when tasks are not scheduled to run
 * - Support for event-driven task invocation via Status Request object
 * - Support for task IDs and Control Points for error handling and watchdog timer
 * - Support for Local Task Storage pointer (allowing the use of same callback code for multiple tasks)
 * - Support for layered task prioritization
 * - Support for std::functions (ESP8266 only)
 * - Overall task timeout
 * - Static and dynamic callback method binding
 *
 * Scheduling overhead: between 15 and 18 microseconds per scheduling pass (Arduino UNO rev 3 @ 16MHz clock, single scheduler w/o prioritization)
 */

#ifndef _TASKSCHEDULERDECLARATIONS_H_
#define _TASKSCHEDULERDECLARATIONS_H_

// ----------------------------------------
/**
 * @defgroup CompileTimeOptions Compile Time Options
 * @brief The following "defines" control library functionality at compile time,
 * and should be used in the main sketch depending on the functionality required
 * @{
 */

/**
 * @def _TASK_TIMECRITICAL
 * @brief Enable monitoring scheduling overruns
 * @details Enables tracking when the current execution took place relative to when it was scheduled.
 * Provides getStartDelay() and getOverrun() methods for performance monitoring.
 */
// #define _TASK_TIMECRITICAL

/**
 * @def _TASK_SLEEP_ON_IDLE_RUN
 * @brief Enable 1 ms SLEEP_IDLE powerdowns between runs if no callback methods were invoked during the pass
 * @details When enabled, the scheduler will place the processor into IDLE sleep mode for approximately 1 ms
 * after what is determined to be an "idle" pass. AVR boards only.
 */
// #define _TASK_SLEEP_ON_IDLE_RUN

/**
 * @def _TASK_STATUS_REQUEST
 * @brief Compile with support for StatusRequest functionality
 * @details Enables triggering tasks on status change events in addition to time-based scheduling.
 * Allows tasks to wait on an event and signal event completion to each other.
 */
// #define _TASK_STATUS_REQUEST

/**
 * @def _TASK_WDT_IDS
 * @brief Compile with support for watchdog timer control points and task IDs
 * @details Each task can be assigned an ID and Control Points can be defined within tasks
 * for watchdog timer integration and error handling.
 */
// #define _TASK_WDT_IDS

/**
 * @def _TASK_LTS_POINTER
 * @brief Compile with support for local task storage pointer
 * @details LTS is a generic (void*) pointer that can reference a variable or structure
 * specific to a particular task, allowing the same callback code for multiple tasks.
 */
// #define _TASK_LTS_POINTER

/**
 * @def _TASK_PRIORITY
 * @brief Support for layered scheduling priority
 * @details Enables task prioritization by creating several schedulers and organizing them in priority layers.
 * Higher priority tasks are evaluated more frequently.
 */
// #define _TASK_PRIORITY

/**
 * @def _TASK_MICRO_RES
 * @brief Support for microsecond resolution
 * @details Enables microsecond scheduling resolution instead of default millisecond resolution.
 * All time-relevant parameters will be treated as microseconds.
 */
// #define _TASK_MICRO_RES

/**
 * @def _TASK_STD_FUNCTION
 * @brief Support for std::function (ESP8266/ESP32 ONLY)
 * @details Enables support for standard functions instead of function pointers for callbacks.
 */
// #define _TASK_STD_FUNCTION

/**
 * @def _TASK_DEBUG
 * @brief Make all methods and variables public for debug purposes
 * @details Should not be used in production. Exposes all private and protected members as public.
 */
// #define _TASK_DEBUG

/**
 * @def _TASK_INLINE
 * @brief Make all methods "inline"
 * @details Needed to support some multi-tab, multi-file implementations. Lets compiler optimize.
 */
// #define _TASK_INLINE

/**
 * @def _TASK_TIMEOUT
 * @brief Support for overall task timeout
 * @details Any task can be set to time out after a certain period, and timeout can be reset.
 * Can be used as an individual Task's watchdog timer.
 */
// #define _TASK_TIMEOUT

/**
 * @def _TASK_OO_CALLBACKS
 * @brief Support for callbacks via inheritance
 * @details Useful for implementing Tasks as classes derived from the Task class.
 * Enables dynamic binding for object-oriented callback approach.
 */
// #define _TASK_OO_CALLBACKS

/**
 * @def _TASK_EXPOSE_CHAIN
 * @brief Methods to access tasks in the task chain
 * @details Provides access to scheduling chain methods and tasks on the chain.
 */
// #define _TASK_EXPOSE_CHAIN

/**
 * @def _TASK_SCHEDULING_OPTIONS
 * @brief Support for multiple scheduling options
 * @details Enables different task scheduling options like TASK_SCHEDULE, TASK_SCHEDULE_NC, and TASK_INTERVAL.
 */
// #define _TASK_SCHEDULING_OPTIONS

/**
 * @def _TASK_SELF_DESTRUCT
 * @brief Enable tasks to "self-destruct" after disable
 * @details Tasks can be set to automatically delete themselves when disabled.
 */
// #define _TASK_SELF_DESTRUCT

/**
 * @def _TASK_TICKLESS
 * @brief Enable support for tickless sleep on FreeRTOS
 * @details Enables support for tickless sleep mode on FreeRTOS systems.
 */
// #define _TASK_TICKLESS

/**
 * @def _TASK_DO_NOT_YIELD
 * @brief Disable yield() method in execute() for ESP chips
 * @details Disables automatic yielding in the execute loop for ESP-based systems.
 */
// #define _TASK_DO_NOT_YIELD

/**
 * @def _TASK_ISR_SUPPORT
 * @brief For ESP chips - place control methods in IRAM
 * @details Places critical control methods in IRAM for ESP8266/ESP32 interrupt support.
 */
// #define _TASK_ISR_SUPPORT

/**
 * @def _TASK_NON_ARDUINO
 * @brief For non-Arduino use
 * @details Enables compilation for non-Arduino environments.
 */
// #define _TASK_NON_ARDUINO

/**
 * @def _TASK_HEADER_AND_CPP
 * @brief Compile CPP file (non-Arduino IDE platforms)
 * @details For non-Arduino IDE platforms that require explicit CPP compilation.
 */
// #define _TASK_HEADER_AND_CPP

/**
 * @def _TASK_THREAD_SAFE
 * @brief Enable additional checking for thread safety
 * @details Uses an internal mutex to protect task scheduling methods from preemption.
 * Recommended for ESP32 and other MCUs running under preemptive schedulers like FreeRTOS.
 */
// #define _TASK_THREAD_SAFE

/** @} */ // End of CompileTimeOptions group

#ifdef _TASK_NON_ARDUINO
#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#else 
#include <Arduino.h>
#endif

/**
 * @brief Forward declaration of Scheduler class
 */
class Scheduler;

/**
 * @defgroup SchedulingOptions Task Scheduling Options
 * @brief Constants defining different task scheduling behaviors
 * @{
 */

/**
 * @def TASK_SCHEDULE
 * @brief Default scheduling option
 * @details Maintains the original schedule. Tasks may need to "catch up" if delayed.
 * If a task is scheduled to run every 10 seconds and starts at 9:00:00,
 * the scheduler will try to invoke tasks as close to 9:00:10, 9:00:20 as possible.
 */
#define TASK_SCHEDULE       0

/**
 * @def TASK_SCHEDULE_NC
 * @brief Schedule with no catch-ups (always in the future)
 * @details Similar to TASK_SCHEDULE but without "catch up". Tasks are invoked at the next
 * scheduled point, but the number of iterations may not be correct if delayed.
 */
#define TASK_SCHEDULE_NC    1

/**
 * @def TASK_INTERVAL
 * @brief Interval-based scheduling (always in the future)
 * @details Schedules the next invocation with priority to a "period". A task scheduled for
 * 9:00:00 with a 10-second interval that was actually invoked at 9:00:06 will be
 * scheduled for the next invocation at 9:00:16.
 */
#define TASK_INTERVAL       2

/** @} */ // End of SchedulingOptions group

#ifdef _TASK_DEBUG
    #define _TASK_SCOPE  public
#else
    #define _TASK_SCOPE  private
#endif

/**
 * @defgroup TaskIterationOptions Task Iteration Options
 * @brief Common options for task scheduling iterations
 * @{
 */

/**
 * @def TASK_IMMEDIATE
 * @brief Task interval for immediate execution
 * @details When used as interval, causes task to execute immediately during next scheduling pass.
 */
#define TASK_IMMEDIATE          0

/**
 * @def TASK_FOREVER
 * @brief Task number of iterations for infinite execution
 * @details When used as iteration count, causes task to run indefinitely until explicitly disabled.
 */
#define TASK_FOREVER         (-1)

/**
 * @def TASK_ONCE
 * @brief Task single iteration
 * @details When used as iteration count, causes task to run only once and then become disabled.
 */
#define TASK_ONCE               1

/** @} */ // End of TaskIterationOptions group

/**
 * @defgroup IntervalNoDelayOptions Options for setIntervalNodelay() method
 * @brief Options controlling how interval changes are handled
 * @{
 */

/**
 * @def TASK_INTERVAL_KEEP
 * @brief Keep current execution timing
 * @details Maintains the current execution schedule when changing interval.
 */
#define TASK_INTERVAL_KEEP      0

/**
 * @def TASK_INTERVAL_RECALC
 * @brief Recalculate execution timing
 * @details Recalculates the next execution time based on the new interval.
 */
#define TASK_INTERVAL_RECALC    1

/**
 * @def TASK_INTERVAL_RESET
 * @brief Reset execution timing
 * @details Resets the execution schedule to start from the current time.
 */
#define TASK_INTERVAL_RESET     2

/** @} */ // End of IntervalNoDelayOptions group

/**
 * @defgroup TimeoutOptions Task Timeout Options
 * @brief Options for task timeout functionality
 * @{
 */
#ifdef _TASK_TIMEOUT
/**
 * @def TASK_NOTIMEOUT
 * @brief No timeout value
 * @details Used to indicate that a task should not have a timeout.
 */
#define TASK_NOTIMEOUT          0
#endif
/** @} */ // End of TimeoutOptions group

#ifdef _TASK_PRIORITY
    extern Scheduler* iCurrentScheduler;
#endif // _TASK_PRIORITY

#ifdef _TASK_INLINE
#define __TASK_INLINE  inline
#else
#define __TASK_INLINE
#endif

#ifdef _TASK_ISR_SUPPORT
#if defined (ARDUINO_ARCH_ESP8266)
#define __TASK_IRAM ICACHE_RAM_ATTR
#endif 
#if  defined (ARDUINO_ARCH_ESP32)
#define __TASK_IRAM IRAM_ATTR
#endif
#endif
#ifndef __TASK_IRAM
#define __TASK_IRAM
#endif

/**
 * @defgroup TimeConstants Time Constants
 * @brief Predefined time constants for task intervals
 * @details Values are in milliseconds by default, or microseconds when _TASK_MICRO_RES is enabled
 * @{
 */
#ifndef _TASK_MICRO_RES

/**
 * @def TASK_MILLISECOND
 * @brief One millisecond interval
 */
#define TASK_MILLISECOND       1UL

/**
 * @def TASK_SECOND
 * @brief One second interval (1000 milliseconds)
 */
#define TASK_SECOND         1000UL

/**
 * @def TASK_MINUTE
 * @brief One minute interval (60000 milliseconds)
 */
#define TASK_MINUTE        60000UL

/**
 * @def TASK_HOUR
 * @brief One hour interval (3600000 milliseconds)
 */
#define TASK_HOUR        3600000UL

#else

/**
 * @def TASK_MILLISECOND
 * @brief One millisecond interval (1000 microseconds)
 */
#define TASK_MILLISECOND    1000UL

/**
 * @def TASK_SECOND
 * @brief One second interval (1000000 microseconds)
 */
#define TASK_SECOND      1000000UL

/**
 * @def TASK_MINUTE
 * @brief One minute interval (60000000 microseconds)
 */
#define TASK_MINUTE     60000000UL

/**
 * @def TASK_HOUR
 * @brief One hour interval (3600000000 microseconds)
 */
#define TASK_HOUR     3600000000UL

#endif  // _TASK_MICRO_RES
/** @} */ // End of TimeConstants group

#ifdef _TASK_TICKLESS
#define _TASK_NEXTRUN_UNDEFINED 0b0
#define _TASK_NEXTRUN_IMMEDIATE 0b1
#define _TASK_NEXTRUN_TIMED     0x10
#endif  //  _TASK_TICKLESS

#ifdef _TASK_THREAD_SAFE

/**
 * @enum _task_request_type_t
 * @brief Task request types for thread-safe operations
 *
 * @details This enum mirrors the Task class API for invocation via requestAction methods
 * when _TASK_THREAD_SAFE option is active. It provides a thread-safe way to execute
 * task control methods by placing requests on a queue that is processed during the
 * scheduler's execute() method.
 *
 * Each enum value corresponds to a specific Task method that can be called safely
 * from other threads or interrupt contexts. The scheduler processes these requests
 * atomically during its execution cycle.
 *
 * @note Only available when compiled with _TASK_THREAD_SAFE support.
 *
 * @see Scheduler::requestAction()
 */
typedef enum {
#ifdef _TASK_STATUS_REQUEST
    TASK_SR_REQUEST_SETWAITING,        ///< Thread-safe StatusRequest::setWaiting()
    TASK_SR_REQUEST_SIGNAL,            ///< Thread-safe StatusRequest::signal()
    TASK_SR_REQUEST_SIGNALCOMPLETE,    ///< Thread-safe StatusRequest::signalComplete()

#ifdef _TASK_TIMEOUT
    TASK_SR_REQUEST_SETTIMEOUT,        ///< Thread-safe StatusRequest::setTimeout()
    TASK_SR_REQUEST_RESETTIMEOUT,      ///< Thread-safe StatusRequest::resetTimeout()
#endif  // _TASK_TIMEOUT

#endif  // _TASK_STATUS_REQUEST

#ifdef _TASK_LTS_POINTER
    TASK_REQUEST_SETLTSPOINTER,        ///< Thread-safe Task::setLtsPointer()
#endif

#ifdef _TASK_SELF_DESTRUCT
    TASK_REQUEST_SETSELFDESTRUCT,      ///< Thread-safe Task::setSelfDestruct()
#endif  // _TASK_SELF_DESTRUCT

#ifdef _TASK_SCHEDULING_OPTIONS
    TASK_REQUEST_SETSCHEDULINGOPTION,  ///< Thread-safe Task::setSchedulingOption()
#endif  // _TASK_SCHEDULING_OPTIONS

#ifdef _TASK_TIMEOUT
    TASK_REQUEST_SETTIMEOUT,           ///< Thread-safe Task::setTimeout()
    TASK_REQUEST_RESETTIMEOUT,         ///< Thread-safe Task::resetTimeout()
#endif  // _TASK_TIMEOUT

#ifdef _TASK_STATUS_REQUEST
    TASK_REQUEST_WAITFOR,              ///< Thread-safe Task::waitFor()
    TASK_REQUEST_WAITFORDELAYED,       ///< Thread-safe Task::waitForDelayed()
#endif  // _TASK_STATUS_REQUEST

#ifdef _TASK_WDT_IDS
    TASK_REQUEST_SETID,                ///< Thread-safe Task::setId()
    TASK_REQUEST_SETCONTROLPOINT,      ///< Thread-safe Task::setControlPoint()
#endif  // _TASK_WDT_IDS

    TASK_REQUEST_ENABLE,               ///< Thread-safe Task::enable()
    TASK_REQUEST_ENABLEIFNOT,          ///< Thread-safe Task::enableIfNot()
    TASK_REQUEST_ENABLEDELAYED,        ///< Thread-safe Task::enableDelayed()
    TASK_REQUEST_RESTART,              ///< Thread-safe Task::restart()
    TASK_REQUEST_RESTARTDELAYED,       ///< Thread-safe Task::restartDelayed()
    TASK_REQUEST_DELAY,                ///< Thread-safe Task::delay()
    TASK_REQUEST_ADJUST,               ///< Thread-safe Task::adjust()
    TASK_REQUEST_FORCENEXTITERATION,   ///< Thread-safe Task::forceNextIteration()
    TASK_REQUEST_DISABLE,              ///< Thread-safe Task::disable()
    TASK_REQUEST_ABORT,                ///< Thread-safe Task::abort()
    TASK_REQUEST_CANCEL,               ///< Thread-safe Task::cancel()
    TASK_REQUEST_SET,                  ///< Thread-safe Task::set()
    TASK_REQUEST_SETINTERVAL,          ///< Thread-safe Task::setInterval()
    TASK_REQUEST_SETINTERVALNODELAY,   ///< Thread-safe Task::setIntervalNodelay()
    TASK_REQUEST_SETITERATIONS,        ///< Thread-safe Task::setIterations()
    TASK_REQUEST_SETCALLBACK,          ///< Thread-safe Task::setCallback()
    TASK_REQUEST_SETONENABLE,          ///< Thread-safe Task::setOnEnable()
    TASK_REQUEST_SETONDISABLE          ///< Thread-safe Task::setOnDisable()
} _task_request_type_t;

#ifdef _TASK_STATUS_REQUEST
// void setWaiting(unsigned int aCount = 1);
#define TASK_SR_REQUEST_SETWAITING_1        TASK_SR_REQUEST_SETWAITING
#define TASK_SR_REQUEST_SETWAITING_1_DEFAULT   1
// __TASK_IRAM signal(int aStatus = 0);
#define TASK_SR_REQUEST_SIGNAL_1            TASK_SR_REQUEST_SIGNAL
#define TASK_SR_REQUEST_SIGNAL_1_DEFAULT    0
// __TASK_IRAM signalComplete(int aStatus = 0);
#define TASK_SR_REQUEST_SIGNALCOMPLETE_1    TASK_SR_REQUEST_SIGNALCOMPLETE
#define TASK_SR_REQUEST_SIGNALCOMPLETE_1_DEFAULT    0

#ifdef _TASK_TIMEOUT
// void setTimeout(unsigned long aTimeout)
#define TASK_SR_REQUEST_SETTIMEOUT_1        TASK_SR_REQUEST_SETTIMEOUT
// void resetTimeout();
#define TASK_SR_REQUEST_RESETTIMEOUT_0      TASK_SR_REQUEST_RESETTIMEOUT
#endif  // _TASK_TIMEOUT

#endif

#ifdef _TASK_LTS_POINTER
// void  setLtsPointer(void *aPtr);
#define TASK_REQUEST_SETLTSPOINTER_1        TASK_REQUEST_SETLTSPOINTER
#endif

#ifdef _TASK_SELF_DESTRUCT
// void setSelfDestruct(bool aSelfDestruct=true) 
#define TASK_REQUEST_SETSELFDESTRUCT_1      TASK_REQUEST_SETSELFDESTRUCT
#define TASK_REQUEST_SETSELFDESTRUCT_1_DEFAULT  true
#endif

#ifdef _TASK_SCHEDULING_OPTIONS
// void setSchedulingOption(unsigned int aOption)
#define TASK_REQUEST_SETSCHEDULINGOPTION_1  TASK_REQUEST_SETSCHEDULINGOPTION
#endif

#ifdef _TASK_TIMEOUT
// void setTimeout(unsigned long aTimeout, bool aReset=false);
#define TASK_REQUEST_SETTIMEOUT_2           TASK_REQUEST_SETTIMEOUT
#define TASK_REQUEST_SETTIMEOUT_2_DEFAULT   false
// void resetTimeout();
#define TASK_REQUEST_RESETTIMEOUT_0         TASK_REQUEST_RESETTIMEOUT
#endif  // _TASK_TIMEOUT

#ifdef _TASK_STATUS_REQUEST
// bool waitFor(StatusRequest* aStatusRequest, unsigned long aInterval = 0, long aIterations = 1)
#define TASK_REQUEST_WAITFOR_3              TASK_REQUEST_WAITFOR
#define TASK_REQUEST_WAITFOR_2_DEFAULT  0
#define TASK_REQUEST_WAITFOR_3_DEFAULT  1
// bool waitForDelayed(StatusRequest* aStatusRequest, unsigned long aInterval = 0, long aIterations = 1)
#define TASK_REQUEST_WAITFORDELAYED_3       TASK_REQUEST_WAITFORDELAYED
#define TASK_REQUEST_WAITFORDELAYED_2_DEFAULT   0
#define TASK_REQUEST_WAITFORDELAYED_3_DEFAULT   1
#endif  // _TASK_STATUS_REQUEST

#ifdef _TASK_WDT_IDS
// __TASK_INLINE void setId(unsigned int aID) ;
#define TASK_REQUEST_SETID_1                TASK_REQUEST_SETID
// __TASK_INLINE void setControlPoint(unsigned int aPoint) ;
#define TASK_REQUEST_SETCONTROLPOINT_1      TASK_REQUEST_SETCONTROLPOINT
#endif  // _TASK_WDT_IDS

// bool __TASK_IRAM enable();
#define TASK_REQUEST_ENABLE_0               TASK_REQUEST_ENABLE
// bool __TASK_IRAM enableIfNot();
#define TASK_REQUEST_ENABLEIFNOT_0          TASK_REQUEST_ENABLEIFNOT
// bool __TASK_IRAM enableDelayed(unsigned long aDelay=0);
#define TASK_REQUEST_ENABLEDELAYED_1        TASK_REQUEST_ENABLEDELAYED
#define TASK_REQUEST_ENABLEDELAYED_1_DEFAULT    0
// bool __TASK_IRAM restart();
#define TASK_REQUEST_RESTART_0              TASK_REQUEST_RESTART
// bool __TASK_IRAM restartDelayed(unsigned long aDelay=0);
#define TASK_REQUEST_RESTARTDELAYED_1       TASK_REQUEST_RESTARTDELAYED
#define TASK_REQUEST_RESTARTDELAYED_1_DEFAULT   0
// void __TASK_IRAM delay(unsigned long aDelay=0);
#define TASK_REQUEST_DELAY_1                TASK_REQUEST_DELAY
#define TASK_REQUEST_DELAY_1_DEFAULT    0
// void adjust(long aInterval);
#define TASK_REQUEST_ADJUST_1               TASK_REQUEST_ADJUST
// void __TASK_IRAM forceNextIteration();
#define TASK_REQUEST_FORCENEXTITERATION_0   TASK_REQUEST_FORCENEXTITERATION
// bool disable();
#define TASK_REQUEST_DISABLE_0              TASK_REQUEST_DISABLE
// void abort();
#define TASK_REQUEST_ABORT_0                TASK_REQUEST_ABORT
// void cancel();
#define TASK_REQUEST_CANCEL_0               TASK_REQUEST_CANCEL
// void set(unsigned long aInterval, long aIterations, TaskCallback aCallback,TaskOnEnable aOnEnable=NULL, TaskOnDisable aOnDisable=NULL);
#define TASK_REQUEST_SET_5                  TASK_REQUEST_SET
#define TASK_REQUEST_SET_4_DEFAULT          NULL
#define TASK_REQUEST_SET_5_DEFAULT          NULL
// void setInterval(unsigned long aInterval);
#define TASK_REQUEST_SETINTERVAL_1          TASK_REQUEST_SETINTERVAL
// void setIntervalNodelay(unsigned long aInterval, unsigned int aOption = TASK_INTERVAL_KEEP);
#define TASK_REQUEST_SETINTERVALNODELAY_2   TASK_REQUEST_SETINTERVALNODELAY
#define TASK_REQUEST_SETINTERVALNODELAY_2_DEFAULT   TASK_INTERVAL_KEEP
// void setIterations(long aIterations);
#define TASK_REQUEST_SETITERATIONS_1        TASK_REQUEST_SETITERATIONS
// void setCallback(TaskCallback aCallback)
#define TASK_REQUEST_SETCALLBACK_1          TASK_REQUEST_SETCALLBACK
// setOnEnable(TaskOnEnable aCallback) ;
#define TASK_REQUEST_SETONENABLE_1          TASK_REQUEST_SETONENABLE
// setOnDisable(TaskOnDisable aCallback) ;
#define TASK_REQUEST_SETONDISABLE_1         TASK_REQUEST_SETONDISABLE


/**
 * @struct _task_request_t
 * @brief Structure for thread-safe task operation requests
 *
 * @details This structure is used to call Scheduler::requestAction() and provide parameters
 * to the respective API method in a thread-safe manner. When _TASK_THREAD_SAFE is enabled,
 * this structure allows tasks to be controlled safely from interrupt contexts or other threads
 * by queuing requests that are processed atomically during the scheduler's execute() cycle.
 *
 * The structure contains the request type and up to 5 parameters that correspond to the
 * parameters of the target Task or StatusRequest method being called. The scheduler will
 * unpack these parameters and call the appropriate method during its execution.
 *
 * Usage example:
 * @code
 * _task_request_t request;
 * request.req_type = TASK_REQUEST_ENABLE;
 * request.object_ptr = &myTask;
 * // param1-param5 not needed for enable()
 * scheduler.requestAction(&request);
 * @endcode
 *
 * @note Only available when compiled with _TASK_THREAD_SAFE support.
 *
 * @see Scheduler::requestAction(), _task_request_type_t
 */
typedef struct {
    _task_request_type_t req_type;     ///< Type of request (corresponds to Task/StatusRequest method)
    void*           object_ptr;        ///< Pointer to Task or StatusRequest object to operate on
    unsigned long   param1;            ///< First parameter for the target method (if needed)
    unsigned long   param2;            ///< Second parameter for the target method (if needed)
    unsigned long   param3;            ///< Third parameter for the target method (if needed)
    unsigned long   param4;            ///< Fourth parameter for the target method (if needed)
    unsigned long   param5;            ///< Fifth parameter for the target method (if needed)
} _task_request_t;

#endif  //_TASK_THREAD_SAFE

#ifdef _TASK_STATUS_REQUEST

/**
 * @defgroup StatusRequestCodes Status Request Return Codes
 * @brief Status codes returned by StatusRequest operations
 * @{
 */

/**
 * @def TASK_SR_OK
 * @brief Successful completion status
 * @details Default successful completion status for StatusRequest operations.
 */
#define TASK_SR_OK          0

/**
 * @def TASK_SR_ERROR
 * @brief General error status
 * @details Indicates an error condition occurred during StatusRequest operation.
 */
#define TASK_SR_ERROR       (-1)

/**
 * @def TASK_SR_CANCEL
 * @brief Cancelled status
 * @details Indicates the StatusRequest operation was cancelled.
 */
#define TASK_SR_CANCEL      (-32766)

/**
 * @def TASK_SR_ABORT
 * @brief Aborted status
 * @details Indicates the StatusRequest operation was aborted.
 */
#define TASK_SR_ABORT       (-32767)

/**
 * @def TASK_SR_TIMEOUT
 * @brief Timeout status
 * @details Indicates the StatusRequest operation timed out.
 */
#define TASK_SR_TIMEOUT     (-32768)

/** @} */ // End of StatusRequestCodes group

/**
 * @defgroup StatusRequestModes Status Request Internal Modes
 * @brief Internal mode constants for StatusRequest operations
 * @{
 */

/**
 * @def _TASK_SR_NODELAY
 * @brief No delay mode for StatusRequest
 */
#define _TASK_SR_NODELAY    1

/**
 * @def _TASK_SR_DELAY
 * @brief Delay mode for StatusRequest
 */
#define _TASK_SR_DELAY      2

/** @} */ // End of StatusRequestModes group

/**
 * @class StatusRequest
 * @brief Status Request object for event-driven task invocation
 *
 * @details StatusRequest objects allow tasks to wait on an event and signal event completion to each other.
 * This enables event-driven programming where one task can "signal" completion of its function via a
 * StatusRequest object, and other tasks can "wait" on the same StatusRequest object.
 *
 * Key features:
 * - Can wait for multiple events (set via setWaiting())
 * - Supports timeout functionality
 * - Provides completion status codes
 * - Thread-safe signaling mechanisms
 *
 * @note Each task has an internal StatusRequest object that is:
 * - Always waits on 1 event (completion of this task)
 * - Activated (set to "waiting" status) after Task is enabled
 * - Completed after Task is disabled
 *
 * @see Task::waitFor(), Task::waitForDelayed()
 */
class StatusRequest {
  friend class Scheduler;
  public:
    /**
     * @brief Default constructor
     * @details Creates Status Request object with "completed" status on creation.
     */
    __TASK_INLINE StatusRequest();

    /**
     * @brief Activates Status Request object
     * @param aCount Number of events to wait for (default = 1)
     * @details By default each object waits on one event only. If aCount is supplied,
     * StatusRequest can wait on multiple events. For example, setWaiting(3) will wait
     * on three signals - useful for waiting for completion of measurements from 3 sensors.
     */
    __TASK_INLINE void setWaiting(unsigned int aCount = 1);

    /**
     * @brief Signals completion of one event
     * @param aStatus Completion code (default = 0)
     * @return true if signal was processed successfully
     * @details Signals completion of the event to the StatusRequest object and passes
     * a completion code. Passing a negative status code is considered reporting an error
     * condition and will complete the status request regardless of how many outstanding
     * signals it is still waiting for. Only the latest status code is kept.
     */
    __TASK_INLINE bool  signal(int aStatus = 0);

    /**
     * @brief Signals completion of ALL events
     * @param aStatus Completion code (default = 0)
     * @details Signals completion of ALL events to the StatusRequest object and passes
     * a completion code. The status request completes regardless of how many events
     * it is still waiting on.
     */
    __TASK_INLINE void  signalComplete(int aStatus = 0);

    /**
     * @brief Check if status request is pending (deprecated)
     * @return true if status request is still waiting for events
     * @deprecated Use isPending() instead
     */
    __TASK_INLINE bool pending() { return isPending(); };

    /**
     * @brief Check if status request is pending
     * @return true if status request is still waiting for events to happen
     */
    __TASK_INLINE bool isPending();

    /**
     * @brief Check if status request is completed (deprecated)
     * @return true if status request event has completed
     * @deprecated Use isCompleted() instead
     */
    __TASK_INLINE bool completed() { return isCompleted(); };

    /**
     * @brief Check if status request is completed
     * @return true if status request event has completed
     */
    __TASK_INLINE bool isCompleted();

    /**
     * @brief Get the status code
     * @return The status code passed to signal() and signalComplete() methods
     * @details Any positive number is considered successful completion status.
     * A 0 (zero) is considered default successful completion status.
     * Any negative number is considered an error code and unsuccessful completion.
     */
    __TASK_INLINE int  getStatus();

    /**
     * @brief Get count of remaining events
     * @return The count of events not yet completed
     * @details Typically by default a StatusRequest object only waits on 1 event.
     * However, when waiting on multiple events, returns number of events not yet completed.
     */
    __TASK_INLINE int  getCount();
    
#ifdef _TASK_TIMEOUT
    /**
     * @brief Set timeout for this StatusRequest object
     * @param aTimeout Timeout interval in milliseconds (or microseconds)
     * @details When enabled, the activated StatusRequest object will complete with
     * the code TASK_SR_TIMEOUT if no other process calls its signal() or signalComplete() method.
     */
    __TASK_INLINE void setTimeout(unsigned long aTimeout) { iTimeout = aTimeout; };

    /**
     * @brief Get timeout interval
     * @return The timeout interval for the current StatusRequest object
     * @details This is the full original interval, not the remaining time.
     */
    __TASK_INLINE unsigned long getTimeout() { return iTimeout; };

    /**
     * @brief Reset timeout counter
     * @details Resets the current timeout counter to the original value.
     * The timeout countdown starts from the beginning again.
     */
    __TASK_INLINE void resetTimeout();

    /**
     * @brief Get time until timeout
     * @return Number of milliseconds (or microseconds) until timeout
     * @details The value could be negative if the timeout has already occurred.
     */
    __TASK_INLINE long untilTimeout();
#endif

  _TASK_SCOPE:
    unsigned int  iCount;          // number of statuses to wait for. waiting for more that 65000 events seems unreasonable: unsigned int should be sufficient
    int           iStatus;         // status of the last completed request. negative = error;  zero = OK; positive = OK with a specific status (see TASK_SR_ constants)

#ifdef _TASK_TIMEOUT
    unsigned long            iTimeout;               // Task overall timeout
    unsigned long            iStarttime;             // millis at task start time
#endif // _TASK_TIMEOUT
};
#endif  // _TASK_STATUS_REQUEST

/**
 * @defgroup CallbackTypes Task Callback Types
 * @brief Type definitions for task callback functions
 * @{
 */
#ifdef _TASK_STD_FUNCTION
#include <functional>
/**
 * @typedef TaskCallback
 * @brief Main task callback function type (std::function version)
 * @details Function called periodically when task is executed. Should be non-blocking
 * and return quickly to maintain cooperative multitasking behavior.
 */
typedef std::function<void()> TaskCallback;

/**
 * @typedef TaskOnDisable
 * @brief Task disable callback function type (std::function version)
 * @details Function called when task is disabled. Used for cleanup operations.
 */
typedef std::function<void()> TaskOnDisable;

/**
 * @typedef TaskOnEnable
 * @brief Task enable callback function type (std::function version)
 * @details Function called when task is enabled. Should return true to enable task,
 * false to keep task disabled. Used for initialization operations.
 */
typedef std::function<bool()> TaskOnEnable;
#else
/**
 * @typedef TaskCallback
 * @brief Main task callback function pointer type
 * @details Function called periodically when task is executed. Should be non-blocking
 * and return quickly to maintain cooperative multitasking behavior.
 */
typedef void (*TaskCallback)();

/**
 * @typedef TaskOnDisable
 * @brief Task disable callback function pointer type
 * @details Function called when task is disabled. Used for cleanup operations.
 */
typedef void (*TaskOnDisable)();

/**
 * @typedef TaskOnEnable
 * @brief Task enable callback function pointer type
 * @details Function called when task is enabled. Should return true to enable task,
 * false to keep task disabled. Used for initialization operations.
 */
typedef bool (*TaskOnEnable)();
#endif  // _TASK_STD_FUNCTION
/** @} */ // End of CallbackTypes group


#ifdef _TASK_SLEEP_ON_IDLE_RUN
  /**
   * @typedef SleepCallback
   * @brief Sleep function callback type
   * @param aDuration Duration to sleep in milliseconds
   * @details Function pointer type for custom sleep implementations during idle periods.
   */
  typedef void (*SleepCallback)( unsigned long aDuration );

  /**
   * @brief Global pointer to scheduler that controls sleep functionality
   */
  extern Scheduler* iSleepScheduler;

  /**
   * @brief Global pointer to custom sleep method
   */
  extern SleepCallback iSleepMethod;
#endif  // _TASK_SLEEP_ON_IDLE_RUN

/**
 * @struct _task_status
 * @brief Internal task status structure
 * @details Bit-packed structure containing task state flags for efficient memory usage.
 * This structure tracks various task states and conditions.
 */
typedef struct  {
    bool  enabled       : 1;           ///< Indicates that task is enabled or not
    bool  inonenable    : 1;           ///< Indicates that task execution is inside OnEnable method (preventing infinite loops)
    bool  canceled      : 1;           ///< Indication that task has been canceled prior to normal end of all iterations or regular call to disable()
#ifdef _TASK_SELF_DESTRUCT
    bool  selfdestruct  : 1;           ///< Indication that task has been requested to self-destruct on disable
    bool  sd_request    : 1;           ///< Request for scheduler to delete task object and take task out of the queue
#endif  // _TASK_SELF_DESTRUCT
#ifdef _TASK_STATUS_REQUEST
    uint8_t  waiting    : 2;           ///< Indication if task is waiting on the status request
#endif  // _TASK_STATUS_REQUEST

#ifdef _TASK_TIMEOUT
    bool  timeout       : 1;           ///< Indication if task timed out
#endif  //  _TASK_TIMEOUT
} _task_status;


/**
 * @class Task
 * @brief Core task class for cooperative multitasking
 *
 * @details A "Task" is an action, a part of the program logic, which requires scheduled execution.
 * A concept of Task combines the following aspects:
 * - Program code performing specific activities (callback methods)
 * - Execution interval
 * - Number of execution iterations
 * - (Optionally) Execution start event (Status Request)
 * - (Optionally) Pointer to a Local Task Storage area
 *
 * Tasks perform certain functions, which could require periodic or one-time execution,
 * update of specific variables, or waiting for specific events. Tasks also could be
 * controlling specific hardware, or triggered by hardware interrupts.
 *
 * For execution purposes Tasks are linked into execution chains, which are processed
 * by the Scheduler in the order they were added (linked together).
 *
 * Key Features:
 * - Cooperative multitasking (non-preemptive)
 * - Event-driven execution via StatusRequest objects
 * - Dynamic parameter adjustment during runtime
 * - Support for callback chaining and state machines
 * - Built-in timeout and watchdog functionality
 * - Local task storage for task-specific data
 *
 * @note Tasks are responsible for supporting cooperative multitasking by being "good neighbors",
 * i.e., running their callback methods quickly in a non-blocking way, and releasing control
 * back to the scheduler as soon as possible.
 *
 * @see Scheduler, StatusRequest
 */
class Task {
  friend class Scheduler;
  public:

    /**
     * @brief Task constructor with parameters
     * @param aInterval Execution interval in milliseconds (or microseconds if _TASK_MICRO_RES enabled) (default = 0)
     * @param aIterations Number of iterations, -1 for indefinite execution (default = 0)
     * @param aScheduler Optional reference to existing scheduler. If supplied (not NULL) this task will be appended to the task chain (default = NULL)
     * @param aEnable Optional. Value of true will create task enabled (default = false)
     *
     * @details Creates a task that is scheduled to run every aInterval milliseconds, aIterations times.
     * All tasks are created disabled by default (unless aEnable = true). You have to explicitly enable
     * the task for execution.
     *
     * @note Tasks do not remember the number of iterations set initially. After the iterations are done,
     * internal iteration counter is 0. If you need to perform another set of iterations, you need to set
     * the number of iterations again.
     *
     * @note Tasks which performed all their iterations remain active.
     */
#ifdef _TASK_OO_CALLBACKS
    __TASK_INLINE Task(unsigned long aInterval=0, long aIterations=0, Scheduler* aScheduler=NULL, bool aEnable=false
#ifdef _TASK_SELF_DESTRUCT
    , bool aSelfDestruct=false);
#else
    );
#endif  // #ifdef _TASK_SELF_DESTRUCT
#else
    /**
     * @brief Task constructor with callback parameters
     * @param aInterval Execution interval in milliseconds (or microseconds if _TASK_MICRO_RES enabled) (default = 0)
     * @param aIterations Number of iterations, -1 for indefinite execution (default = 0)
     * @param aCallback Pointer to the void callback method without parameters (default = NULL)
     * @param aScheduler Optional reference to existing scheduler. If supplied (not NULL) this task will be appended to the task chain (default = NULL)
     * @param aEnable Optional. Value of true will create task enabled (default = false)
     * @param aOnEnable Pointer to the bool OnEnable callback method without parameters, invoked when task is enabled (default = NULL)
     * @param aOnDisable Pointer to the void OnDisable method without parameters, invoked when task is disabled (default = NULL)
     *
     * @details Creates a task with callback methods. OnEnable method returns true if task should be enabled,
     * false if it should remain disabled. OnDisable method is called when task is disabled.
     *
     * @note OnEnable callback method is called immediately when task is enabled, which could be well ahead
     * of the scheduled execution time of the task. It is advisable to explicitly enable tasks with OnEnable
     * methods after all initialization methods completed (e.g., at the end of setup() method).
     */
    __TASK_INLINE Task(unsigned long aInterval=0, long aIterations=0, TaskCallback aCallback=NULL, Scheduler* aScheduler=NULL, bool aEnable=false, TaskOnEnable aOnEnable=NULL, TaskOnDisable aOnDisable=NULL
#ifdef _TASK_SELF_DESTRUCT
  , bool aSelfDestruct=false);
#else
  );
#endif  // #ifdef _TASK_SELF_DESTRUCT
#endif // _TASK_OO_CALLBACKS


#ifdef _TASK_STATUS_REQUEST
#ifdef _TASK_OO_CALLBACKS
    /**
     * @brief Constructor for StatusRequest-based tasks (Object-Oriented callbacks)
     * @param aScheduler Pointer to scheduler
     * @details Creates a Task for activation on event. Such tasks must run waitFor() method,
     * their interval, iteration and enabled status will be set by that method.
     */
    __TASK_INLINE Task(Scheduler* aScheduler);
#else
    /**
     * @brief Constructor for StatusRequest-based tasks (Function pointer callbacks)
     * @param aCallback Pointer to callback function
     * @param aScheduler Pointer to scheduler
     * @param aOnEnable Pointer to OnEnable callback (default = NULL)
     * @param aOnDisable Pointer to OnDisable callback (default = NULL)
     * @details Creates a Task for activation on event. Such tasks must run waitFor() method,
     * their interval, iteration and enabled status will be set by that method.
     */
    __TASK_INLINE Task(TaskCallback aCallback, Scheduler* aScheduler, TaskOnEnable aOnEnable=NULL, TaskOnDisable aOnDisable=NULL);
#endif // _TASK_OO_CALLBACKS
#endif  // _TASK_STATUS_REQUEST

    /**
     * @brief Virtual destructor
     * @details Properly destroys the task object and cleans up resources.
     */
    virtual __TASK_INLINE ~Task();

#ifdef _TASK_TIMEOUT
    /**
     * @brief Set overall task timeout
     * @param aTimeout Timeout period in milliseconds (or microseconds)
     * @param aReset Whether to reset timeout counter (default = false)
     * @details Any task can be set to time out after a certain period of time.
     * Timeout can be reset, so it can be used as an individual Task's watchdog timer.
     */
    __TASK_INLINE void setTimeout(unsigned long aTimeout, bool aReset=false);

    /**
     * @brief Reset timeout counter
     * @details Resets the timeout counter to start counting from the beginning.
     */
    __TASK_INLINE void resetTimeout();

    /**
     * @brief Get timeout value
     * @return Current timeout value in milliseconds (or microseconds)
     */
    __TASK_INLINE unsigned long getTimeout();

    /**
     * @brief Get time until timeout
     * @return Number of milliseconds (or microseconds) until timeout occurs
     * @details Returns negative value if timeout has already occurred.
     */
    __TASK_INLINE long untilTimeout();

    /**
     * @brief Check if task has timed out (deprecated)
     * @return true if task has timed out
     * @deprecated Use isTimedOut() instead
     */
    __TASK_INLINE bool timedOut() { return isTimedOut(); };

    /**
     * @brief Check if task has timed out
     * @return true if task has timed out
     */
    __TASK_INLINE bool isTimedOut();
#endif

    /**
     * @brief Enable the task
     * @return true if task was successfully enabled
     * @details Enables the task and schedules it for immediate execution (without delay) at this or next
     * scheduling pass. Scheduler will execute the next pass without any delay because there is a task
     * which was enabled and requires execution.
     *
     * @note If task being enabled is not assigned to a scheduler and is not part of execution chain,
     * then task will not be enabled.
     *
     * @note enable() invokes task's OnEnable method (if not NULL) immediately, which can prepare task
     * for execution. OnEnable must return true for task to be enabled. If OnEnable returns false,
     * task remains disabled.
     *
     * @warning OnEnable is invoked every time enable() is called, regardless if task is already enabled.
     * Alignment to current millis() is performed after OnEnable exits.
     */
    __TASK_INLINE bool  enable();

    /**
     * @brief Enable the task only if it was previously disabled
     * @return Previous enable state: true if task was already enabled, false if task was disabled
     * @details Since enable() schedules Task for execution immediately, this method provides a way
     * to activate tasks and schedule them for immediate execution only if they are not active already.
     * All NOTES from the enable() method apply.
     */
    __TASK_INLINE bool  enableIfNot();

    /**
     * @brief Enable the task with delay
     * @param aDelay Delay before first execution in milliseconds (default = 0)
     * @return true if task was successfully enabled
     * @details Enables the task and schedules it for execution after a specific delay.
     */
    __TASK_INLINE bool  enableDelayed(unsigned long aDelay=0);

    /**
     * @brief Restart the task
     * @return true if task was successfully restarted
     * @details For tasks with limited number of iterations only, restart method will re-enable the task,
     * set the number of iterations back to last set value, and schedule task for execution as soon as possible.
     */
    __TASK_INLINE bool  restart();

    /**
     * @brief Restart the task with delay
     * @param aDelay Delay before first execution in milliseconds (default = 0)
     * @return true if task was successfully restarted
     * @details Same as restart() method, with the only difference being that Task is scheduled to run
     * first iteration after a delay.
     */
    __TASK_INLINE bool  restartDelayed(unsigned long aDelay=0);

    /**
     * @brief Delay the task execution
     * @param aDelay Delay in milliseconds (default = 0)
     * @details Schedules the task for execution after a delay, but does not change the enabled/disabled
     * status of the task. A delay of 0 (zero) will delay task for current execution interval.
     * Use forceNextIteration() method to force execution during immediate next scheduling pass.
     */
    __TASK_INLINE void  delay(unsigned long aDelay=0);

    /**
     * @brief Adjust task interval
     * @param aInterval New interval value (can be negative for adjustment)
     * @details Adjusts the task execution interval by the specified amount.
     */
    __TASK_INLINE void  adjust(long aInterval);

    /**
     * @brief Force next iteration to execute immediately
     * @details Schedules the task for execution during immediate next scheduling pass.
     * The Task must be already enabled prior to this method. Task's schedule is adjusted
     * to run from this moment in time.
     *
     * @note If a task was running every 10 seconds: 10, 20, 30, calling forceNextIteration
     * at 44th second will make subsequent schedule look like: 44, 54, 64, 74, ..
     */
    __TASK_INLINE void  forceNextIteration();

    /**
     * @brief Disable the task
     * @return Previous enabled state: true if task was enabled, false otherwise
     * @details Scheduler will not execute this task any longer, even if it remains in the chain.
     * Task can be later re-enabled for execution. If not NULL, task's OnDisable method is invoked
     * almost immediately (actually during the next scheduling pass). 
     * OnDisable is invoked only if task was in enabled state.
     */
    __TASK_INLINE bool  disable();

    /**
     * @brief Abort the task
     * @details Immediately stops task execution and marks it as aborted.
     * Different from disable() in that it indicates abnormal termination.
     * OnDisable method is NOT invoked for Aborted tasks.
     */
    __TASK_INLINE void  abort();

    /**
     * @brief Cancel the task
     * @details Marks the task as cancelled, indicating it was stopped before normal completion.
     * OnDisable method is invoked if task was in enabled state.
     */
    __TASK_INLINE void  cancel();

    /**
     * @brief Check if task is enabled
     * @return true if task is enabled
     */
    __TASK_INLINE bool  isEnabled();

    /**
     * @brief Check if task is cancelled
     * @return true if task has been cancelled
     */
    __TASK_INLINE bool  isCanceled();

    /**
     * @brief Check if task is cancelled (deprecated)
     * @return true if task has been cancelled
     * @deprecated Use isCanceled() instead
     */
    __TASK_INLINE bool  canceled() { return isCanceled(); };

#ifdef _TASK_SCHEDULING_OPTIONS
    /**
     * @brief Get the current scheduling option for this task
     *
     * Returns the current scheduling behavior option that determines how the task's
     * interval and iterations are managed during execution.
     *
     * Available scheduling options:
     * - TASK_SCHEDULE: Default behavior - interval countdown starts after OnEnable
     * - TASK_SCHEDULE_NC: No countdown - interval countdown starts immediately upon enable
     * - TASK_INTERVAL: Interval mode - maintains exact intervals regardless of execution time
     *
     * @return unsigned int Current scheduling option value (TASK_SCHEDULE, TASK_SCHEDULE_NC, or TASK_INTERVAL)
     *
     * @note This method is only available when _TASK_SCHEDULING_OPTIONS compile-time option is enabled
     * @see setSchedulingOption(), TASK_SCHEDULE, TASK_SCHEDULE_NC, TASK_INTERVAL
     * @since Version 3.0.0
     */
    __TASK_INLINE unsigned int getSchedulingOption() { return iOption; }

    /**
     * @brief Set the scheduling option for this task
     *
     * Sets the scheduling behavior option that determines how the task's interval
     * and iterations are managed during execution. This affects when the interval
     * countdown begins and how timing is calculated.
     *
     * Scheduling options:
     * - TASK_SCHEDULE: (Default) Interval countdown starts after OnEnable callback completes
     * - TASK_SCHEDULE_NC: No countdown - interval countdown starts immediately when task is enabled
     * - TASK_INTERVAL: Interval mode - maintains exact intervals by accounting for execution time
     *
     * @param aOption The scheduling option to set (TASK_SCHEDULE, TASK_SCHEDULE_NC, or TASK_INTERVAL)
     *
     * @note This method is only available when _TASK_SCHEDULING_OPTIONS compile-time option is enabled
     * @note The scheduling option can be changed at any time, even when the task is running
     * @warning Using TASK_INTERVAL may cause timing drift if task execution time exceeds the interval
     * @see getSchedulingOption(), TASK_SCHEDULE, TASK_SCHEDULE_NC, TASK_INTERVAL
     * @since Version 3.0.0
     */
    __TASK_INLINE void setSchedulingOption(unsigned int aOption) {  iOption = aOption; }
#endif  //_TASK_SCHEDULING_OPTIONS

#ifdef _TASK_OO_CALLBACKS
    /**
     * @brief Set task parameters (Object-Oriented callbacks)
     * @param aInterval Execution interval in milliseconds (or microseconds)
     * @param aIterations Number of iterations (-1 for infinite)
     * @details Allows dynamic control of task execution parameters in one method call.
     */
    __TASK_INLINE void set(unsigned long aInterval, long aIterations);
#else
    /**
     * @brief Set task parameters (Function pointer callbacks)
     * @param aInterval Execution interval in milliseconds (or microseconds)
     * @param aIterations Number of iterations (-1 for infinite)
     * @param aCallback Pointer to callback function
     * @param aOnEnable Pointer to OnEnable callback (default = NULL)
     * @param aOnDisable Pointer to OnDisable callback (default = NULL)
     * @details Allows dynamic control of task execution parameters in one method call.
     * OnEnable and OnDisable parameters can be omitted (assigned to NULL).
     */
    __TASK_INLINE void set(unsigned long aInterval, long aIterations, TaskCallback aCallback,TaskOnEnable aOnEnable=NULL, TaskOnDisable aOnDisable=NULL);
#endif // _TASK_OO_CALLBACKS

    /**
     * @brief Set task execution interval
     * @param aInterval New interval in milliseconds (or microseconds)
     * @details Next execution time calculation takes place after the callback method is called,
     * so new interval will be used immediately by the scheduler. Execution is delayed by the
     * provided interval. If immediate invocation is required, call forceNextIteration() after
     * setting new interval or use setIntervalNoDelay() method.
     */
    __TASK_INLINE void setInterval(unsigned long aInterval);

    /**
     * @brief Set task interval without delay
     * @param aInterval New interval in milliseconds (or microseconds)
     * @param aOption How to handle the interval change (default = TASK_INTERVAL_KEEP)
     * TASK_INTERVAL_KEEP - keep the current delay, new interval will be used after current delay expires
     * TASK_INTERVAL_RECALC - recalculate next execution time based on new interval
     * TASK_INTERVAL_RESET - reset schedule, next execution time and new interval are updated
     * @details Sets new interval and provides options for how to handle the timing change.
     */
    __TASK_INLINE void setIntervalNodelay(unsigned long aInterval, unsigned int aOption = TASK_INTERVAL_KEEP);

    /**
     * @brief Get current task interval
     * @return Current execution interval in milliseconds (or microseconds)
     */
    __TASK_INLINE unsigned long getInterval();

    /**
     * @brief Set number of iterations
     * @param aIterations Number of iterations (-1 for infinite)
     * @details Tasks that ran through all their allocated iterations are disabled.
     * SetIterations() method DOES NOT enable the task. Either enable explicitly,
     * or use restart methods.
     */
    __TASK_INLINE void setIterations(long aIterations);

    /**
     * @brief Get remaining iterations
     * @return Number of remaining iterations
     */
    __TASK_INLINE long getIterations();

    /**
     * @brief Get run counter
     * @return Number of times callback method has been invoked since last enable
     * @details The runCounter value is incremented before callback method is invoked.
     * If a task is checking the runCounter value within its callback method, then
     * the first run value is 1.
     */
    __TASK_INLINE unsigned long getRunCounter();
    
#ifdef _TASK_SELF_DESTRUCT
    /**
     * @brief Enable or disable self-destruction for this task
     *
     * When self-destruction is enabled, the task will automatically delete itself
     * from the scheduler after it completes its final iteration. This is useful
     * for one-time or temporary tasks that should clean themselves up automatically.
     *
     * Self-destruction behavior:
     * - Task removes itself from the scheduler when it finishes its last iteration
     * - Applies to tasks with finite iterations (setIterations() > 0)
     * - Does not apply to infinite tasks (TASK_FOREVER iterations)
     * - Task object memory is not freed - only removed from scheduler
     *
     * @param aSelfDestruct True to enable self-destruction, false to disable (default: true)
     *
     * @note This method is only available when _TASK_SELF_DESTRUCT compile-time option is enabled
     * @note Self-destruction only occurs after the task naturally completes its iterations
     * @note Manually disabled tasks will not self-destruct
     * @warning Ensure task object remains valid until self-destruction occurs
     * @see getSelfDestruct(), setIterations(), disable()
     * @since Version 3.0.0
     */
    __TASK_INLINE void setSelfDestruct(bool aSelfDestruct=true) { iStatus.selfdestruct = aSelfDestruct; }

    /**
     * @brief Check if self-destruction is enabled for this task
     *
     * Returns whether the task is configured to automatically remove itself from
     * the scheduler after completing its final iteration.
     *
     * @return bool True if self-destruction is enabled, false otherwise
     *
     * @note This method is only available when _TASK_SELF_DESTRUCT compile-time option is enabled
     * @see setSelfDestruct(), setIterations()
     * @since Version 3.0.0
     */
    __TASK_INLINE bool getSelfDestruct() { return iStatus.selfdestruct; }
#endif  //  #ifdef _TASK_SELF_DESTRUCT

#ifdef _TASK_OO_CALLBACKS
    virtual __TASK_INLINE bool Callback() =0;  // return true if run was "productive - this will disable sleep on the idle run for next pass
    virtual __TASK_INLINE bool OnEnable();  // return true if task should be enabled, false if it should remain disabled
    virtual __TASK_INLINE void OnDisable();
#else
    __TASK_INLINE void setCallback(TaskCallback aCallback) ;
    __TASK_INLINE void setOnEnable(TaskOnEnable aCallback) ;
    __TASK_INLINE void setOnDisable(TaskOnDisable aCallback) ;
    __TASK_INLINE void yield(TaskCallback aCallback);
    __TASK_INLINE void yieldOnce(TaskCallback aCallback);
#endif // _TASK_OO_CALLBACKS

    /**
     * @brief Check if this is the first iteration
     * @return true if current pass is (or will be) a first iteration of the task
     */
    __TASK_INLINE bool isFirstIteration() ;

    /**
     * @brief Check if this is the last iteration
     * @return true if current pass is the last iteration (for tasks with limited iterations only)
     */
    __TASK_INLINE bool isLastIteration() ;

#ifdef _TASK_TIMECRITICAL
    __TASK_INLINE long getOverrun() ;
    __TASK_INLINE long getStartDelay() ;
#endif  // _TASK_TIMECRITICAL

#ifdef _TASK_STATUS_REQUEST
    __TASK_INLINE bool waitFor(StatusRequest* aStatusRequest, unsigned long aInterval = 0, long aIterations = 1);
    __TASK_INLINE bool waitForDelayed(StatusRequest* aStatusRequest, unsigned long aInterval = 0, long aIterations = 1);
    __TASK_INLINE StatusRequest* getStatusRequest() ;
    __TASK_INLINE StatusRequest* getInternalStatusRequest() ;
#endif  // _TASK_STATUS_REQUEST

#ifdef _TASK_WDT_IDS
    __TASK_INLINE void setId(unsigned int aID) ;
    __TASK_INLINE unsigned int getId() ;
    __TASK_INLINE void setControlPoint(unsigned int aPoint) ;
    __TASK_INLINE unsigned int getControlPoint() ;
#endif  // _TASK_WDT_IDS

#ifdef _TASK_LTS_POINTER
    __TASK_INLINE void  setLtsPointer(void *aPtr) ;
    __TASK_INLINE void* getLtsPointer() ;
#endif  // _TASK_LTS_POINTER

#ifdef _TASK_EXPOSE_CHAIN
    __TASK_INLINE Task*  getPreviousTask() { return iPrev; };  // pointer to the previous task in the chain, NULL if first or not set
    __TASK_INLINE Task*  getNextTask()     { return iNext; };  // pointer to the next task in the chain, NULL if last or not set
#endif // _TASK_EXPOSE_CHAIN

  _TASK_SCOPE:
    __TASK_INLINE void reset();

    volatile _task_status     iStatus;
    volatile unsigned long    iInterval;             // execution interval in milliseconds (or microseconds). 0 - immediate
    volatile unsigned long    iDelay;                // actual delay until next execution (usually equal iInterval)
    volatile unsigned long    iPreviousMillis;       // previous invocation time (millis).  Next invocation = iPreviousMillis + iInterval.  Delayed tasks will "catch up"

#ifdef _TASK_SCHEDULING_OPTIONS
    unsigned int              iOption;               // scheduling option
#endif  // _TASK_SCHEDULING_OPTIONS

#ifdef _TASK_TIMECRITICAL
    volatile long             iOverrun;              // negative if task is "catching up" to it's schedule (next invocation time is already in the past)
    volatile long             iStartDelay;           // actual execution of the task's callback method was delayed by this number of millis
#endif  // _TASK_TIMECRITICAL

    volatile long             iIterations;           // number of iterations left. 0 - last iteration. -1 - infinite iterations
    long                      iSetIterations;        // number of iterations originally requested (for restarts)
    unsigned long             iRunCounter;           // current number of iteration (starting with 1). Resets on enable.

#ifndef _TASK_OO_CALLBACKS
    TaskCallback              iCallback;             // pointer to the void callback method
    TaskOnEnable              iOnEnable;             // pointer to the bool OnEnable callback method
    TaskOnDisable             iOnDisable;            // pointer to the void OnDisable method
#endif // _TASK_OO_CALLBACKS

    Task                     *iPrev, *iNext;         // pointers to the previous and next tasks in the chain
    Scheduler                *iScheduler;            // pointer to the current scheduler

#ifdef _TASK_STATUS_REQUEST
    StatusRequest            *iStatusRequest;        // pointer to the status request task is or was waiting on
    StatusRequest             iMyStatusRequest;      // internal Status request to let other tasks know of completion
#endif  // _TASK_STATUS_REQUEST

#ifdef _TASK_WDT_IDS
    unsigned int              iTaskID;               // task ID (for debugging and watchdog identification)
    unsigned int              iControlPoint;         // current control point within the callback method. Reset to 0 by scheduler at the beginning of each pass
#endif  // _TASK_WDT_IDS

#ifdef _TASK_LTS_POINTER
    void                     *iLTS;                  // pointer to task's local storage. Needs to be recast to appropriate type (usually a struct).
#endif  // _TASK_LTS_POINTER

#ifdef _TASK_TIMEOUT
    unsigned long            iTimeout;               // Task overall timeout
    unsigned long            iStarttime;             // millis at task start time
#endif // _TASK_TIMEOUT
};

/**
 * @class Scheduler
 * @brief Task scheduler for cooperative multitasking
 *
 * @details The Scheduler class manages the execution of tasks in a cooperative multitasking environment.
 * It maintains a chain of tasks and executes them according to their schedules and priorities.
 *
 * Key features:
 * - Cooperative multitasking (non-preemptive)
 * - Task chain management
 * - Priority layer support
 * - Sleep mode support for power saving
 * - Thread safety options
 * - Performance monitoring
 *
 * The scheduler executes Tasks' callback methods in the order the tasks were added to the chain,
 * from first to last. Scheduler stops and exits after processing the chain once in order to allow
 * other statements in the main code of loop() method to run. This is referred to as a "scheduling pass".
 *
 * @note Normally, there is no need to have any other statements in the loop() method other than
 * the Scheduler's execute() method.
 *
 * @see Task, StatusRequest
 */
class Scheduler {
  friend class Task;
  public:
    /**
     * @brief Default constructor
     * @details Creates task scheduler with default parameters and an empty task queue.
     */
    __TASK_INLINE Scheduler();

    /**
     * @brief Initialize the scheduler
     * @details Initializes the task queue and scheduler parameters. Executed as part of constructor,
     * so doesn't need to be explicitly called after creation.
     *
     * @note By default scheduler is allowed to put processor to IDLE sleep mode. If this behavior
     * was changed via allowSleep() method, init() will NOT reset the sleep parameter.
     */
    __TASK_INLINE void init();

    /**
     * @brief Add task to execution chain
     * @param aTask Reference to task to be added
     * @details Adds task to the execution queue (or chain) of tasks by appending it to the end
     * of the chain. If two tasks are scheduled for execution, the sequence will match the order
     * in which tasks were appended to the chain.
     *
     * @note Currently, changing the execution sequence in a chain dynamically is not supported.
     * If you need to reorder the chain sequence – initialize the scheduler and re-add the tasks
     * in a different order.
     */
    __TASK_INLINE void addTask(Task& aTask);

    /**
     * @brief Delete task from execution chain
     * @param aTask Reference to task to be deleted
     * @details Deletes task from the execution chain. The chain of remaining tasks is linked
     * together. It is not required to delete a task from the chain. A disabled task will not
     * be executed anyway, but you save a few microseconds per scheduling pass by deleting it.
     */
    __TASK_INLINE void deleteTask(Task& aTask);

    /**
     * @brief Pause the scheduler
     * @details Temporarily suspends all task execution. Tasks remain in their current state
     * but will not be executed until resume() is called.
     */
    __TASK_INLINE void pause() { iPaused = true; };

    /**
     * @brief Resume the scheduler
     * @details Resumes task execution after pause().
     */
    __TASK_INLINE void resume() { iPaused = false; };

    /**
     * @brief Enable the scheduler
     * @details Enables the scheduler for task execution.
     */
    __TASK_INLINE void enable() { iEnabled = true; };

    /**
     * @brief Disable the scheduler
     * @details Disables the scheduler, preventing any task execution.
     */
    __TASK_INLINE void disable() { iEnabled = false; };
#ifdef _TASK_PRIORITY
    /**
     * @brief Disable all tasks (with priority support)
     * @param aRecursive If true (default), disable higher priority tasks as well
     * @details Convenient method to disable majority of tasks.
     */
    __TASK_INLINE void disableAll(bool aRecursive = true);

    /**
     * @brief Enable all tasks (with priority support)
     * @param aRecursive If true (default), enable higher priority tasks as well
     * @details Convenient method to enable majority of tasks.
     */
    __TASK_INLINE void enableAll(bool aRecursive = true);

    /**
     * @brief Set all tasks to start immediately (with priority support)
     * @param aRecursive If true (default), affect higher priority tasks as well
     * @details Sets ALL active tasks to start execution immediately. Should be placed
     * at the end of setup() method to prevent task execution race due to long running
     * setup tasks. Any tasks which should execute after a delay should be explicitly
     * delayed after call to startNow() method.
     */
    __TASK_INLINE void startNow(bool aRecursive = true);
#else
    /**
     * @brief Disable all tasks
     * @details Convenient method to disable majority of tasks.
     */
    __TASK_INLINE void disableAll();

    /**
     * @brief Enable all tasks
     * @details Convenient method to enable majority of tasks.
     */
    __TASK_INLINE void enableAll();

    /**
     * @brief Set all tasks to start immediately
     * @details Sets ALL active tasks to start execution immediately. Should be placed
     * at the end of setup() method to prevent task execution race due to long running
     * setup tasks.
     */
    __TASK_INLINE void startNow();
#endif

    /**
     * @brief Execute one scheduling pass
     * @return true if none of the tasks' callback methods was invoked (true = idle run)
     * @details Executes one scheduling pass, including (in case of the base priority scheduler)
     * end-of-pass sleep. This method should be placed inside the loop() method of the sketch.
     * Since execute exits after every pass, you can put additional statements after execute
     * inside the loop().
     *
     * If layered task prioritization is enabled, all higher priority tasks will be evaluated
     * and invoked by the base execute() method. There is no need to call execute() of the
     * higher priority schedulers explicitly.
     *
     * The execute method performs the following steps:
     * 1. Call higher priority scheduler's execute method, if provided
     * 2. Ignore task completely if it is disabled
     * 3. Disable task if it ran out of iterations (calling OnDisable, if necessary)
     * 4. Check if task is waiting on a StatusRequest object, and make appropriate scheduling arrangements
     * 5. Perform necessary timing calculations
     * 6. Invoke task's callback method, if it is time to do so, and one is provided
     * 7. Put microcontroller to sleep (if requested and supported) if none of the tasks were invoked
     *
     * @note Schedule-related calculations are performed prior to task's callback method invocation.
     * This allows tasks to manipulate their runtime parameters (like execution interval) directly.
     */
    __TASK_INLINE bool execute();

    /**
     * @brief Get reference to currently executing task (deprecated)
     * @return Reference to the currently active task
     * @deprecated Use getCurrentTask() instead
     */
    __TASK_INLINE Task& currentTask() ;

    /**
     * @brief Get pointer to currently executing task
     * @return Pointer to the currently active task
     * @details Returns pointer to the task currently executing via execute() loop OR for
     * OnEnable and OnDisable methods, pointer to the task being enabled or disabled.
     * This distinction is important because one task can activate another.
     * Could be used by callback methods to identify which Task invoked this callback method.
     */
    __TASK_INLINE Task* getCurrentTask() ;

    /**
     * @brief Get time until next iteration of a task
     * @param aTask Reference to the task to query
     * @return Number of milliseconds (or microseconds) until next scheduled iteration
     * @details Returns 0 if next iteration is already due (or overdue).
     * Returns -1 if a Task is not active or waiting on an event, and next iteration
     * runtime cannot be determined.
     */
    __TASK_INLINE long timeUntilNextIteration(Task& aTask);

    /**
     * @brief Get number of active tasks
     * @return Number of currently active (enabled) tasks
     */
    __TASK_INLINE unsigned long getActiveTasks() { return iActiveTasks; }

    /**
     * @brief Get total number of tasks
     * @return Total number of tasks in the scheduler
     */
    __TASK_INLINE unsigned long getTotalTasks() { return iTotalTasks; }

    /**
     * @brief Get number of invoked tasks
     * @return Number of tasks that were invoked during the last execute() pass
     */
    __TASK_INLINE unsigned long getInvokedTasks() { return iInvokedTasks; }

#ifdef _TASK_TICKLESS
    /**
     * @brief Get next run time (tickless mode)
     * @return Next scheduled run time
     * @details Available only when compiled with _TASK_TICKLESS support.
     */
    __TASK_INLINE unsigned long getNextRun() { return iNextRun; }
#endif

#ifdef _TASK_SLEEP_ON_IDLE_RUN
    __TASK_INLINE void allowSleep(bool aState = true);
    __TASK_INLINE void setSleepMethod( SleepCallback aCallback );
#endif  // _TASK_SLEEP_ON_IDLE_RUN

#ifdef _TASK_LTS_POINTER
    __TASK_INLINE void* currentLts();
#endif  // _TASK_LTS_POINTER

#ifdef _TASK_TIMECRITICAL
    __TASK_INLINE bool isOverrun();
    __TASK_INLINE void cpuLoadReset();
    __TASK_INLINE unsigned long getCpuLoadCycle(){ return iCPUCycle; };
    __TASK_INLINE unsigned long getCpuLoadIdle() { return iCPUIdle; };
    __TASK_INLINE unsigned long getCpuLoadTotal();
#endif  // _TASK_TIMECRITICAL

#ifdef _TASK_PRIORITY
    __TASK_INLINE void setHighPriorityScheduler(Scheduler* aScheduler);
    __TASK_INLINE static Scheduler& currentScheduler() { return *(iCurrentScheduler); };
#endif  // _TASK_PRIORITY

#ifdef _TASK_EXPOSE_CHAIN
    __TASK_INLINE Task*  getFirstTask() { return iFirst; };       // pointer to the previous task in the chain, NULL if first or not set
    __TASK_INLINE Task*  getLastTask()  { return iLast;  };       // pointer to the next task in the chain, NULL if last or not set
#endif // _TASK_EXPOSE_CHAIN

#ifdef _TASK_THREAD_SAFE
    __TASK_INLINE bool   requestAction(_task_request_t* aRequest);    // this method places the request on the task request queue
    __TASK_INLINE bool   requestAction(void* aObject, _task_request_type_t aType, unsigned long aParam1, unsigned long aParam2, unsigned long aParam3, unsigned long aParam4, unsigned long aParam5);
#endif
  _TASK_SCOPE:
#ifdef _TASK_THREAD_SAFE
    __TASK_INLINE void   processRequests();
#endif
    Task          *iFirst, *iLast, *iCurrent;        // pointers to first, last and current tasks in the chain

    volatile bool iPaused, iEnabled;
    unsigned long iActiveTasks;
    unsigned long iTotalTasks;
    unsigned long iInvokedTasks;

#ifdef _TASK_SLEEP_ON_IDLE_RUN
    bool          iAllowSleep;                      // indication if putting MC to IDLE_SLEEP mode is allowed by the program at this time.
#endif  // _TASK_SLEEP_ON_IDLE_RUN

#ifdef _TASK_PRIORITY
    Scheduler*    iHighPriority;                    // Pointer to a higher priority scheduler
#endif  // _TASK_PRIORITY

#ifdef _TASK_TIMECRITICAL
    unsigned long iCPUStart;
    unsigned long iCPUCycle;
    unsigned long iCPUIdle;
#endif  // _TASK_TIMECRITICAL

#ifdef _TASK_TICKLESS
    unsigned long iNextRun;
#endif
};


#endif /* _TASKSCHEDULERDECLARATIONS_H_ */
