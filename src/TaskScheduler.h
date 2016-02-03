// Cooperative multitasking library for Arduino version 2.0.2
// Copyright (c) 2015 Anatoli Arkhipenko
//
// Changelog:
// v1.0.0:
//     2015-02-24 - Initial release 
//     2015-02-28 - added delay() and disableOnLastIteration() methods
//     2015-03-25 - changed scheduler execute() method for a more precise delay calculation:
//                  1. Do not delay if any of the tasks ran (making request for immediate execution redundant)
//                  2. Delay is invoked only if none of the tasks ran 
//                  3. Delay is based on the min anticipated wait until next task _AND_ the runtime of execute method itself.
//     2015-05-11 - added  restart() and restartDelayed() methods to restart tasks which are on hold after running all iterations
//     2015-05-19 - completely removed  delay from the scheduler since there are no power saving there. using 1 ms sleep instead
//
// v1.4.1:
//     2015-09-15 - more careful placement of AVR-specific includes for sleep method (compatibility with DUE)
//                          sleep on idle run is no longer a default and should be explicitly compiled with _TASK_SLEEP_ON_IDLE_RUN defined
//
// v1.5.0:
//	   2015-09-20 - access to currently executing task (for callback methods)
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
//	  2015-10-08 - introduced callback run counter - callback methods can branch on the iteration number. 
//	  2015-10-11 - enableIfNot() - enable a task only if it is not already enabled. Returns true if was already enabled, false if was disabled. 
//	  2015-10-11 - disable() returns previous enable state (true if was enabled, false if was already disabled)
//	  2015-10-11 - introduced callback methods "on enable" and "on disable". On enable runs every time enable is called, on disable runs only if task was enabled
//	  2015-10-12 - new Task method: forceNextIteration() - makes next iteration happen immediately during the next pass regardless how much time is left
//
// v1.8.0:
//	  2015-10-13 - support for status request objects allowing tasks waiting on requests
//	  2015-10-13 - moved to a single header file to allow compilation control via #defines from the main sketch
//
// v1.8.1:
//	  2015-10-22 - implement Task id and control points to support identification of failure points for watchdog timer logging
//
// v1.8.2:
//    2015-10-27 - implement Local Task Storage Pointer (allow use of same callback code for different tasks)
//    2015-10-27 - bug: currentTask() method returns incorrect Task reference if called within OnEnable and OnDisable methods
//    2015-10-27 - protection against infinite loop in OnEnable (if enable() methods are called within OnEnable)
//    2015-10-29 - new currentLts() method in the scheduler class returns current task's LTS pointer in one call
//
// v1.8.3:
//    2015-11-05 - support for task activation on a status request with arbitrary interval and number of iterations (0 and 1 are still default values)
//    2015-11-05 - implement waitForDelayed() method to allow task activation on the status request completion delayed for one current interval
//    2015-11-09 - added callback methods prototypes to all examples for Arduino IDE 1.6.6 compatibility
//    2015-11-14 - added several constants to be used as task parameters for readability (e.g, TASK_FOREVER, TASK_SECOND, etc.)
//    2015-11-14 - significant optimization of the scheduler's execute loop, including millis() rollover fix option
//
// v1.8.4:
//    2015-11-15 - bug fix: Task alignment with millis() for scheduling purposes should be done after OnEnable, not before. Especially since OnEnable method can change the interval
//    2015-11-16 - further optimizations of the task scheduler execute loop
//
// v1.8.5:
//    2015-11-23 - bug fix: incorrect calculation of next task invocation in case callback changed the interval
//    2015-11-23 - bug fix: Task::set() method calls setInterval() explicitly, therefore delaying the task in the same manner
//
// v1.9.0:
//    2015-11-24 - packed three byte-long status variables into bit array structure data type - saving 2 bytes per each task instance
//
// v1.9.2:
//    2015-11-28 - _TASK_ROLLOVER_FIX is deprecated (not necessary)
//    2015-12-16 - bug fixes: automatic millis rollover support for delay methods
//    2015-12-17 - new method for _TASK_TIMECRITICAL option: getStartDelay() 
//
// v2.0.0:
//    2015-12-22 - _TASK_PRIORITY - support for layered task prioritization
//
// v2.0.1:
//    2016-01-02 - bug fix: issue#11 Xtensa compiler (esp8266): Declaration of constructor does not match implementation
//
// v2.0.2:
//    2016-01-05 - bug fix: time constants wrapped inside compile option
//    2016-01-05 - support for ESP8266 wifi power saving mode for _TASK_SLEEP_ON_IDLE_RUN compile option
//
// v2.1.0:
//    2016-02-01 - support for microsecond resolution
//    2016-02-02 - added Scheduler baseline start time reset method: startNow()

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
 *  #define _TASK_TIMECRITICAL      // Enable monitoring scheduling overruns
 *  #define _TASK_SLEEP_ON_IDLE_RUN // Enable 1 ms SLEEP_IDLE powerdowns between tasks if no callback methods were invoked during the pass 
 *  #define _TASK_STATUS_REQUEST    // Compile with support for StatusRequest functionality - triggering tasks on status change events in addition to time only
 *  #define _TASK_WDT_IDS           // Compile with support for wdt control points and task ids
 *  #define _TASK_LTS_POINTER       // Compile with support for local task storage pointer
 *  #define _TASK_PRIORITY			// Support for layered scheduling priority
 *  #define _TASK_MICRO_RES			// Support for microsecond resolution
 */


 #ifdef _TASK_MICRO_RES
 
 #undef _TASK_SLEEP_ON_IDLE_RUN		// SLEEP_ON_IDLE has only millisecond resolution
 #define _TASK_TIME_FUNCTION() micros()
 
 #else
	 
 #define _TASK_TIME_FUNCTION() millis()
 
 #endif  // _TASK_MICRO_RES
 
 
