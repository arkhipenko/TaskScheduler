// Cooperative multitasking library for Arduino
// Copyright (c) 2015-2023 Anatoli Arkhipenko

#include <stddef.h>
#include <stdint.h>
#include <Arduino.h>

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
// #define _TASK_DEFINE_MILLIS      // Force forward declaration of millis() and micros() "C" style
// #define _TASK_EXTERNAL_TIME      // Custom millis() and micros() methods
// #define _TASK_THREAD_SAFE        // Enable additional checking for thread safety
// #define _TASK_SELF_DESTRUCT      // Enable tasks to "self-destruct" after disable
// #define _TASK_TICKLESS           // Enable support for tickless sleep on FreeRTOS
// #define _TASK_DO_NOT_YIELD       // Disable yield() method in execute()
// #define _TASK_ISR_SUPPORT        // for esp chips - place control methods in IRAM

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
#define __INLINE  inline
#else
#define __INLINE
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


#ifdef _TASK_EXTERNAL_TIME
uint32_t external_millis();
#define _task_millis()  external_millis()
#ifdef _TASK_MICRO_RES
uint32_t external_micros();
#define _task_micros()  external_micros()
#endif  //  _TASK_MICRO_RES
#endif  //  _TASK_EXTERNAL_TIME

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
    __INLINE StatusRequest();
    __INLINE void setWaiting(unsigned int aCount = 1);
    __INLINE bool __TASK_IRAM signal(int aStatus = 0);
    __INLINE void __TASK_IRAM signalComplete(int aStatus = 0);
    __INLINE bool pending();
    __INLINE bool completed();
    __INLINE int  getStatus();
    __INLINE int  getCount();
    
#ifdef _TASK_TIMEOUT
    __INLINE void setTimeout(unsigned long aTimeout) { iTimeout = aTimeout; };
    __INLINE unsigned long getTimeout() { return iTimeout; };
    __INLINE void resetTimeout();
    __INLINE long untilTimeout();
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
} __task_status;


class Task {
  friend class Scheduler;
  public:

#ifdef _TASK_OO_CALLBACKS
    __INLINE Task(unsigned long aInterval=0, long aIterations=0, Scheduler* aScheduler=NULL, bool aEnable=false
#ifdef _TASK_SELF_DESTRUCT
    , bool aSelfDestruct=false);
#else
    );
#endif  // #ifdef _TASK_SELF_DESTRUCT
#else
    __INLINE Task(unsigned long aInterval=0, long aIterations=0, TaskCallback aCallback=NULL, Scheduler* aScheduler=NULL, bool aEnable=false, TaskOnEnable aOnEnable=NULL, TaskOnDisable aOnDisable=NULL
#ifdef _TASK_SELF_DESTRUCT
  , bool aSelfDestruct=false);
#else
  );
#endif  // #ifdef _TASK_SELF_DESTRUCT
#endif // _TASK_OO_CALLBACKS


#ifdef _TASK_STATUS_REQUEST
#ifdef _TASK_OO_CALLBACKS
//    __INLINE Task(Scheduler* aScheduler=NULL);
    __INLINE Task(Scheduler* aScheduler);
#else
//    __INLINE Task(TaskCallback aCallback=NULL, Scheduler* aScheduler=NULL, TaskOnEnable aOnEnable=NULL, TaskOnDisable aOnDisable=NULL);
    __INLINE Task(TaskCallback aCallback, Scheduler* aScheduler, TaskOnEnable aOnEnable=NULL, TaskOnDisable aOnDisable=NULL);
#endif // _TASK_OO_CALLBACKS
#endif  // _TASK_STATUS_REQUEST

    virtual __INLINE ~Task();

#ifdef _TASK_TIMEOUT
    __INLINE void setTimeout(unsigned long aTimeout, bool aReset=false);
    __INLINE void resetTimeout();
    __INLINE unsigned long getTimeout();
    __INLINE long untilTimeout();
    __INLINE bool timedOut();
