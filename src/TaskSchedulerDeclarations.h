// Cooperative multitasking library for Arduino
// Copyright (c) 2015-2019 Anatoli Arkhipenko

#include <stddef.h>
#include <stdint.h>

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

class Scheduler;

#define TASK_SCHEDULE       0   // default
#define TASK_SCHEDULE_NC    1   // schedule + no catch-ups (always in the future)
#define TASK_INTERVAL       2   // interval (always in the future)

#ifdef _TASK_DEBUG
    #define _TASK_SCOPE  public
#else
    #define _TASK_SCOPE  private
#endif

#define TASK_IMMEDIATE          0
#define TASK_FOREVER         (-1)
#define TASK_ONCE               1

#ifdef _TASK_TIMEOUT
#define TASK_NOTIMEOUT          0
#endif

#ifdef _TASK_PRIORITY
    extern Scheduler* iCurrentScheduler;
#endif // _TASK_PRIORITY

#ifdef _TASK_INLINE
#define INLINE  inline
#else
#define INLINE
#endif

#ifdef _TASK_EXTERNAL_TIME
#define _task_millis()  external_millis()
#define _task_micros()  external_micros()
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


#ifdef _TASK_STATUS_REQUEST

#define TASK_SR_OK          0
#define TASK_SR_ERROR       (-1)
#define TASK_SR_TIMEOUT     (-99)
 
#define _TASK_SR_NODELAY    1
#define _TASK_SR_DELAY      2

class Scheduler;

class StatusRequest {
  friend class Scheduler;
  public:
    INLINE StatusRequest();
    INLINE void setWaiting(unsigned int aCount = 1);
    INLINE bool signal(int aStatus = 0);
    INLINE void signalComplete(int aStatus = 0);
    INLINE bool pending();
    INLINE bool completed();
    INLINE int getStatus();
    INLINE int getCount();
    
#ifdef _TASK_TIMEOUT
    INLINE void setTimeout(unsigned long aTimeout) { iTimeout = aTimeout; };
    INLINE unsigned long getTimeout() { return iTimeout; };
    INLINE void resetTimeout();
    INLINE long untilTimeout();
#endif

  _TASK_SCOPE:
    unsigned int  iCount;          // number of statuses to wait for. waiting for more that 65000 events seems unreasonable: unsigned int should be sufficient
    int           iStatus;         // status of the last completed request. negative = error;  zero = OK; positive = OK with a specific status

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
    bool  enabled    : 1;           // indicates that task is enabled or not.
    bool  inonenable : 1;           // indicates that task execution is inside OnEnable method (preventing infinite loops)
    bool  canceled   : 1;           // indication that tast has been canceled prior to normal end of all iterations or regular call to disable()
#ifdef _TASK_STATUS_REQUEST
    uint8_t  waiting : 2;           // indication if task is waiting on the status request
#endif

#ifdef _TASK_TIMEOUT
    bool  timeout    : 1;           // indication if task timed out
#endif

} __task_status;


class Task {
  friend class Scheduler;
  public:

#ifdef _TASK_OO_CALLBACKS
    INLINE Task(unsigned long aInterval=0, long aIterations=0, Scheduler* aScheduler=NULL, bool aEnable=false);
#else
    INLINE Task(unsigned long aInterval=0, long aIterations=0, TaskCallback aCallback=NULL, Scheduler* aScheduler=NULL, bool aEnable=false, TaskOnEnable aOnEnable=NULL, TaskOnDisable aOnDisable=NULL);
#endif // _TASK_OO_CALLBACKS


#ifdef _TASK_STATUS_REQUEST
#ifdef _TASK_OO_CALLBACKS
//    INLINE Task(Scheduler* aScheduler=NULL);
    INLINE Task(Scheduler* aScheduler);
#else
//    INLINE Task(TaskCallback aCallback=NULL, Scheduler* aScheduler=NULL, TaskOnEnable aOnEnable=NULL, TaskOnDisable aOnDisable=NULL);
    INLINE Task(TaskCallback aCallback, Scheduler* aScheduler, TaskOnEnable aOnEnable=NULL, TaskOnDisable aOnDisable=NULL);
#endif // _TASK_OO_CALLBACKS
#endif  // _TASK_STATUS_REQUEST

