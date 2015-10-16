// Cooperative multitasking library for Arduino version 1.8.0
// Copyright (c) 2015 Anatoli Arkhipenko
//
// Changelog:
// v1.0.0:
//     2015-02-24 - Initial release 
//     2015-02-28 - added delay() and disableOnLastIteration() functions
//     2015-03-25 - changed scheduler execute() function for a more precise delay calculation:
//                  1. Do not delay if any of the tasks ran (making request for immediate execution redundant)
//                  2. Delay is invoked only if none of the tasks ran 
//                  3. Delay is based on the min anticipated wait until next task _AND_ the runtime of execute function itself.
//     2015-05-11 - added  restart() and restartDelayed() functions to restart tasks which are on hold after running all iterations
//     2015-05-19 - completely removed  delay from the scheduler since there are no power saving there. using 1 ms sleep instead
//
// v1.4.1:
//     2015-09-15 - more careful placement of AVR-specific includes for sleep functions (compatibility with DUE)
//                          sleep on idle run is no longer a default and should be explicitly compiled with _TASK_SLEEP_ON_IDLE_RUN defined
//
// v1.5.0:
//	   2015-09-20 - access to currently executing task (for callback functions)
//	   2015-09-20 - pass scheduler as a parameter to the task constructor to append the task to the end of the chain
//     2015-09-20 - option to create a task already enabled
//
// v1.5.1:
//	   2015-09-21 - bug fix: incorrect handling of active tasks via set() and setIterations(). 
//					Thanks to Hannes Morgenstern for catching this one
//
// v1.6.0:
//	   2015-09-22 - revert back to having all tasks disable on last iteration.
//	   2015-09-22 - deprecated disableOnLastIteration method as a result
//	   2015-09-22 - created a separate branch 'disable-on-last-iteration' for this
//	   2015-10-01 - made version numbers semver compliant (documentation only)
//
// v1.7.0:
//	  2015-10-08 - introduced callback run counter - callback functions can branch on the iteration number. 
//	  2015-10-11 - enableIfNot() - enable a task only if it is not already enabled. Returns true if was already enabled, false if was disabled. 
//	  2015-10-11 - disable() returns previous enable state (true if was enabled, false if was already disabled)
//	  2015-10-11 - introduced callback functions "on enable" and "on disable". On enable runs every time enable is called, on disable runs only if task was enabled
//	  2015-10-12 - new Task method: forceNextIteration() - makes next iteration happen immediately during the next pass regardless how much time is left
//
// v1.8.0:
//	  2015-10-13 - support for status request objects allowing tasks waiting on requests
//	  2015-10-13 - moved to a single header file to allow compilation control via #defines from the main sketch


/* ============================================
Cooperative multitasking library code is placed under the MIT license
Copyright (c) 2015 Anatoli Arkhipenko

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
===============================================
*/


#include <Arduino.h>


#ifndef _TASKSCHEDULER_H_
#define _TASKSCHEDULER_H_

/** ----------------------------------------
 * The following "defines" control library functionality at compile time,
 * and should be used in the main sketch depending on the functionality required
 * 
 *	#define _TASK_TIMECRITICAL		// Enable monitoring scheduling overruns
 *	#define _TASK_SLEEP_ON_IDLE_RUN	// Enable 1 ms SLEEP_IDLE powerdowns between tasks if no callback functions were invoked during the pass 
 *	#define _TASK_STATUS_REQUEST	// Compile with support for StatusRequest functionality - triggering tasks on status change events in addition to time only
 */

#ifdef _TASK_SLEEP_ON_IDLE_RUN
#include <avr/sleep.h>
#include <avr/power.h>
#endif


#ifdef _TASK_STATUS_REQUEST
class StatusRequest {
	public:
		StatusRequest() {iCount = 0; iStatus = 0; }
		inline void setWaiting(unsigned int aCount = 1) { iCount = aCount; iStatus = 0; }
		bool signal(int aStatus = 0);
		void signalComplete(int aStatus = 0);
		inline bool pending() { return (iCount != 0); }
		inline bool completed() { return (iCount == 0); }
		inline int getStatus() { return iStatus; }
		
	private:
		unsigned int	iCount;
		int			iStatus;  // negative = error;  zero = OK; >positive = OK with a specific status
};
#endif


class Task; 

class Scheduler {
	public:
		Scheduler();
		inline void init() { iFirst = NULL; iLast = NULL; iCurrent = NULL; }
		void addTask(Task& aTask);
		void deleteTask(Task& aTask);
		void disableAll();
		void enableAll();
		void execute();
		inline Task& currentTask() {return *iCurrent; }
#ifdef _TASK_SLEEP_ON_IDLE_RUN
		void allowSleep(bool aState = true) { iAllowSleep = aState; }
#endif

	private:
		Task			*iFirst, *iLast, *iCurrent;
#ifdef _TASK_SLEEP_ON_IDLE_RUN
		bool	iAllowSleep;
#endif
};

