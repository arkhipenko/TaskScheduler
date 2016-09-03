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


//#include <Arduino.h>
#include <stddef.h> //for NULL

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

#define TASK_IMMEDIATE			0
#define TASK_FOREVER		 (-1)
#define TASK_ONCE				1


#ifdef _TASK_PRIORITY
	class Scheduler;
	extern Scheduler* iCurrentScheduler;
#endif  // _TASK_PRIORITY



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
#endif /* _TASKSCHEDULER_H_ */
