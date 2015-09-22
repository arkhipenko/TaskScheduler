// Cooperative multitasking library for Arduino version 1.51
// Copyright (c) 2015 Anatoli Arkhipenko
//
// Changelog:
//     2015-02-24 - Initial release 
//     2015-02-28 - added delay() and disableOnLastIteration() functions
//     2015-03-25 - changed scheduler execute() function for a more precise delay calculation:
//                  1. Do not delay if any of the tasks ran (making request for immediate execution redundant)
//                  2. Delay is invoked only if none of the tasks ran 
//                  3. Delay is based on the min anticipated wait until next task _AND_ the runtime of execute function itself.
//     2015-05-11 - added  restart() and restartDelayed() functions to restart tasks which are on hold after running all iterations
//     2015-05-19 - completely removed  delay from the scheduler since there are no power saving there. using 1 ms sleep instead
// v1.41:
//     2015-09-15 - more careful placement of AVR-specific includes for sleep functions (compatibility with DUE)
//                          sleep on idle run is no longer a default and should be explicitly compiled with _TASK_SLEEP_ON_IDLE_RUN defined
// v1.50:
//	   2015-09-20 - access to currently executing task (for callback functions)
//	   2015-09-20 - pass scheduler as a parameter to the task constructor to append the task to the end of the chain
//     2015-09-20 - option to create a task already enabled
// v1.51:
//	   2015-09-21 - bug fix: incorrect handling of active tasks via set() and setIterations(). 
//					Thanks to Hannes Morgenstern for catching this one


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

//#define _TASK_DEBUG
//#define _TASK_TIMECRITICAL
//#define _TASK_SLEEP_ON_IDLE_RUN


#ifdef _TASK_SLEEP_ON_IDLE_RUN
#include <avr/sleep.h>
#include <avr/power.h>
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
		void allowSleep(bool aState) { iAllowSleep = aState; }
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
	Task(unsigned long aInterval=0, long aIterations=0, void (*aCallback)()=NULL, Scheduler* aScheduler=NULL, boolean aEnable=false);

	void enable();
	void enableDelayed(unsigned long aDelay=0);
	void delay(unsigned long aDelay=0);
	void restart();
	void restartDelayed(unsigned long aDelay=0);
	void disable();
	inline bool isEnabled() { return iEnabled; }
	void set(unsigned long aInterval, long aIterations, void (*aCallback)());
	void setInterval(unsigned long aInterval);
	inline unsigned long getInterval() { return iInterval; }
	void setIterations(long aIterations);
	inline long getIterations() { return iIterations; }
	inline void setCallback(void (*aCallback)()) { iCallback = aCallback; }
	inline void disableOnLastIteration(bool aBool) { iDisableOnLastIteration = aBool; }  // default is false
#ifdef _TASK_TIMECRITICAL
	inline long getOverrun() { return iOverrun; }
#endif
	inline bool isFirstIteration() { return (iIterations >= iSetIterations-1); } 
	inline bool isLastIteration() { return (iIterations == 0); }

    private:
	void reset();

    volatile bool			iEnabled;
	volatile bool			iDisableOnLastIteration;
    volatile unsigned long	iInterval;
	volatile unsigned long	iPreviousMillis;
#ifdef _TASK_TIMECRITICAL
	volatile long			iOverrun; 
#endif
	volatile long			iIterations;
	long					iSetIterations; 
	void					(*iCallback)();
	Task					*iPrev, *iNext;
	Scheduler				*iScheduler;
};


#endif /* _TASKSCHEDULER_H_ */
