#include "TaskScheduler.h"

// ------------------ TaskScheduler implementation --------------------

#ifndef _TASK_EXTERNAL_TIME

#ifdef _TASK_MICRO_RES
static uint32_t _task_micros() {return micros();}
#else
static uint32_t _task_millis() {return millis();}
#endif  // _TASK_MICRO_RES

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
 *  Negative status will complete Status Request fully (since an error occurred).
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
    iStatusRequest = aStatusRequest;
    if ( iStatusRequest != NULL ) { // assign internal StatusRequest var and check if it is not NULL
        setIterations(aIterations);
        setInterval(aInterval);
        iStatus.waiting = _TASK_SR_NODELAY;  // no delay
        return enable();
    }
    return false;
}

bool Task::waitForDelayed(StatusRequest* aStatusRequest, unsigned long aInterval, long aIterations) {
    iStatusRequest = aStatusRequest;
    if ( iStatusRequest != NULL ) { // assign internal StatusRequest var and check if it is not NULL
        setIterations(aIterations);
        if ( aInterval ) setInterval(aInterval);  // For the delayed version only set the interval if it was not a zero
        iStatus.waiting = _TASK_SR_DELAY;  // with delay equal to the current interval
        return enable();
    }
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
    iCallback = aCallback;
    iOnEnable = aOnEnable;
    iOnDisable = aOnDisable;
#endif // _TASK_OO_CALLBACKS

    setInterval(aInterval);
    iSetIterations = iIterations = aIterations;
}

/** Sets number of iterations for the task
 * if task is enabled, schedule for immediate execution
 * @param aIterations - number of iterations, use -1 for no limit
 */
void Task::setIterations(long aIterations) {
    iSetIterations = iIterations = aIterations;
}

#ifndef _TASK_OO_CALLBACKS

/** Prepare task for next step iteration following yielding of control to the scheduler
 * @param aCallback - pointer to the callback method for the next step
 */
void Task::yield (TaskCallback aCallback) {
    iCallback = aCallback;
    forceNextIteration();

    // The next 2 lines adjust runcounter and number of iterations
    // as if it is the same run of the callback, just split between
    // a series of callback methods
    iRunCounter--;
    if ( iIterations >= 0 ) iIterations++;
}

/** Prepare task for next step iteration following yielding of control to the scheduler
 * @param aCallback - pointer to the callback method for the next step
 */
void Task::yieldOnce (TaskCallback aCallback) {
    yield(aCallback);
    iIterations = 1;
}
#endif // _TASK_OO_CALLBACKS


/** Enables the task
 *  schedules it for execution as soon as possible,
 *  and resets the RunCounter back to zero
 */
bool Task::enable() {
    if (iScheduler) { // activation without active scheduler does not make sense
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
        return iStatus.enabled;
    }
    return false;
}

/** Enables the task only if it was not enabled already
 * Returns previous state (true if was already enabled, false if was not)
 */
bool Task::enableIfNot() {
    bool previousEnabled = iStatus.enabled;
    if ( !previousEnabled ) enable();
    return (previousEnabled);
}

/** Enables the task
 * and schedules it for execution after a delay = aInterval
 */
bool Task::enableDelayed(unsigned long aDelay) {
    enable();
    delay(aDelay);
    return iStatus.enabled;
}

#ifdef _TASK_TIMEOUT
void Task::setTimeout(unsigned long aTimeout, bool aReset) {
    iTimeout = aTimeout;
    if (aReset) resetTimeout();
}

void Task::resetTimeout() {
    iStarttime = _TASK_TIME_FUNCTION();
    iStatus.timeout = false;
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
//  if (!aDelay) aDelay = iInterval;
    iDelay = aDelay ? aDelay : iInterval;
    iPreviousMillis = _TASK_TIME_FUNCTION(); // - iInterval + aDelay;
}

