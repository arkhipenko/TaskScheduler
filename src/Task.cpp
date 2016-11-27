#include "TaskScheduler.h"

#ifdef _TASK_WDT_IDS
static unsigned int __task_id_counter = 0;		// global task ID counter for assiging task IDs automatically. 
#endif  // _TASK_WDT_IDS


bool Task::isEnabled() { return iStatus.enabled; }

unsigned long Task::getInterval() { return iInterval; }
long Task::getIterations() { return iIterations; }
unsigned long Task::getRunCounter() { return iRunCounter; }
void Task::setCallback(void (*aCallback)()) { iCallback = aCallback; }
void Task::setOnEnable(bool (*aCallback)()) { iOnEnable = aCallback; }
void Task::setOnDisable(void (*aCallback)()) { iOnDisable = aCallback; }
#ifdef _TASK_TIMECRITICAL
long Task::getOverrun() { return iOverrun; }
long Task::getStartDelay() { return iStartDelay; }
#endif  // _TASK_TIMECRITICAL
bool Task::isFirstIteration() { return (iRunCounter <= 1); } 
bool Task::isLastIteration() { return (iIterations == 0); }
#ifdef _TASK_STATUS_REQUEST
Task::StatusRequest* getStatusRequest() {return iStatusRequest; }
#endif  // _TASK_STATUS_REQUEST
#ifdef _TASK_WDT_IDS
void Task::setId(unsigned int aID) { iTaskID = aID; }
unsigned int Task::getId() { return iTaskID; }
void Task::setControlPoint(unsigned int aPoint) { iControlPoint = aPoint; }
unsigned int Task::getControlPoint() { return iControlPoint; }
#endif  // _TASK_WDT_IDS
#ifdef _TASK_LTS_POINTER
void	Task::setLtsPointer(void *aPtr) { iLTS = aPtr; }
void* Task::getLtsPointer() { return iLTS; }
#endif  // _TASK_LTS_POINTER


// ------------------ TaskScheduler implementation --------------------

/** Constructor, uses default values for the parameters
 * so could be called with no parameters.
 */
Task::Task( unsigned long aInterval, long aIterations, void (*aCallback)(), Scheduler* aScheduler, bool aEnable, bool (*aOnEnable)(), void (*aOnDisable)() ) {
	reset();
	set(aInterval, aIterations, aCallback, aOnEnable, aOnDisable);
	if (aScheduler) aScheduler->addTask(*this);
#ifdef _TASK_STATUS_REQUEST
	iStatusRequest = NULL;
#endif  // _TASK_STATUS_REQUEST
#ifdef _TASK_WDT_IDS
	iTaskID = ++__task_id_counter;
#endif  // _TASK_WDT_IDS
	if (aEnable) enable();
}


#ifdef _TASK_STATUS_REQUEST

/** Constructor with reduced parameter list for tasks created for 
 *  StatusRequest only triggering (always immediate and only 1 iteration)
 */
Task::Task( void (*aCallback)(), Scheduler* aScheduler, bool (*aOnEnable)(), void (*aOnDisable)() ) {
	reset();
	set(TASK_IMMEDIATE, TASK_ONCE, aCallback, aOnEnable, aOnDisable);
	if (aScheduler) aScheduler->addTask(*this);
	iStatusRequest = NULL;
#ifdef _TASK_WDT_IDS
	iTaskID = ++__task_id_counter;
#endif  // _TASK_WDT_IDS
}

/** Sets a Task to wait until a particular event completes 
 *  @param: aStatusRequest - a pointer for the StatusRequest to wait for.
 *  If aStatusRequest is NULL, request for waiting is ignored, and the waiting task is not enabled. 
 */
void Task::waitFor(StatusRequest* aStatusRequest, unsigned long aInterval, long aIterations) {
	if ( ( iStatusRequest = aStatusRequest) ) { // assign internal StatusRequest var and check if it is not NULL
		setIterations(aIterations);
		setInterval(aInterval); 
		iStatus.waiting = _TASK_SR_NODELAY;  // no delay
		enable();
	}
}

