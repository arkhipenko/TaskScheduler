// Cooperative multitasking library for Arduino
// Copyright (c) 2015-2017 Anatoli Arkhipenko

#include <stddef.h> 

#ifndef _TASKSCHEDULERDECLARATIONS_H_
#define _TASKSCHEDULERDECLARATIONS_H_

// ----------------------------------------
// The following "defines" control library functionality at compile time,
// and should be used in the main sketch depending on the functionality required
// 
// #define _TASK_TIMECRITICAL      // Enable monitoring scheduling overruns
// #define _TASK_SLEEP_ON_IDLE_RUN // Enable 1 ms SLEEP_IDLE powerdowns between tasks if no callback methods were invoked during the pass 
// #define _TASK_STATUS_REQUEST    // Compile with support for StatusRequest functionality - triggering tasks on status change events in addition to time only
// #define _TASK_WDT_IDS           // Compile with support for wdt control points and task ids
// #define _TASK_LTS_POINTER       // Compile with support for local task storage pointer
// #define _TASK_PRIORITY          // Support for layered scheduling priority
// #define _TASK_MICRO_RES         // Support for microsecond resolution
// #define _TASK_STD_FUNCTION      // Support for std::function (ESP8266 ONLY)
// #define _TASK_DEBUG             // Make all methods and variables public for debug purposes

#ifdef _TASK_DEBUG
    #define _TASK_SCOPE  public
#else
    #define _TASK_SCOPE  private
#endif

#define TASK_IMMEDIATE          0
#define TASK_FOREVER         (-1)
#define TASK_ONCE               1

#ifdef _TASK_PRIORITY
    class Scheduler;
    extern Scheduler* iCurrentScheduler;
#endif // _TASK_PRIORITY

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

#define _TASK_SR_NODELAY    1
#define _TASK_SR_DELAY      2

class StatusRequest {
  public:
    StatusRequest();
    void setWaiting(unsigned int aCount = 1);
    bool signal(int aStatus = 0);
    void signalComplete(int aStatus = 0);
    bool pending();
    bool completed();
    int getStatus();
    int getCount();

  _TASK_SCOPE:
    unsigned int  iCount;          // number of statuses to wait for. waiting for more that 65000 events seems unreasonable: unsigned int should be sufficient
    int           iStatus;         // status of the last completed request. negative = error;  zero = OK; positive = OK with a specific status
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
#endif

typedef struct  {
    bool  enabled    : 1;           // indicates that task is enabled or not.
    bool  inonenable : 1;           // indicates that task execution is inside OnEnable method (preventing infinite loops)
#ifdef _TASK_STATUS_REQUEST
    byte  waiting    : 2;           // indication if task is waiting on the status request
#endif
} __task_status;

class Scheduler; 


class Task {
  friend class Scheduler;
  public:
    Task(unsigned long aInterval=0, long aIterations=0, TaskCallback aCallback=NULL, Scheduler* aScheduler=NULL, bool aEnable=false, TaskOnEnable aOnEnable=NULL, TaskOnDisable aOnDisable=NULL);
#ifdef _TASK_STATUS_REQUEST
    Task(TaskCallback aCallback=NULL, Scheduler* aScheduler=NULL, TaskOnEnable aOnEnable=NULL, TaskOnDisable aOnDisable=NULL);
#endif  // _TASK_STATUS_REQUEST
    ~Task();