/** Schedules next iteration of Task for execution immediately (if enabled)
 * leaves task enabled or disabled
 * Task's original schedule is shifted, and all subsequent iterations will continue from this point in time
 */
void Task::forceNextIteration() {
    iPreviousMillis = _TASK_TIME_FUNCTION() - (iDelay = iInterval);
}

/** Sets the execution interval.
 * Task execution is delayed for aInterval
 * Use  enable() to schedule execution ASAP
 * @param aInterval - new execution interval
 */
void Task::setInterval (unsigned long aInterval) {
    iInterval = aInterval;
    delay(); // iDelay will be updated by the delay() function
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
 * Task will no longer be executed by the scheduler. The ondisable method will be called after 'canceled' flag is set
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

/** Initializes all internal variables
 */
void Scheduler::init() {
    iFirst = NULL;
    iLast = NULL;
    iCurrent = NULL;

    iPaused = false;
    iEnabled = true;  

#ifdef _TASK_PRIORITY
    iHighPriority = NULL;
#endif  // _TASK_PRIORITY

#ifdef _TASK_SLEEP_ON_IDLE_RUN
    allowSleep(true);
#endif  // _TASK_SLEEP_ON_IDLE_RUN

#ifdef _TASK_TIMECRITICAL
    cpuLoadReset();
#endif  // _TASK_TIMECRITICAL
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
}

/** Deletes specific Task from the execution chain
 * @param &aTask - reference to the task to be deleted from the chain
 */
void Scheduler::deleteTask(Task& aTask) {
// Can only delete own tasks
    if (aTask.iScheduler != this) 
        return;
    
    aTask.iScheduler = NULL;
    if (aTask.iPrev == NULL) {
        if (aTask.iNext == NULL) {
            iFirst = NULL;
            iLast = NULL;
            return;
        }
        else {
            aTask.iNext->iPrev = NULL;
            iFirst = aTask.iNext;
            aTask.iNext = NULL;
            return;
        }
    }

    if (aTask.iNext == NULL) {
        aTask.iPrev->iNext = NULL;
        iLast = aTask.iPrev;
        aTask.iPrev = NULL;
        return;
    }

    aTask.iPrev->iNext = aTask.iNext;
    aTask.iNext->iPrev = aTask.iPrev;
    aTask.iPrev = NULL;
    aTask.iNext = NULL;
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
    Task    *current = iFirst;
    while (current) {
        current->disable();
        current = current->iNext;
    }

#ifdef _TASK_PRIORITY
    if (aRecursive && iHighPriority) iHighPriority->disableAll(true);
#endif  // _TASK_PRIORITY
}


/** Enables all the tasks in the execution chain
 * @param aRecursive - if true, tasks of the higher priority chains are enabled as well recursively
 */
#ifdef _TASK_PRIORITY
void Scheduler::enableAll(bool aRecursive) {
#else
void Scheduler::enableAll() {
#endif    
    Task    *current = iFirst;
    while (current) {
        current->enable();
        current = current->iNext;
    }

#ifdef _TASK_PRIORITY
    if (aRecursive && iHighPriority) iHighPriority->enableAll(true);
#endif  // _TASK_PRIORITY

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

    iCurrent = iFirst;
    while (iCurrent) {
        if ( iCurrent->iStatus.enabled ) iCurrent->iPreviousMillis = t - iCurrent->iDelay;
        iCurrent = iCurrent->iNext;
    }

#ifdef _TASK_PRIORITY
    if (aRecursive && iHighPriority) iHighPriority->startNow( true );
#endif  // _TASK_PRIORITY
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

    long d = (long) aTask.iDelay - ( (long) ((_TASK_TIME_FUNCTION() - aTask.iPreviousMillis)) );

    if ( d < 0 )
        return (0); // Task will run as soon as possible
    return ( d );
}


Task& Scheduler::currentTask() { return *iCurrent; }      // DEPRECATED. Use the next one instead
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

    //  each scheduled is enabled/disabled individually, so check iEnabed only
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
