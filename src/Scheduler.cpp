#include "TaskScheduler.h"

Task& Scheduler::currentTask() {return *iCurrent; }
#ifdef _TASK_LTS_POINTER
void* Scheduler::currentLts() {return iCurrent->iLTS; }
#endif  // _TASK_LTS_POINTER
#ifdef _TASK_TIMECRITICAL
bool Scheduler::isOverrun() { return (iCurrent->iOverrun < 0); }
#endif  // _TASK_TIMECRITICAL
#ifdef _TASK_PRIORITY
static Scheduler& Scheduler::currentScheduler() { return *(iCurrentScheduler); };
#endif  // _TASK_PRIORITY

// ------------------ Scheduler implementation --------------------

/** Default constructor.
 * Creates a scheduler with an empty execution chain.
 */
Scheduler::Scheduler() {
	init();
}

/** Initializes all internal varaibles
 */
void Scheduler::init() { 
	iFirst = NULL; 
	iLast = NULL; 
	iCurrent = NULL; 
#ifdef _TASK_PRIORITY
	iHighPriority = NULL;
#endif  // _TASK_PRIORITY
#ifdef _TASK_SLEEP_ON_IDLE_RUN
	allowSleep(true);
#endif  // _TASK_SLEEP_ON_IDLE_RUN
}

/** Appends task aTask to the tail of the execution chain.
 * @param &aTask - reference to the Task to be appended.
 * @note Task can only be part of the chain once.
 */
 void Scheduler::addTask(Task& aTask) {

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
void Scheduler::disableAll(bool aRecursive) {
	Task	*current = iFirst;
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
 void Scheduler::enableAll(bool aRecursive) {
	Task	*current = iFirst;
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
		iHighPriority->allowSleep(false);		// Higher priority schedulers should not do power management
	}
#endif  // _TASK_SLEEP_ON_IDLE_RUN
};
#endif  // _TASK_PRIORITY


#ifdef _TASK_SLEEP_ON_IDLE_RUN
void Scheduler::allowSleep(bool aState) { 
	iAllowSleep = aState; 

#ifdef ARDUINO_ARCH_ESP8266
	wifi_set_sleep_type( iAllowSleep ? LIGHT_SLEEP_T : NONE_SLEEP_T );
#endif  // ARDUINO_ARCH_ESP8266

}
#endif  // _TASK_SLEEP_ON_IDLE_RUN


void Scheduler::startNow( bool aRecursive ) {
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

/** Makes one pass through the execution chain.
 * Tasks are executed in the order they were added to the chain
 * There is no concept of priority
 * Different pseudo "priority" could be achieved
 * by running task more frequently 
 */
bool Scheduler::execute() {
	bool	 idleRun = true;
	register unsigned long m, i;  // millis, interval;

#ifdef ARDUINO_ARCH_ESP8266
	  unsigned long t1 = micros();
	  unsigned long t2 = 0;
#endif  // ARDUINO_ARCH_ESP8266

	iCurrent = iFirst;
	
	while (iCurrent) {
		
#ifdef _TASK_PRIORITY
	// If scheduler for higher priority tasks is set, it's entire chain is executed on every pass of the base scheduler
		if (iHighPriority) idleRun = iHighPriority->execute() && idleRun; 
		iCurrentScheduler = this;
#endif  // _TASK_PRIORITY

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

#ifdef  _TASK_STATUS_REQUEST
	// If StatusRequest object was provided, and still pending, and task is waiting, this task should not run
	// Otherwise, continue with execution as usual.  Tasks waiting to StatusRequest need to be rescheduled according to 
	// how they were placed into waiting state (waitFor or waitForDelayed)
				if ( iCurrent->iStatus.waiting ) {
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
				iCurrent->iPreviousMillis += iCurrent->iDelay;

#ifdef _TASK_TIMECRITICAL
	// Updated_previous+current interval should put us into the future, so iOverrun should be positive or zero. 
	// If negative - the task is behind (next execution time is already in the past) 
				unsigned long p = iCurrent->iPreviousMillis;
				iCurrent->iOverrun = (long) ( p + i - m );
				iCurrent->iStartDelay = (long) ( m - p ); 
#endif  // _TASK_TIMECRITICAL

				iCurrent->iDelay = i;
				if ( iCurrent->iCallback ) {
					( *(iCurrent->iCallback) )();
					idleRun = false;
				}
			}
		} while (0); 	//guaranteed single run - allows use of "break" to exit 
		iCurrent = iCurrent->iNext;
	}

#ifdef _TASK_SLEEP_ON_IDLE_RUN
  	if (idleRun && iAllowSleep) {

#ifdef ARDUINO_ARCH_AVR	// Could be used only for AVR-based boards. 
  	  set_sleep_mode(SLEEP_MODE_IDLE);
  	  sleep_enable();
	  /* Now enter sleep mode. */
	  sleep_mode();
	  
	  /* The program will continue from here after the timer timeout ~1 ms */
	  sleep_disable(); /* First thing to do is disable sleep. */
#endif // ARDUINO_ARCH_AVR

#ifdef ARDUINO_ARCH_ESP8266
// to do: find suitable sleep function for esp8266
	  t2 = micros() - t1;
	  if (t2 < _TASK_ESP8266_DLY_THRESHOLD) delay(1); 	// ESP8266 implementation of delay() uses timers and yield
#endif  // ARDUINO_ARCH_ESP8266
	}
#endif  // _TASK_SLEEP_ON_IDLE_RUN

	return (idleRun);
}


