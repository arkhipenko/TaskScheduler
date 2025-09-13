// Cooperative multitasking library for Arduino
// Copyright (c) 2015-2023 Anatoli Arkhipenko

#ifndef _TASKSCHEDULERDECLARATIONS_H_
#define _TASKSCHEDULERDECLARATIONS_H_

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
// #define _TASK_SELF_DESTRUCT      // Enable tasks to "self-destruct" after disable
// #define _TASK_TICKLESS           // Enable support for tickless sleep on FreeRTOS
// #define _TASK_DO_NOT_YIELD       // Disable yield() method in execute() for ESP chips
// #define _TASK_ISR_SUPPORT        // for esp chips - place control methods in IRAM
// #define _TASK_NON_ARDUINO        // for non-arduino use
// #define _TASK_HEADER_AND_CPP     // compile CPP file (non-Arduino IDE platforms)
// #define _TASK_THREAD_SAFE        // Enable additional checking for thread safety

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

class Scheduler;

#define TASK_SCHEDULE       0   // default
#define TASK_SCHEDULE_NC    1   // schedule + no catch-ups (always in the future)
#define TASK_INTERVAL       2   // interval (always in the future)

#ifdef _TASK_DEBUG
    #define _TASK_SCOPE  public
#else
    #define _TASK_SCOPE  private
#endif

//  task scheduling iteration common options
#define TASK_IMMEDIATE          0
#define TASK_FOREVER         (-1)
#define TASK_ONCE               1

//  options for setIntervalNodelay() method
#define TASK_INTERVAL_KEEP      0
#define TASK_INTERVAL_RECALC    1
#define TASK_INTERVAL_RESET     2

#ifdef _TASK_TIMEOUT
#define TASK_NOTIMEOUT          0
#endif

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

#ifndef _TASK_MICRO_RES

#define TASK_MILLISECOND       1UL
#define TASK_SECOND         1000UL
#define TASK_MINUTE        60000UL
#define TASK_HOUR        3600000UL

#else

#define TASK_MILLISECOND    1000UL
#define TASK_SECOND      1000000UL
#define TASK_MINUTE     60000000UL
#define TASK_HOUR     3600000000UL

#endif  // _TASK_MICRO_RES

#ifdef _TASK_TICKLESS
#define _TASK_NEXTRUN_UNDEFINED 0b0
#define _TASK_NEXTRUN_IMMEDIATE 0b1
#define _TASK_NEXTRUN_TIMED     0x10
#endif  //  _TASK_TICKLESS

#ifdef _TASK_THREAD_SAFE

