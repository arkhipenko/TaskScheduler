// test-scheduler-non-arduino-split.cpp
//
// Regression test for D17: _TASK_NON_ARDUINO in a split build.
//
// This file is built together with src/TaskScheduler.cpp, with
// _TASK_HEADER_AND_CPP and _TASK_NON_ARDUINO given as build flags (see the
// test-non-arduino-split job in .github/workflows/test.yml). Like example 29,
// it includes only TaskSchedulerDeclarations.h and defines the time and yield
// hooks with external linkage. The scheduler code compiled in TaskScheduler.cpp
// must call these definitions.
//
// Before v4.1.1 the library declared the hooks static, so TaskScheduler.cpp
// referenced its own undefined internal functions. GCC 8 bound those calls to
// the user's definitions by accident; GCC 14 fails to link.
//
// The job builds this file twice: with the example 29 options
// (_TASK_SLEEP_ON_IDLE_RUN, _TASK_STATUS_REQUEST) and with _TASK_MICRO_RES.
// The fake clock returns the same tick count from both time hooks, so the
// tests read the same in milliseconds and in microseconds.

#include <gtest/gtest.h>

#include "TaskSchedulerDeclarations.h"

#if !defined(_TASK_HEADER_AND_CPP) || !defined(_TASK_NON_ARDUINO)
#error "Build with -D_TASK_HEADER_AND_CPP -D_TASK_NON_ARDUINO and src/TaskScheduler.cpp"
#endif

// ---------------------------------------------------------------------------
// User hooks: external linkage (not static), outside TaskScheduler.cpp.
// ---------------------------------------------------------------------------
static unsigned long g_ticks = 1000;
static unsigned long s_clockReads = 0;
static int           s_yields = 0;

unsigned long _task_millis() { s_clockReads++; return g_ticks; }
unsigned long _task_micros() { s_clockReads++; return g_ticks; }
void _task_yield() { s_yields++; }

// ---------------------------------------------------------------------------
// Callback state
// ---------------------------------------------------------------------------
static int s_runs = 0;

static void cbCount() { s_runs++; }
#ifdef _TASK_SLEEP_ON_IDLE_RUN
static int s_sleeps = 0;
static void countSleep(unsigned long) { s_sleeps++; }
#endif

TEST(NonArduinoSplit, SchedulerUsesUserClock) {
    s_runs = 0;
    s_clockReads = 0;
    Scheduler ts;
    Task t(100, TASK_FOREVER, &cbCount, &ts, true);
    EXPECT_GT(s_clockReads, 0UL);                         // enable() read the user clock

    ts.execute();
    EXPECT_EQ(1, s_runs);                                 // due at once
    g_ticks += 99; ts.execute();
    EXPECT_EQ(1, s_runs);
    g_ticks += 1;  ts.execute();
    EXPECT_EQ(2, s_runs);                                 // then every 100 ticks
}

TEST(NonArduinoSplit, ExecuteCallsUserYield) {
    s_yields = 0;
    Scheduler ts;
    Task t(100, TASK_FOREVER, &cbCount, &ts, true);
    ts.execute();
    EXPECT_GT(s_yields, 0);                               // once per task evaluated
}

#ifdef _TASK_SLEEP_ON_IDLE_RUN
TEST(NonArduinoSplit, IdlePassCallsSleepMethod) {
    s_sleeps = 0;
    Scheduler ts;
    ts.setSleepMethod(&countSleep);
    ts.allowSleep(true);
    Task t(100, TASK_FOREVER, &cbCount, &ts, true);

    ts.execute();                                         // the task runs: not idle
    EXPECT_EQ(0, s_sleeps);
    ts.execute();                                         // nothing due: idle
    EXPECT_EQ(1, s_sleeps);
}
#endif

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