#ifdef _TASK_SLEEP_ON_IDLE_RUN

#ifdef ARDUINO_ARCH_AVR  
#include <avr/sleep.h>
#include <avr/power.h>
#endif  // ARDUINO_ARCH_AVR 

#ifdef ARDUINO_ARCH_ESP8266
extern "C" {
#include "user_interface.h"
}
#define _TASK_ESP8266_DLY_THRESHOLD	200L
#endif  // ARDUINO_ARCH_ESP8266

#endif  // _TASK_SLEEP_ON_IDLE_RUN

#define TASK_IMMEDIATE			0
#define TASK_FOREVER		 (-1)
#define TASK_ONCE				1


#ifndef _TASK_MICRO_RES

#define TASK_SECOND			1000L
#define TASK_MINUTE		   60000L
#define TASK_HOUR		 3600000L

#else

#define TASK_SECOND		1000000L
#define TASK_MINUTE	   60000000L
#define TASK_HOUR	 3600000000L

#endif  // _TASK_MICRO_RES


#ifdef _TASK_STATUS_REQUEST

#define	_TASK_SR_NODELAY 	1
#define	_TASK_SR_DELAY		2

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
		unsigned int	iCount;  					// number of statuses to wait for. waiting for more that 65000 events seems unreasonable: unsigned int should be sufficient
		int			iStatus;  					// status of the last completed request. negative = error;  zero = OK; >positive = OK with a specific status
};
#endif  // _TASK_STATUS_REQUEST


typedef struct  {
	bool enabled : 1;							// indicates that task is enabled or not.
	bool inonenable : 1;						// indicates that task execution is inside OnEnable method (preventing infinite loops)
#ifdef _TASK_STATUS_REQUEST
	byte	waiting : 2;							// indication if task is waiting on the status request
#endif
} __task_status;

class Scheduler; 


#ifdef _TASK_WDT_IDS
	static unsigned int __task_id_counter = 0;		// global task ID counter for assiging task IDs automatically. 
#endif  // _TASK_WDT_IDS

class Task {
    friend class Scheduler;
    public:
		Task(unsigned long aInterval=0, long aIterations=0, void (*aCallback)()=NULL, Scheduler* aScheduler=NULL, bool aEnable=false, bool (*aOnEnable)()=NULL, void (*aOnDisable)()=NULL);
#ifdef _TASK_STATUS_REQUEST
		Task(void (*aCallback)()=NULL, Scheduler* aScheduler=NULL, bool (*aOnEnable)()=NULL, void (*aOnDisable)()=NULL);
#endif  // _TASK_STATUS_REQUEST

		void enable();
		bool enableIfNot();
		void enableDelayed(unsigned long aDelay=0);
		void delay(unsigned long aDelay=0);
		void forceNextIteration(); 
		void restart();
		void restartDelayed(unsigned long aDelay=0);
		bool disable();
		inline bool isEnabled() { return iStatus.enabled; }
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
		inline long getStartDelay() { return iStartDelay; }
#endif  // _TASK_TIMECRITICAL
		inline bool isFirstIteration() { return (iRunCounter <= 1); } 
		inline bool isLastIteration() { return (iIterations == 0); }
#ifdef _TASK_STATUS_REQUEST
		void waitFor(StatusRequest* aStatusRequest, unsigned long aInterval = 0, long aIterations = 1);
		void waitForDelayed(StatusRequest* aStatusRequest, unsigned long aInterval = 0, long aIterations = 1);
		inline StatusRequest* getStatusRequest() {return iStatusRequest; }
#endif  // _TASK_STATUS_REQUEST
#ifdef _TASK_WDT_IDS
		inline void setId(unsigned int aID) { iTaskID = aID; }
		inline unsigned int getId() { return iTaskID; }
		inline void setControlPoint(unsigned int aPoint) { iControlPoint = aPoint; }
		inline unsigned int getControlPoint() { return iControlPoint; }
#endif  // _TASK_WDT_IDS
#ifdef _TASK_LTS_POINTER
		inline void	setLtsPointer(void *aPtr) { iLTS = aPtr; }
		inline void* getLtsPointer() { return iLTS; }
#endif  // _TASK_LTS_POINTER
	
