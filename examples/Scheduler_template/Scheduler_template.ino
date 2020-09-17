/*
    Your code description here
    (c) you, 20XX
    All rights reserved.

    Functionality: 
    
    Version log:
    
    20XX-MM-DD:
        v1.0.0  - Initial release
        
*/
// ==== DEFINES ===================================================================================

// ==== Debug and Test options ==================
#define _DEBUG_
//#define _TEST_

//===== Debugging macros ========================
#ifdef _DEBUG_
#define SerialD Serial
#define _PM(a) SerialD.print(millis()); SerialD.print(": "); SerialD.println(a)
#define _PP(a) SerialD.print(a)
#define _PL(a) SerialD.println(a)
#define _PX(a) SerialD.println(a, HEX)
#else
#define _PM(a)
#define _PP(a)
#define _PL(a)
#define _PX(a)
#endif




// ==== INCLUDES ==================================================================================

// ==== Uncomment desired compile options =================================
// #define _TASK_SLEEP_ON_IDLE_RUN  // Enable 1 ms SLEEP_IDLE powerdowns between tasks if no callback methods were invoked during the pass
// #define _TASK_TIMECRITICAL       // Enable monitoring scheduling overruns
// #define _TASK_STATUS_REQUEST     // Compile with support for StatusRequest functionality - triggering tasks on status change events in addition to time only
// #define _TASK_WDT_IDS            // Compile with support for wdt control points and task ids
// #define _TASK_LTS_POINTER        // Compile with support for local task storage pointer
// #define _TASK_PRIORITY           // Support for layered scheduling priority
// #define _TASK_MICRO_RES          // Support for microsecond resolution
// #define _TASK_STD_FUNCTION       // Support for std::function (ESP8266 and ESP32 ONLY)
// #define _TASK_DEBUG              // Make all methods and variables public for debug purposes
// #define _TASK_INLINE             // Make all methods "inline" - needed to support some multi-tab, multi-file implementations
// #define _TASK_TIMEOUT            // Support for overall task timeout
// #define _TASK_OO_CALLBACKS       // Support for dynamic callback method binding
// #define _TASK_DEFINE_MILLIS      // Force forward declaration of millis() and micros() "C" style
// #define _TASK_EXPOSE_CHAIN       // Methods to access tasks in the task chain
// #define _TASK_SCHEDULING_OPTIONS // Support for multiple scheduling options

#include <TaskScheduler.h>



// ==== GLOBALS ===================================================================================
// ==== Scheduler ==============================
Scheduler ts;

void task1Callback();
void task2Callback();

// ==== Scheduling defines (cheat sheet) =====================
/*
  TASK_MILLISECOND
  TASK_SECOND
  TASK_MINUTE
  TASK_HOUR
  TASK_IMMEDIATE
  TASK_FOREVER
  TASK_ONCE
  TASK_NOTIMEOUT
  
  TASK_SCHEDULE     - schedule is a priority, with "catch up" (default)
  TASK_SCHEDULE_NC  - schedule is a priority, without "catch up"
  TASK_INTERVAL     - interval is a priority, without "catch up"
*/

// ==== Task definitions ========================
Task t1 (100 * TASK_MILLISECOND, TASK_FOREVER, &task1Callback, &ts, true);
Task t2 (TASK_IMMEDIATE, 100, &task2Callback, &ts, true);



// ==== CODE ======================================================================================

/**************************************************************************/
/*!
    @brief    Standard Arduino SETUP method - initialize sketch
    @param    none
    @returns  none
*/
/**************************************************************************/
void setup() {
  // put your setup code here, to run once:
#if defined(_DEBUG_) || defined(_TEST_)
  Serial.begin(115200);
  delay(2000);
  _PL("Scheduler Template: setup()");
#endif
}


/**************************************************************************/
/*!
    @brief    Standard Arduino LOOP method - using with TaskScheduler there 
              should be nothing here but ts.execute()
    @param    none
    @returns  none
*/
/**************************************************************************/
void loop() {
  ts.execute();
}


/**************************************************************************/
/*!
    @brief    Callback method of task1 - explain
    @param    none
    @returns  none
*/
/**************************************************************************/
void task1Callback() {
_PM("task1Callback()");
//  task code
}


/**************************************************************************/
/*!
    @brief    Callback method of task2 - explain
    @param    none
    @returns  none
*/
/**************************************************************************/
void task2Callback() {
_PM("task2Callback()");
//  task code
}