typedef enum {
#ifdef _TASK_STATUS_REQUEST
    TASK_SR_REQUEST_SETWAITING,
    TASK_SR_REQUEST_SIGNAL,
    TASK_SR_REQUEST_SIGNALCOMPLETE,

#ifdef _TASK_TIMEOUT
    TASK_SR_REQUEST_SETTIMEOUT,
    TASK_SR_REQUEST_RESETTIMEOUT,
#endif  // _TASK_TIMEOUT

#endif  // _TASK_STATUS_REQUEST

#ifdef _TASK_LTS_POINTER
    TASK_REQUEST_SETLTSPOINTER,
#endif

#ifdef _TASK_SELF_DESTRUCT
    TASK_REQUEST_SETSELFDESTRUCT,
#endif  // _TASK_SELF_DESTRUCT

#ifdef _TASK_SCHEDULING_OPTIONS
    TASK_REQUEST_SETSCHEDULINGOPTION,
#endif  // _TASK_SCHEDULING_OPTIONS

#ifdef _TASK_TIMEOUT
    TASK_REQUEST_SETTIMEOUT,
    TASK_REQUEST_RESETTIMEOUT,
#endif  // _TASK_TIMEOUT

#ifdef _TASK_STATUS_REQUEST
    TASK_REQUEST_WAITFOR,
    TASK_REQUEST_WAITFORDELAYED,
#endif  // _TASK_STATUS_REQUEST

#ifdef _TASK_WDT_IDS
    TASK_REQUEST_SETID,
    TASK_REQUEST_SETCONTROLPOINT,
#endif  // _TASK_WDT_IDS

    TASK_REQUEST_ENABLE,
    TASK_REQUEST_ENABLEIFNOT,
    TASK_REQUEST_ENABLEDELAYED,
    TASK_REQUEST_RESTART,
    TASK_REQUEST_RESTARTDELAYED,
    TASK_REQUEST_DELAY,
    TASK_REQUEST_ADJUST,
    TASK_REQUEST_FORCENEXTITERATION,
    TASK_REQUEST_DISABLE,
    TASK_REQUEST_ABORT,
    TASK_REQUEST_CANCEL,
    TASK_REQUEST_SET,
    TASK_REQUEST_SETINTERVAL,
    TASK_REQUEST_SETINTERVALNODELAY,
    TASK_REQUEST_SETITERATIONS,
    TASK_REQUEST_SETCALLBACK,
    TASK_REQUEST_SETONENABLE,
    TASK_REQUEST_SETONDISABLE
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


typedef struct {
    _task_request_type_t req_type;
    void*           object_ptr;
    unsigned long   param1;
    unsigned long   param2;
    unsigned long   param3;
    unsigned long   param4;
    unsigned long   param5;
} _task_request_t;

#endif  //_TASK_THREAD_SAFE

#ifdef _TASK_STATUS_REQUEST

#define TASK_SR_OK          0
#define TASK_SR_ERROR       (-1)
#define TASK_SR_CANCEL      (-32766)
#define TASK_SR_ABORT       (-32767)
#define TASK_SR_TIMEOUT     (-32768)
 
#define _TASK_SR_NODELAY    1
#define _TASK_SR_DELAY      2

class StatusRequest {
  friend class Scheduler;
  public:
    __TASK_INLINE StatusRequest();
    __TASK_INLINE void setWaiting(unsigned int aCount = 1);
    __TASK_INLINE bool  signal(int aStatus = 0);
    __TASK_INLINE void  signalComplete(int aStatus = 0);
    __TASK_INLINE bool pending();
    __TASK_INLINE bool completed();
    __TASK_INLINE int  getStatus();
    __TASK_INLINE int  getCount();
    
#ifdef _TASK_TIMEOUT
    __TASK_INLINE void setTimeout(unsigned long aTimeout) { iTimeout = aTimeout; };
    __TASK_INLINE unsigned long getTimeout() { return iTimeout; };
    __TASK_INLINE void resetTimeout();
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

#ifdef _TASK_STD_FUNCTION
#include <functional>
typedef std::function<void()> TaskCallback;
typedef std::function<void()> TaskOnDisable;
typedef std::function<bool()> TaskOnEnable;
#else
typedef void (*TaskCallback)();
typedef void (*TaskOnDisable)();
typedef bool (*TaskOnEnable)();
#endif  // _TASK_STD_FUNCTION


#ifdef _TASK_SLEEP_ON_IDLE_RUN
  typedef void (*SleepCallback)( unsigned long aDuration );

  extern Scheduler* iSleepScheduler;
  extern SleepCallback iSleepMethod;
#endif  // _TASK_SLEEP_ON_IDLE_RUN

typedef struct  {
    bool  enabled       : 1;           // indicates that task is enabled or not.
    bool  inonenable    : 1;           // indicates that task execution is inside OnEnable method (preventing infinite loops)
    bool  canceled      : 1;           // indication that task has been canceled prior to normal end of all iterations or regular call to disable()
#ifdef _TASK_SELF_DESTRUCT
    bool  selfdestruct  : 1;           // indication that task has been requested to self-destruct on disable
    bool  sd_request    : 1;           // request for scheduler to delete task object and take task out of the queue
#endif  // _TASK_SELF_DESTRUCT
#ifdef _TASK_STATUS_REQUEST
    uint8_t  waiting    : 2;           // indication if task is waiting on the status request
#endif  // _TASK_STATUS_REQUEST

#ifdef _TASK_TIMEOUT
    bool  timeout       : 1;           // indication if task timed out
#endif  //  _TASK_TIMEOUT
} _task_status;


class Task {
  friend class Scheduler;
  public:

#ifdef _TASK_OO_CALLBACKS
    __TASK_INLINE Task(unsigned long aInterval=0, long aIterations=0, Scheduler* aScheduler=NULL, bool aEnable=false
#ifdef _TASK_SELF_DESTRUCT
    , bool aSelfDestruct=false);
#else
    );
#endif  // #ifdef _TASK_SELF_DESTRUCT
#else
    __TASK_INLINE Task(unsigned long aInterval=0, long aIterations=0, TaskCallback aCallback=NULL, Scheduler* aScheduler=NULL, bool aEnable=false, TaskOnEnable aOnEnable=NULL, TaskOnDisable aOnDisable=NULL
#ifdef _TASK_SELF_DESTRUCT
  , bool aSelfDestruct=false);
#else
  );
#endif  // #ifdef _TASK_SELF_DESTRUCT
#endif // _TASK_OO_CALLBACKS


#ifdef _TASK_STATUS_REQUEST
#ifdef _TASK_OO_CALLBACKS
//    __TASK_INLINE Task(Scheduler* aScheduler=NULL);
    __TASK_INLINE Task(Scheduler* aScheduler);
#else
//    __TASK_INLINE Task(TaskCallback aCallback=NULL, Scheduler* aScheduler=NULL, TaskOnEnable aOnEnable=NULL, TaskOnDisable aOnDisable=NULL);
    __TASK_INLINE Task(TaskCallback aCallback, Scheduler* aScheduler, TaskOnEnable aOnEnable=NULL, TaskOnDisable aOnDisable=NULL);
#endif // _TASK_OO_CALLBACKS
#endif  // _TASK_STATUS_REQUEST

    virtual __TASK_INLINE ~Task();

#ifdef _TASK_TIMEOUT
    __TASK_INLINE void setTimeout(unsigned long aTimeout, bool aReset=false);
    __TASK_INLINE void resetTimeout();
    __TASK_INLINE unsigned long getTimeout();
    __TASK_INLINE long untilTimeout();
    __TASK_INLINE bool timedOut();
#endif

    __TASK_INLINE bool  enable();
    __TASK_INLINE bool  enableIfNot();
    __TASK_INLINE bool  enableDelayed(unsigned long aDelay=0);
    __TASK_INLINE bool  restart();
    __TASK_INLINE bool  restartDelayed(unsigned long aDelay=0);

    __TASK_INLINE void  delay(unsigned long aDelay=0);
    __TASK_INLINE void adjust(long aInterval);
    __TASK_INLINE void  forceNextIteration();
    __TASK_INLINE bool  disable();
    __TASK_INLINE void  abort();
    __TASK_INLINE void  cancel();
    __TASK_INLINE bool isEnabled();
    __TASK_INLINE bool canceled();

#ifdef _TASK_SCHEDULING_OPTIONS
    __TASK_INLINE unsigned int getSchedulingOption() { return iOption; }
    __TASK_INLINE void setSchedulingOption(unsigned int aOption) {  iOption = aOption; }
#endif  //_TASK_SCHEDULING_OPTIONS

#ifdef _TASK_OO_CALLBACKS
    __TASK_INLINE void set(unsigned long aInterval, long aIterations);
#else
    __TASK_INLINE void set(unsigned long aInterval, long aIterations, TaskCallback aCallback,TaskOnEnable aOnEnable=NULL, TaskOnDisable aOnDisable=NULL);
#endif // _TASK_OO_CALLBACKS
    __TASK_INLINE void setInterval(unsigned long aInterval);
    __TASK_INLINE void setIntervalNodelay(unsigned long aInterval, unsigned int aOption = TASK_INTERVAL_KEEP);
    __TASK_INLINE unsigned long getInterval();
    __TASK_INLINE void setIterations(long aIterations);
    __TASK_INLINE long getIterations();
    __TASK_INLINE unsigned long getRunCounter();
    
#ifdef _TASK_SELF_DESTRUCT
    __TASK_INLINE void setSelfDestruct(bool aSelfDestruct=true) { iStatus.selfdestruct = aSelfDestruct; }
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

    __TASK_INLINE bool isFirstIteration() ;
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

class Scheduler {
  friend class Task;
  public:
    __TASK_INLINE Scheduler();
//  ~Scheduler();
    __TASK_INLINE void init();
    __TASK_INLINE void addTask(Task& aTask);
    __TASK_INLINE void deleteTask(Task& aTask);
    __TASK_INLINE void pause() { iPaused = true; };
    __TASK_INLINE void resume() { iPaused = false; };
    __TASK_INLINE void enable() { iEnabled = true; };
    __TASK_INLINE void disable() { iEnabled = false; };
#ifdef _TASK_PRIORITY
    __TASK_INLINE void disableAll(bool aRecursive = true);
    __TASK_INLINE void enableAll(bool aRecursive = true);
    __TASK_INLINE void startNow(bool aRecursive = true);       // reset ALL active tasks to immediate execution NOW.
#else
    __TASK_INLINE void disableAll();
    __TASK_INLINE void enableAll();
    __TASK_INLINE void startNow();                             // reset ALL active tasks to immediate execution NOW.
#endif

    __TASK_INLINE bool execute();                              // Returns true if none of the tasks' callback methods was invoked (true = idle run)

    __TASK_INLINE Task& currentTask() ;                        // DEPRICATED
    __TASK_INLINE Task* getCurrentTask() ;                     // Returns pointer to the currently active task
    __TASK_INLINE long timeUntilNextIteration(Task& aTask);    // return number of ms until next iteration of a given Task

    __TASK_INLINE unsigned long getActiveTasks() { return iActiveTasks; }
    __TASK_INLINE unsigned long getTotalTasks() { return iTotalTasks; }
    __TASK_INLINE unsigned long getInvokedTasks() { return iInvokedTasks; }
#ifdef _TASK_TICKLESS
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