    private:
		void reset();

		volatile __task_status	iStatus;
		volatile unsigned long	iInterval;			// execution interval in milliseconds (or microseconds). 0 - immediate
		volatile unsigned long	iDelay; 			// actual delay until next execution (usually equal iInterval)
		volatile unsigned long	iPreviousMillis;	// previous invocation time (millis).  Next invocation = iPreviousMillis + iInterval.  Delayed tasks will "catch up" 
#ifdef _TASK_TIMECRITICAL
		volatile long			iOverrun; 			// negative if task is "catching up" to it's schedule (next invocation time is already in the past)
		volatile long			iStartDelay;		// actual execution of the task's callback method was delayed by this number of millis
#endif  // _TASK_TIMECRITICAL
		volatile long			iIterations;		// number of iterations left. 0 - last iteration. -1 - infinite iterations
		long					iSetIterations; 		// number of iterations originally requested (for restarts)
		unsigned long			iRunCounter;		// current number of iteration (starting with 1). Resets on enable. 
		void					(*iCallback)();		// pointer to the void callback method
		bool					(*iOnEnable)();		// pointer to the bolol OnEnable callback method
		void					(*iOnDisable)();	// pointer to the void OnDisable method
		Task					*iPrev, *iNext;		// pointers to the previous and next tasks in the chain
		Scheduler				*iScheduler;		// pointer to the current scheduler
#ifdef _TASK_STATUS_REQUEST
		StatusRequest			*iStatusRequest;	// pointer to the status request task is or was waiting on
#endif  // _TASK_STATUS_REQUEST
#ifdef _TASK_WDT_IDS
		unsigned int			iTaskID;			// task ID (for debugging and watchdog identification)
		unsigned int			iControlPoint;		// current control point within the callback method. Reset to 0 by scheduler at the beginning of each pass
#endif  // _TASK_WDT_IDS
#ifdef _TASK_LTS_POINTER
		void					*iLTS;				// pointer to task's local storage. Needs to be recast to appropriate type (usually a struct).
#endif  // _TASK_LTS_POINTER
};


#ifdef _TASK_PRIORITY
		static Scheduler* iCurrentScheduler;
#endif  // _TASK_PRIORITY

class Scheduler {
	friend class Task;
	public:
		Scheduler();
		void init();
		void addTask(Task& aTask);
		void deleteTask(Task& aTask);
		void disableAll(bool aRecursive = true);
		void enableAll(bool aRecursive = true);
		bool execute();			// Returns true if at none of the tasks' callback methods was invoked (true if idle run)
		void startNow(bool aRecursive = true); 			// reset ALL active tasks to immediate execution NOW.
		inline Task& currentTask() {return *iCurrent; }
#ifdef _TASK_SLEEP_ON_IDLE_RUN
		void allowSleep(bool aState = true);
#endif  // _TASK_SLEEP_ON_IDLE_RUN
#ifdef _TASK_LTS_POINTER
		inline void* currentLts() {return iCurrent->iLTS; }
#endif  // _TASK_LTS_POINTER
#ifdef _TASK_TIMECRITICAL
		inline bool isOverrun() { return (iCurrent->iOverrun < 0); }
#endif  // _TASK_TIMECRITICAL
#ifdef _TASK_PRIORITY
		void setHighPriorityScheduler(Scheduler* aScheduler);
		static Scheduler& currentScheduler() { return *(iCurrentScheduler); };
#endif  // _TASK_PRIORITY

	private:
		Task	*iFirst, *iLast, *iCurrent;			// pointers to first, last and current tasks in the chain
#ifdef _TASK_SLEEP_ON_IDLE_RUN
		bool	iAllowSleep;						// indication if putting avr to IDLE_SLEEP mode is allowed by the program at this time. 
#endif  // _TASK_SLEEP_ON_IDLE_RUN
#ifdef _TASK_PRIORITY
		Scheduler *iHighPriority;					// Pointer to a higher priority scheduler
#endif  // _TASK_PRIORITY
};


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



#endif /* _TASKSCHEDULER_H_ */
