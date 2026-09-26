// test-scheduler-defect-fixes-oo.cpp
//
// Regression tests for the v4.1.0 defect fixes with _TASK_OO_CALLBACKS.
// The function-pointer tests are in test-scheduler-defect-fixes.cpp.
//
// The tests build with _TASK_NON_ARDUINO and a fake clock, so timing is
// deterministic and no test waits for wall-clock time.
//
// Covered fixes
// -------------
// D1:  a pass is idle only if no Callback() returned true (previously the
//      last invoked task decided).
// D2:  getInvokedTasks() counts OO callbacks (it always returned 0).
// D6:  the StatusRequest-only OO constructor clears the self-destruct flag.
// D20: a task may delete itself inside its own OnDisable override.

#define _TASK_NON_ARDUINO
#define _TASK_OO_CALLBACKS
#define _TASK_STATUS_REQUEST
#define _TASK_SELF_DESTRUCT
#define _TASK_SLEEP_ON_IDLE_RUN

#include <gtest/gtest.h>
#include <cstring>
#include <new>

static unsigned long g_ms = 1000;
static unsigned long g_us = 1000000;

#include "TaskScheduler.h"

static unsigned long _task_millis() { return g_ms; }
static unsigned long _task_micros() { return g_us; }
static void _task_yield() {}

static int s_sleeps = 0;
static int s_ondisable = 0;
static void countSleep(unsigned long) { s_sleeps++; }

// A task whose Callback() returns a fixed "productive" value
class FixedTask : public Task {
  public:
    FixedTask(Scheduler* aS, bool aProductive)
        : Task(TASK_IMMEDIATE, TASK_FOREVER, aS, false), iProductive(aProductive), iRuns(0) {}
    bool Callback() override { iRuns++; return iProductive; }
    bool iProductive;
    int  iRuns;
};

// A StatusRequest-only task (OO constructor)
class EventTask : public Task {
  public:
    explicit EventTask(Scheduler* aS) : Task(aS) {}
    bool Callback() override { return true; }
};

// A heap task that deletes itself in its OnDisable override
class SelfDeletingTask : public Task {
  public:
    SelfDeletingTask(Scheduler* aS, bool aSelfDestruct)
        : Task(TASK_IMMEDIATE, 1, aS, false, aSelfDestruct) {}
    bool Callback() override { return true; }
    void OnDisable() override { s_ondisable++; delete this; }
};

// ---------------------------------------------------------------------------
// D1 / D2: idle detection and invoked counter
// ---------------------------------------------------------------------------
TEST(DefectFixesOO, D1_ProductiveThenIdleIsNotIdle) {
    Scheduler ts;
    FixedTask a(&ts, true), b(&ts, false);
    a.enable(); b.enable();

    EXPECT_FALSE(ts.execute());          // a did work; b's false must not override it
    EXPECT_EQ(1, a.iRuns);
    EXPECT_EQ(1, b.iRuns);
}

TEST(DefectFixesOO, D1_IdleThenProductiveIsNotIdle) {
    Scheduler ts;
    FixedTask a(&ts, false), b(&ts, true);
    a.enable(); b.enable();

    EXPECT_FALSE(ts.execute());
}

TEST(DefectFixesOO, D1_AllIdleCallbacksGiveIdlePass) {
    Scheduler ts;
    FixedTask a(&ts, false), b(&ts, false);
    a.enable(); b.enable();

    EXPECT_TRUE(ts.execute());
}

TEST(DefectFixesOO, D1_NoSleepAfterProductivePass) {
    Scheduler ts;
    iSleepMethod = &countSleep;
    s_sleeps = 0;
    FixedTask a(&ts, true), b(&ts, false);
    a.enable(); b.enable();

    for (int i = 0; i < 3; i++) ts.execute();
    EXPECT_EQ(0, s_sleeps);

    a.disable();                         // only the idle task is left
    for (int i = 0; i < 3; i++) ts.execute();
    EXPECT_EQ(3, s_sleeps);
}

TEST(DefectFixesOO, D2_InvokedTasksCountsOOCallbacks) {
    Scheduler ts;
    FixedTask a(&ts, true), b(&ts, false), c(&ts, true);
    a.enable(); b.enable();              // c stays disabled

    ts.execute();
    EXPECT_EQ(3UL, ts.getTotalTasks());
    EXPECT_EQ(2UL, ts.getActiveTasks());
    EXPECT_EQ(2UL, ts.getInvokedTasks());
}

// ---------------------------------------------------------------------------
// D6: OO StatusRequest-only constructor
// ---------------------------------------------------------------------------
TEST(DefectFixesOO, D6_EventConstructorClearsSelfDestruct) {
    Scheduler ts;
    alignas(EventTask) unsigned char buf[sizeof(EventTask)];
    memset(buf, 0xFF, sizeof(buf));

    EventTask* t = new (buf) EventTask(&ts);
    EXPECT_FALSE(t->getSelfDestruct());
    t->~EventTask();
}

// ---------------------------------------------------------------------------
// D20: delete this inside the OnDisable override
// ---------------------------------------------------------------------------
TEST(DefectFixesOO, D20_DeleteThisInOnDisable) {
    Scheduler ts;
    s_ondisable = 0;
    SelfDeletingTask* t = new SelfDeletingTask(&ts, false);
    FixedTask after(&ts, true);
    t->enable(); after.enable();

    ts.execute();                        // t runs its only iteration
    ts.execute();                        // t is disabled and deletes itself
    ts.execute();
    EXPECT_EQ(1, s_ondisable);
    EXPECT_EQ(3, after.iRuns);
    EXPECT_EQ(1UL, ts.getChainLength());
}

TEST(DefectFixesOO, D20_SelfDestructTaskDeletingItselfIsNotDeletedTwice) {
    Scheduler ts;
    s_ondisable = 0;
    SelfDeletingTask* t = new SelfDeletingTask(&ts, true);
    t->enable();

    ts.execute();
    ts.execute();
    ts.execute();
    EXPECT_EQ(1, s_ondisable);
    EXPECT_EQ(0UL, ts.getChainLength());
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