#endif

    __INLINE bool __TASK_IRAM enable();
    __INLINE bool __TASK_IRAM enableIfNot();
    __INLINE bool __TASK_IRAM enableDelayed(unsigned long aDelay=0);
    __INLINE bool __TASK_IRAM restart();
    __INLINE bool __TASK_IRAM restartDelayed(unsigned long aDelay=0);

    __INLINE void __TASK_IRAM delay(unsigned long aDelay=0);
    __INLINE void adjust(long aInterval);
    __INLINE void __TASK_IRAM forceNextIteration();
    __INLINE bool disable();
    __INLINE void abort();
    __INLINE void cancel();
    __INLINE bool isEnabled();
    __INLINE bool canceled();

#ifdef _TASK_SCHEDULING_OPTIONS
    __INLINE unsigned int getSchedulingOption() { return iOption; }
    __INLINE void setSchedulingOption(unsigned int aOption) {  iOption = aOption; }
#endif  //_TASK_SCHEDULING_OPTIONS

#ifdef _TASK_OO_CALLBACKS
    __INLINE void set(unsigned long aInterval, long aIterations);
#else
    __INLINE void set(unsigned long aInterval, long aIterations, TaskCallback aCallback,TaskOnEnable aOnEnable=NULL, TaskOnDisable aOnDisable=NULL);
#endif // _TASK_OO_CALLBACKS
    __INLINE void setInterval(unsigned long aInterval);
    __INLINE void setIntervalNodelay(unsigned long aInterval, unsigned int aOption = TASK_INTERVAL_KEEP);
    __INLINE unsigned long getInterval();
    __INLINE void setIterations(long aIterations);
    __INLINE long getIterations();
    __INLINE unsigned long getRunCounter();
    
#ifdef _TASK_SELF_DESTRUCT
    __INLINE void setSelfDestruct(bool aSelfDestruct=true) { iStatus.selfdestruct = aSelfDestruct; }
    __INLINE bool getSelfDestruct() { return iStatus.selfdestruct; }
#endif  //  #ifdef _TASK_SELF_DESTRUCT

#ifdef _TASK_OO_CALLBACKS
    virtual __INLINE bool Callback() =0;  // return true if run was "productive - this will disable sleep on the idle run for next pass
    virtual __INLINE bool OnEnable();  // return true if task should be enabled, false if it should remain disabled
    virtual __INLINE void OnDisable();
#else
    __INLINE void setCallback(TaskCallback aCallback) ;
    __INLINE void setOnEnable(TaskOnEnable aCallback) ;
    __INLINE void setOnDisable(TaskOnDisable aCallback) ;
    __INLINE void yield(TaskCallback aCallback);
    __INLINE void yieldOnce(TaskCallback aCallback);
#endif // _TASK_OO_CALLBACKS

    __INLINE bool isFirstIteration() ;
    __INLINE bool isLastIteration() ;

#ifdef _TASK_TIMECRITICAL
    __INLINE long getOverrun() ;
    __INLINE long getStartDelay() ;
#endif  // _TASK_TIMECRITICAL

#ifdef _TASK_STATUS_REQUEST
    __INLINE bool waitFor(StatusRequest* aStatusRequest, unsigned long aInterval = 0, long aIterations = 1);
    __INLINE bool waitForDelayed(StatusRequest* aStatusRequest, unsigned long aInterval = 0, long aIterations = 1);
    __INLINE StatusRequest* getStatusRequest() ;
    __INLINE StatusRequest* getInternalStatusRequest() ;
#endif  // _TASK_STATUS_REQUEST

#ifdef _TASK_WDT_IDS
    __INLINE void setId(unsigned int aID) ;
    __INLINE unsigned int getId() ;
    __INLINE void setControlPoint(unsigned int aPoint) ;
    __INLINE unsigned int getControlPoint() ;
#endif  // _TASK_WDT_IDS