class Task {
    friend class Scheduler;
    public:
	Task(unsigned long aInterval=0, long aIterations=0, void (*aCallback)()=NULL, Scheduler* aScheduler=NULL, boolean aEnable=false, bool (*aOnEnable)()=NULL, void (*aOnDisable)()=NULL);
#ifdef _TASK_STATUS_REQUEST
	Task(void (*aCallback)()=NULL, Scheduler* aScheduler=NULL, bool (*aOnEnable)()=NULL, void (*aOnDisable)()=NULL);
#endif

	void enable();
	bool enableIfNot();
	void enableDelayed(unsigned long aDelay=0);
	void delay(unsigned long aDelay=0);
	void forceNextIteration(); 
	void restart();
	void restartDelayed(unsigned long aDelay=0);
	bool disable();
	inline bool isEnabled() { return iEnabled; }
	void set(unsigned long aInterval, long aIterations, void (*aCallback)(),bool (*aOnEnable)()=NULL, void (*aOnDisable)()=NULL);
	void setInterval(unsigned long aInterval);
	inline unsigned long getInterval() { return iInterval; }
	void setIterations(long aIterations);
	inline long getIterations() { return iIterations; }
	inline unsigned long getRunCounter() { return iRunCounter; }
	inline void setCallback(void (*aCallback)()) { iCallback = aCallback; }
	inline void setOnEnable(bool (*aCallback)()) { iOnEnable = aCallback; }
	inline void setOnDisable(void (*aCallback)()) { iOnDisable = aCallback; }
#ifdef _TASK_TIMECRITICAL
	inline long getOverrun() { return iOverrun; }
#endif
	inline bool isFirstIteration() { return (iRunCounter <= 1); } 
	inline bool isLastIteration() { return (iIterations == 0); }
#ifdef _TASK_STATUS_REQUEST
	void waitFor(StatusRequest* aStatusRequest);
	inline StatusRequest* getStatusRequest() {return iStatusRequest; }
#endif
	
	
    private:
	void reset();

    volatile bool			iEnabled;
    volatile unsigned long	iInterval;
	volatile unsigned long	iPreviousMillis;
#ifdef _TASK_TIMECRITICAL
	volatile long			iOverrun; 
#endif
	volatile long			iIterations;
	long					iSetIterations; 
	unsigned long			iRunCounter;
	void					(*iCallback)();
	bool					(*iOnEnable)();
	void					(*iOnDisable)();
	Task					*iPrev, *iNext;
	Scheduler				*iScheduler;
#ifdef _TASK_STATUS_REQUEST
	StatusRequest			*iStatusRequest;
#endif
};


// ------------------ TaskScheduler implementation --------------------

/** Constructor, uses default values for the parameters
 * so could be called with no parameters.
 */
Task::Task( unsigned long aInterval, long aIterations, void (*aCallback)(), Scheduler* aScheduler, bool aEnable, bool (*aOnEnable)(), void (*aOnDisable)() ) {
	reset();
	set(aInterval, aIterations, aCallback, aOnEnable, aOnDisable);
	if (aScheduler) aScheduler->addTask(*this);
	if (aEnable) enable();
#ifdef _TASK_STATUS_REQUEST
	iStatusRequest = NULL;
#endif
}


#ifdef _TASK_STATUS_REQUEST

/** Constructor with reduced parameter list for tasks created for 
 *  StatusRequest only triggering (always immediate and only 1 iteration)
 */
Task::Task( void (*aCallback)(), Scheduler* aScheduler, bool (*aOnEnable)(), void (*aOnDisable)() ) {
	reset();
	set(0, 1, aCallback, aOnEnable, aOnDisable);
	if (aScheduler) aScheduler->addTask(*this);
	iStatusRequest = NULL;
}

/** Signals completion of the StatusRequest by one of the participating events
 *  @param: aStatus - if provided, sets the return code of the StatusRequest: negative = error, 0 (default) = OK, positive = OK with a specific status code
 *  Negative status will complete Status Request fully (since an error occured).
 *  @return: true, if StatusRequest is complete, false otherwise (still waiting for other events)
 */