    virtual INLINE ~Task();

#ifdef _TASK_TIMEOUT
    INLINE void setTimeout(unsigned long aTimeout, bool aReset=false);
    INLINE void resetTimeout();
    INLINE unsigned long getTimeout();
    INLINE long untilTimeout();
    INLINE bool timedOut();
#endif

    INLINE bool enable();
    INLINE bool enableIfNot();
    INLINE bool enableDelayed(unsigned long aDelay=0);
    INLINE bool restart();
    INLINE bool restartDelayed(unsigned long aDelay=0);

    INLINE void delay(unsigned long aDelay=0);
    INLINE void forceNextIteration();
    INLINE bool disable();
    INLINE void abort();
    INLINE void cancel();
    INLINE bool isEnabled();
    INLINE bool canceled();

#ifdef _TASK_SCHEDULING_OPTIONS
    INLINE unsigned int getSchedulingOption() { return iOption; }
    INLINE void setSchedulingOption(unsigned int aOption) {  iOption = aOption; }
#endif  //_TASK_SCHEDULING_OPTIONS

#ifdef _TASK_OO_CALLBACKS
    INLINE void set(unsigned long aInterval, long aIterations);
#else
    INLINE void set(unsigned long aInterval, long aIterations, TaskCallback aCallback,TaskOnEnable aOnEnable=NULL, TaskOnDisable aOnDisable=NULL);
#endif // _TASK_OO_CALLBACKS
    INLINE void setInterval(unsigned long aInterval);
    INLINE unsigned long getInterval();
    INLINE void setIterations(long aIterations);
    INLINE long getIterations();
    INLINE unsigned long getRunCounter() ;

#ifdef _TASK_OO_CALLBACKS
    virtual INLINE bool Callback() =0;  // return true if run was "productive - this will disable sleep on the idle run for next pass
    virtual INLINE bool OnEnable();  // return true if task should be enabled, false if it should remain disabled
    virtual INLINE void OnDisable();
#else
    INLINE void setCallback(TaskCallback aCallback) ;
    INLINE void setOnEnable(TaskOnEnable aCallback) ;
    INLINE void setOnDisable(TaskOnDisable aCallback) ;
    INLINE void yield(TaskCallback aCallback);
    INLINE void yieldOnce(TaskCallback aCallback);
#endif // _TASK_OO_CALLBACKS

    INLINE bool isFirstIteration() ;
    INLINE bool isLastIteration() ;

#ifdef _TASK_TIMECRITICAL
    INLINE long getOverrun() ;
    INLINE long getStartDelay() ;
#endif  // _TASK_TIMECRITICAL

#ifdef _TASK_STATUS_REQUEST
    INLINE bool waitFor(StatusRequest* aStatusRequest, unsigned long aInterval = 0, long aIterations = 1);
    INLINE bool waitForDelayed(StatusRequest* aStatusRequest, unsigned long aInterval = 0, long aIterations = 1);
    INLINE StatusRequest* getStatusRequest() ;
    INLINE StatusRequest* getInternalStatusRequest() ;
#endif  // _TASK_STATUS_REQUEST

#ifdef _TASK_WDT_IDS
    INLINE void setId(unsigned int aID) ;
    INLINE unsigned int getId() ;
    INLINE void setControlPoint(unsigned int aPoint) ;
    INLINE unsigned int getControlPoint() ;
#endif  // _TASK_WDT_IDS

#ifdef _TASK_LTS_POINTER
    INLINE void  setLtsPointer(void *aPtr) ;
    INLINE void* getLtsPointer() ;
#endif  // _TASK_LTS_POINTER

#ifdef _TASK_EXPOSE_CHAIN
    INLINE Task*  getPreviousTask() { return iPrev; };  // pointer to the previous task in the chain, NULL if first or not set
    INLINE Task*  getNextTask()     { return iNext; };  // pointer to the next task in the chain, NULL if last or not set
#endif // _TASK_EXPOSE_CHAIN