    void enable();
    bool enableIfNot();
    void enableDelayed(unsigned long aDelay=0);
    void delay(unsigned long aDelay=0);
    void forceNextIteration(); 
    void restart();
    void restartDelayed(unsigned long aDelay=0);
    bool disable();
    bool isEnabled();
    void set(unsigned long aInterval, long aIterations, TaskCallback aCallback,TaskOnEnable aOnEnable=NULL, TaskOnDisable aOnDisable=NULL);
    void setInterval(unsigned long aInterval);
    unsigned long getInterval();
    void setIterations(long aIterations);
    long getIterations();
    unsigned long getRunCounter() ;
    void setCallback(TaskCallback aCallback) ;
    void setOnEnable(TaskOnEnable aCallback) ;
    void setOnDisable(TaskOnDisable aCallback) ;
    void yield(TaskCallback aCallback);
    void yieldOnce(TaskCallback aCallback);
    bool isFirstIteration() ;
    bool isLastIteration() ;
    
#ifdef _TASK_TIMECRITICAL
    long getOverrun() ;
    long getStartDelay() ;
#endif  // _TASK_TIMECRITICAL

#ifdef _TASK_STATUS_REQUEST
    void waitFor(StatusRequest* aStatusRequest, unsigned long aInterval = 0, long aIterations = 1);
    void waitForDelayed(StatusRequest* aStatusRequest, unsigned long aInterval = 0, long aIterations = 1);
    StatusRequest* getStatusRequest() ;
    StatusRequest* getInternalStatusRequest() ;
#endif  // _TASK_STATUS_REQUEST

#ifdef _TASK_WDT_IDS
    void setId(unsigned int aID) ;
    unsigned int getId() ;
    void setControlPoint(unsigned int aPoint) ;
    unsigned int getControlPoint() ;
#endif  // _TASK_WDT_IDS

#ifdef _TASK_LTS_POINTER
    void  setLtsPointer(void *aPtr) ;
    void* getLtsPointer() ;
#endif  // _TASK_LTS_POINTER

  _TASK_SCOPE:
    void reset();

    volatile __task_status    iStatus;
    volatile unsigned long    iInterval;             // execution interval in milliseconds (or microseconds). 0 - immediate
    volatile unsigned long    iDelay;                // actual delay until next execution (usually equal iInterval)
    volatile unsigned long    iPreviousMillis;       // previous invocation time (millis).  Next invocation = iPreviousMillis + iInterval.  Delayed tasks will "catch up" 
#ifdef _TASK_TIMECRITICAL
    volatile long             iOverrun;              // negative if task is "catching up" to it's schedule (next invocation time is already in the past)
    volatile long             iStartDelay;           // actual execution of the task's callback method was delayed by this number of millis
#endif  // _TASK_TIMECRITICAL
    volatile long             iIterations;           // number of iterations left. 0 - last iteration. -1 - infinite iterations
    long                      iSetIterations;        // number of iterations originally requested (for restarts)
    unsigned long             iRunCounter;           // current number of iteration (starting with 1). Resets on enable. 
    TaskCallback              iCallback;             // pointer to the void callback method
    TaskOnEnable              iOnEnable;             // pointer to the bolol OnEnable callback method
    TaskOnDisable             iOnDisable;            // pointer to the void OnDisable method
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
};

class Scheduler {
  friend class Task;
  public:
    Scheduler();
//	~Scheduler();
    void init();
    void addTask(Task& aTask);
    void deleteTask(Task& aTask);
    void disableAll(bool aRecursive = true);
    void enableAll(bool aRecursive = true);
    bool execute();                              // Returns true if none of the tasks' callback methods was invoked (true = idle run)
    void startNow(bool aRecursive = true);       // reset ALL active tasks to immediate execution NOW.
    Task& currentTask() ;
    long timeUntilNextIteration(Task& aTask);    // return number of ms until next iteration of a given Task
#ifdef _TASK_SLEEP_ON_IDLE_RUN
    void allowSleep(bool aState = true);
#endif  // _TASK_SLEEP_ON_IDLE_RUN
#ifdef _TASK_LTS_POINTER
    void* currentLts() ;
#endif  // _TASK_LTS_POINTER
#ifdef _TASK_TIMECRITICAL
    bool isOverrun() ;
#endif  // _TASK_TIMECRITICAL
#ifdef _TASK_PRIORITY
    void setHighPriorityScheduler(Scheduler* aScheduler);
    static Scheduler& currentScheduler() { return *(iCurrentScheduler); };
#endif  // _TASK_PRIORITY

  _TASK_SCOPE:
    Task       *iFirst, *iLast, *iCurrent;        // pointers to first, last and current tasks in the chain
#ifdef _TASK_SLEEP_ON_IDLE_RUN
    bool        iAllowSleep;                      // indication if putting avr to IDLE_SLEEP mode is allowed by the program at this time. 

#endif  // _TASK_SLEEP_ON_IDLE_RUN
#ifdef _TASK_PRIORITY
    Scheduler  *iHighPriority;                    // Pointer to a higher priority scheduler
#endif  // _TASK_PRIORITY
};


#endif /* _TASKSCHEDULERDECLARATIONS_H_ */