void Task::waitForDelayed(StatusRequest* aStatusRequest, unsigned long aInterval, long aIterations) {
	if ( ( iStatusRequest = aStatusRequest) ) { // assign internal StatusRequest var and check if it is not NULL
		setIterations(aIterations);
		if ( aInterval ) setInterval(aInterval);  // For the dealyed version only set the interval if it was not a zero
		iStatus.waiting = _TASK_SR_DELAY;  // with delay equal to the current interval
		enable();
	}
}
#endif  // _TASK_STATUS_REQUEST

/** Resets (initializes) the task/
 * Task is not enabled and is taken out 
 * out of the execution chain as a result
 */
void Task::reset() {
	iStatus.enabled = false;
	iStatus.inonenable = false;
	iPreviousMillis = 0;
	iInterval = iDelay = 0;
	iPrev = NULL;
	iNext = NULL;
	iScheduler = NULL;
	iRunCounter = 0;
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
	iStatus.waiting = 0;
#endif  // _TASK_STATUS_REQUEST
}

/** Explicitly set Task execution parameters
 * @param aInterval - execution interval in ms
 * @param aIterations - number of iterations, use -1 for no limit
 * @param aCallback - pointer to the callback method which executes the task actions
 * @param aOnEnable - pointer to the callback method which is called on enable()
 * @param aOnDisable - pointer to the callback method which is called on disable()
 */
void Task::set(unsigned long aInterval, long aIterations, void (*aCallback)(),bool (*aOnEnable)(), void (*aOnDisable)()) {
	setInterval(aInterval); 
	iSetIterations = iIterations = aIterations;
	iCallback = aCallback;
	iOnEnable = aOnEnable;
	iOnDisable = aOnDisable;
}

/** Sets number of iterations for the task
 * if task is enabled, schedule for immediate execution
 * @param aIterations - number of iterations, use -1 for no limit
 */
void Task::setIterations(long aIterations) { 
	iSetIterations = iIterations = aIterations; 
}

/** Enables the task 
 *  schedules it for execution as soon as possible,
 *  and resets the RunCounter back to zero
 */
void Task::enable() {
	if (iScheduler) { // activation without active scheduler does not make sense
		iRunCounter = 0;
		if ( iOnEnable && !iStatus.inonenable ) {
			Task *current = iScheduler->iCurrent;
			iScheduler->iCurrent = this;
			iStatus.inonenable = true;		// Protection against potential infinite loop
			iStatus.enabled = (*iOnEnable)();
			iStatus.inonenable = false;	  	// Protection against potential infinite loop
			iScheduler->iCurrent = current;
		}
		else {
			iStatus.enabled = true;
		}
		iPreviousMillis = _TASK_TIME_FUNCTION() - (iDelay = iInterval);
	}
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
void Task::enableDelayed(unsigned long aDelay) {
	enable();
	delay(aDelay);
}

/** Delays Task for execution after a delay = aInterval (if task is enabled).
 * leaves task enabled or disabled
 * if aDelay is zero, delays for the original scheduling interval from now
 */
void Task::delay(unsigned long aDelay) {
//	if (!aDelay) aDelay = iInterval;
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
	if (previousEnabled && iOnDisable) {
		Task *current = iScheduler->iCurrent;
		iScheduler->iCurrent = this;
		(*iOnDisable)();
		iScheduler->iCurrent = current;
	}
	return (previousEnabled);
}

/** Restarts task
 * Task will run number of iterations again
 */
void Task::restart() {
	 iIterations = iSetIterations;
	 enable();
}

/** Restarts task delayed
 * Task will run number of iterations again
 */
void Task::restartDelayed(unsigned long aDelay) {
	 iIterations = iSetIterations;
	 enableDelayed(aDelay);
}