  _TASK_SCOPE:
    INLINE void reset();

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
};

class Scheduler {
  friend class Task;
  public:
    INLINE Scheduler();
//  ~Scheduler();
    INLINE void init();
    INLINE void addTask(Task& aTask);
    INLINE void deleteTask(Task& aTask);
    INLINE void pause() { iPaused = true; };
    INLINE void resume() { iPaused = false; };
    INLINE void enable() { iEnabled = true; };
    INLINE void disable() { iEnabled = false; };
#ifdef _TASK_PRIORITY
    INLINE void disableAll(bool aRecursive = true);
    INLINE void enableAll(bool aRecursive = true);
    INLINE void startNow(bool aRecursive = true);       // reset ALL active tasks to immediate execution NOW.
#else
    INLINE void disableAll();
    INLINE void enableAll();
    INLINE void startNow();                             // reset ALL active tasks to immediate execution NOW.
#endif
    INLINE bool execute();                              // Returns true if none of the tasks' callback methods was invoked (true = idle run)
    INLINE Task& currentTask() ;                        // DEPRICATED
    INLINE Task* getCurrentTask() ;                     // Returns pointer to the currently active task
    INLINE long timeUntilNextIteration(Task& aTask);    // return number of ms until next iteration of a given Task

#ifdef _TASK_SLEEP_ON_IDLE_RUN
    INLINE void allowSleep(bool aState = true);
    INLINE void setSleepMethod( SleepCallback aCallback );
#endif  // _TASK_SLEEP_ON_IDLE_RUN

#ifdef _TASK_LTS_POINTER
    INLINE void* currentLts();
#endif  // _TASK_LTS_POINTER

#ifdef _TASK_TIMECRITICAL
    INLINE bool isOverrun();
    INLINE void cpuLoadReset();
    INLINE unsigned long getCpuLoadCycle(){ return iCPUCycle; };
    INLINE unsigned long getCpuLoadIdle() { return iCPUIdle; };
    INLINE unsigned long getCpuLoadTotal();
#endif  // _TASK_TIMECRITICAL

#ifdef _TASK_PRIORITY
    INLINE void setHighPriorityScheduler(Scheduler* aScheduler);
    INLINE static Scheduler& currentScheduler() { return *(iCurrentScheduler); };
#endif  // _TASK_PRIORITY

#ifdef _TASK_EXPOSE_CHAIN
    INLINE Task*  getFirstTask() { return iFirst; };       // pointer to the previous task in the chain, NULL if first or not set
    INLINE Task*  getLastTask()  { return iLast;  };       // pointer to the next task in the chain, NULL if last or not set
#endif // _TASK_EXPOSE_CHAIN

  _TASK_SCOPE:
    Task       *iFirst, *iLast, *iCurrent;        // pointers to first, last and current tasks in the chain

    bool       iPaused, iEnabled;
#ifdef _TASK_SLEEP_ON_IDLE_RUN
    bool        iAllowSleep;                      // indication if putting MC to IDLE_SLEEP mode is allowed by the program at this time.
#endif  // _TASK_SLEEP_ON_IDLE_RUN

#ifdef _TASK_PRIORITY
    Scheduler  *iHighPriority;                    // Pointer to a higher priority scheduler
#endif  // _TASK_PRIORITY

#ifdef _TASK_TIMECRITICAL
    unsigned long iCPUStart;
    unsigned long iCPUCycle;
    unsigned long iCPUIdle;
#endif  // _TASK_TIMECRITICAL
};


#endif /* _TASKSCHEDULERDECLARATIONS_H_ */