#ifdef _TASK_LTS_POINTER
    __INLINE void  setLtsPointer(void *aPtr) ;
    __INLINE void* getLtsPointer() ;
#endif  // _TASK_LTS_POINTER

#ifdef _TASK_EXPOSE_CHAIN
    __INLINE Task*  getPreviousTask() { return iPrev; };  // pointer to the previous task in the chain, NULL if first or not set
    __INLINE Task*  getNextTask()     { return iNext; };  // pointer to the next task in the chain, NULL if last or not set
#endif // _TASK_EXPOSE_CHAIN


  _TASK_SCOPE:
    __INLINE void reset();

    volatile __task_status    iStatus;
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


#ifdef _TASK_THREAD_SAFE
    volatile uint8_t          iMutex;                // a mutex to pause scheduling during chages to the task
#endif
};

class Scheduler {
  friend class Task;
  public:
    __INLINE Scheduler();
//  ~Scheduler();
    __INLINE void init();
    __INLINE void addTask(Task& aTask);
    __INLINE void deleteTask(Task& aTask);
    __INLINE void pause() { iPaused = true; };
    __INLINE void resume() { iPaused = false; };
    __INLINE void enable() { iEnabled = true; };
    __INLINE void disable() { iEnabled = false; };
#ifdef _TASK_PRIORITY
    __INLINE void disableAll(bool aRecursive = true);
    __INLINE void enableAll(bool aRecursive = true);
    __INLINE void startNow(bool aRecursive = true);       // reset ALL active tasks to immediate execution NOW.
#else
    __INLINE void disableAll();
    __INLINE void enableAll();
    __INLINE void startNow();                             // reset ALL active tasks to immediate execution NOW.
#endif

    __INLINE bool execute();                              // Returns true if none of the tasks' callback methods was invoked (true = idle run)

    __INLINE Task& currentTask() ;                        // DEPRICATED
    __INLINE Task* getCurrentTask() ;                     // Returns pointer to the currently active task
    __INLINE long timeUntilNextIteration(Task& aTask);    // return number of ms until next iteration of a given Task

    __INLINE unsigned long getActiveTasks() { return iActiveTasks; }
    __INLINE unsigned long getTotalTasks() { return iTotalTasks; }
    __INLINE unsigned long getInvokedTasks() { return iInvokedTasks; }
#ifdef _TASK_TICKLESS
    __INLINE unsigned long getNextRun() { return iNextRun; }
#endif

#ifdef _TASK_SLEEP_ON_IDLE_RUN
    __INLINE void allowSleep(bool aState = true);
    __INLINE void setSleepMethod( SleepCallback aCallback );
#endif  // _TASK_SLEEP_ON_IDLE_RUN

#ifdef _TASK_LTS_POINTER
    __INLINE void* currentLts();
#endif  // _TASK_LTS_POINTER

#ifdef _TASK_TIMECRITICAL
    __INLINE bool isOverrun();
    __INLINE void cpuLoadReset();
    __INLINE unsigned long getCpuLoadCycle(){ return iCPUCycle; };
    __INLINE unsigned long getCpuLoadIdle() { return iCPUIdle; };
    __INLINE unsigned long getCpuLoadTotal();
#endif  // _TASK_TIMECRITICAL

#ifdef _TASK_PRIORITY
    __INLINE void setHighPriorityScheduler(Scheduler* aScheduler);
    __INLINE static Scheduler& currentScheduler() { return *(iCurrentScheduler); };
#endif  // _TASK_PRIORITY

#ifdef _TASK_EXPOSE_CHAIN
    __INLINE Task*  getFirstTask() { return iFirst; };       // pointer to the previous task in the chain, NULL if first or not set
    __INLINE Task*  getLastTask()  { return iLast;  };       // pointer to the next task in the chain, NULL if last or not set
#endif // _TASK_EXPOSE_CHAIN

  _TASK_SCOPE:
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