bool StatusRequest::signal(int aStatus) {
	if ( iCount) {	// do not update the status request if it was already completed
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
void Task::waitFor(StatusRequest* aStatusRequest) {
	if ( ( iStatusRequest = aStatusRequest) ) { // assign internal StatusRequest var and check if it is not NULL
		setIterations(1);
		setInterval(0); 
		enable();
	}
}

#endif

/** Resets (initializes) the task/
 * Task is not enabled and is taken out 
 * out of the execution chain as a result
 */
void Task::reset() {
	iEnabled = false;
	iPreviousMillis = 0;
	iPrev = NULL;
	iNext = NULL;
	iScheduler = NULL;
	iRunCounter = 0;
#ifdef _TASK_TIMECRITICAL
	iOverrun = 0;
#endif
}

/** Explicitly set Task execution parameters
 * @param aInterval - execution interval in ms
 * @param aIterations - number of iterations, use -1 for no limit
 * @param aCallback - pointer to the callback function which executes the task actions
 * @param aOnEnable - pointer to the callback function which is called on enable()
 * @param aOnDisable - pointer to the callback function which is called on disable()
 */
void Task::set(unsigned long aInterval, long aIterations, void (*aCallback)(),bool (*aOnEnable)(), void (*aOnDisable)()) {
	iInterval = aInterval;
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
	iRunCounter = 0;
	iPreviousMillis = millis() - iInterval;
	iEnabled = iOnEnable ? (*iOnEnable)() : true;
}

/** Enables the task only if it was not enabled already
 * Returns previous state (true if was already enabled, false if was not)
 */
bool Task::enableIfNot() {
	bool previousEnabled = iEnabled;
	if (!iEnabled) enable();
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
	if (!aDelay) aDelay = iInterval;
	iPreviousMillis = millis() - iInterval + aDelay;
}

/** Schedules next iteration of Task for execution immediately (if enabled)
 * leaves task enabled or disabled
 * Task's original schedule is shifted, and all subsequent iterations will continue from this point in time
 */
void Task::forceNextIteration() {
	iPreviousMillis = millis() - iInterval;
}

/** Sets the execution interval.
 * Task execution is delayed for aInterval
 * Use  enable() to schedule execution ASAP
 * @param aInterval - new execution interval
 */
void Task::setInterval (unsigned long aInterval) {
 	iInterval = aInterval; 
	delay();
}

/** Disables task
 * Task will no longer be executed by the scheduler
 * Returns status of the task before disable was called (i.e., if the task was already disabled)
 */
bool Task::disable() {
	bool previousEnabled = iEnabled;
	iEnabled = false;
	if (previousEnabled && iOnDisable) (*iOnDisable)();
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

// ------------------ Scheduler implementation --------------------

/** Default constructor.
 * Creates a scheduler with an empty execution chain.
 */
Scheduler::Scheduler() {
	init();
#ifdef _TASK_SLEEP_ON_IDLE_RUN
	iAllowSleep = true;
#endif
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
 */
void Scheduler::disableAll() {
	Task	*current = iFirst;
	while (current) {
		current->disable();
		current = current->iNext;
	}
}


/** Enables all the tasks in the execution chain
 */
 void Scheduler::enableAll() {
	Task	*current = iFirst;
	while (current) {
		current->enable();
		current = current->iNext;
	}
}

/** Makes one pass through the execution chain.
 * Tasks are executed in the order they were added to the chain
 * There is no concept of priority
 * Different pseudo "priority" could be achieved
 * by running task more frequently 
 */
void Scheduler::execute() {
#ifdef _TASK_SLEEP_ON_IDLE_RUN
	bool		idleRun = true;
#endif
	
	iCurrent = iFirst;

	while (iCurrent) { 
		do {   		
			if (iCurrent->iEnabled) {
				if (iCurrent->iIterations == 0) {
					iCurrent->disable();
					break;
				}
	#ifdef  _TASK_STATUS_REQUEST
	// If StatusRequest object was provided (not NULL) and it is still bending, this task should not run
	// Otherwise, continue with execution as usual.  Tasks waiting to StatusRequest usually have  interval = 0, and iterations = 1
	// so they will run once immediately. 
				if ( iCurrent->iStatusRequest && (iCurrent->iStatusRequest)->pending() ) break;
	#endif
				if ( iCurrent->iInterval > 0 ) {
					unsigned long targetMillis = iCurrent->iPreviousMillis + iCurrent->iInterval;
					if ( targetMillis <= millis() ) {
						if ( iCurrent->iIterations > 0 ) iCurrent->iIterations--;  // do not decrement (-1) being a signal of eternal task
						iCurrent->iRunCounter++;
						iCurrent->iPreviousMillis += iCurrent->iInterval;

	#ifdef _TASK_TIMECRITICAL
	// Updated_previous+current should put us into the future, so iOverrun should be positive or zero. 
	// If negative - the task is behind (next execution time is already in the past) 
						iCurrent->iOverrun = (long) (iCurrent->iPreviousMillis + iCurrent->iInterval - millis());
	#endif

						if ( iCurrent->iCallback ) {
							( *(iCurrent->iCallback) )();
	#ifdef _TASK_SLEEP_ON_IDLE_RUN
							idleRun = false;
	#endif
						}
						break;
					}
				}
				else {
					if ( iCurrent->iIterations > 0 ) iCurrent->iIterations--;  // do not decrement (-1) being a signal of eternal task
					iCurrent->iRunCounter++;
					if ( iCurrent->iCallback ) {
						( *(iCurrent->iCallback) )();
	#ifdef _TASK_SLEEP_ON_IDLE_RUN
						idleRun = false;
	#endif
					}
				}
			}
		} while (0); //guaranteed single run - allows use of "break" to exit 
		iCurrent = iCurrent->iNext;
	}

#ifdef _TASK_SLEEP_ON_IDLE_RUN
  	if (idleRun && iAllowSleep) {
  	  set_sleep_mode(SLEEP_MODE_IDLE);
  	  sleep_enable();
	  /* Now enter sleep mode. */
	  sleep_mode();
	  
	  /* The program will continue from here after the timer timeout ~1 ms */
	  sleep_disable(); /* First thing to do is disable sleep. */
	}
#endif
}



#endif /* _TASKSCHEDULER_H_ */
