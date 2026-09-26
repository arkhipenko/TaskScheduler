// test-scheduler-defect-fixes.cpp
//
// Regression tests for the v4.1.0 defect fixes (function-pointer callbacks).
// The object-oriented callback fixes are in test-scheduler-defect-fixes-oo.cpp.
//
// The tests build with _TASK_NON_ARDUINO and a fake clock (g_ms / g_us), so
// timing is deterministic and no test waits for wall-clock time.
//
// Covered fixes
// -------------
// D3:  cancel() completes the internal StatusRequest with TASK_SR_CANCEL.
//      Tasks waiting on a canceled task run; abort() still aborts waiters.
// D6:  the StatusRequest-only constructor clears the self-destruct flag.
// D7:  setIntervalNodelay(..., TASK_INTERVAL_RECALC) clamps the pending delay
//      at 0 instead of wrapping it.
// D8:  tickless next run with _TASK_TIMEOUT: tasks without a timeout count,
//      a sooner timeout counts, and the estimate survives a counter rollover.
// D9:  with priority layers the base scheduler sleeps, even when a higher
//      layer was constructed last.
// D20: a task may be deleted inside its own OnDisable (execute, disable,
//      disableAll, timeout, self-destruct and nested paths).
// D21: destroying a Scheduler detaches its tasks; Scheduler is not copyable.
// D27: setIntervalNodelay() options behave as documented: KEEP keeps the
//      scheduled next run, RESET restarts the schedule from now.

#define _TASK_NON_ARDUINO
#define _TASK_STATUS_REQUEST
#define _TASK_TIMEOUT
#define _TASK_SELF_DESTRUCT
#define _TASK_TICKLESS
#define _TASK_PRIORITY
#define _TASK_SLEEP_ON_IDLE_RUN

#include <gtest/gtest.h>
#include <cstring>
#include <new>
#include <type_traits>

// ---------------------------------------------------------------------------
// Fake clock. The library declares these hooks static under _TASK_NON_ARDUINO,
// so they are defined in this translation unit, after the include.
// ---------------------------------------------------------------------------
static unsigned long g_ms = 1000;
static unsigned long g_us = 1000000;

#include "TaskScheduler.h"

static unsigned long _task_millis() { return g_ms; }
static unsigned long _task_micros() { return g_us; }
static void _task_yield() {}

// ---------------------------------------------------------------------------
// Callback state
// ---------------------------------------------------------------------------
static int   s_runs = 0;
static int   s_runs2 = 0;
static int   s_ondisable = 0;
static int   s_sleeps = 0;
static Task* s_victim = nullptr;   // task deleted by an OnDisable callback
static Task* s_nested = nullptr;   // task disabled from another OnDisable

static void cbNop() {}
static void cbCount() { s_runs++; }
static void cbCount2() { s_runs2++; }
static void odCount() { s_ondisable++; }
static void countSleep(unsigned long) { s_sleeps++; }

static void odDeleteVictim() {
    s_ondisable++;
    Task* t = s_victim;
    s_victim = nullptr;
    delete t;
}

static void odDisableNested() {
    s_ondisable++;
    s_nested->disable();
}

static void resetCounters() {
    s_runs = 0;
    s_runs2 = 0;
    s_ondisable = 0;
    s_sleeps = 0;
    s_victim = nullptr;
    s_nested = nullptr;
}

// ---------------------------------------------------------------------------
// D3: cancel() status code
// ---------------------------------------------------------------------------
TEST(DefectFixes, D3_CancelCompletesWithCancelCodeAndWaiterRuns) {
    resetCounters();
    Scheduler ts;
    Task producer(1000, TASK_FOREVER, &cbNop, &ts, true);
    Task waiter(&cbCount, &ts, NULL, &odCount);

    ASSERT_TRUE(waiter.waitFor(producer.getInternalStatusRequest()));
    producer.cancel();

    EXPECT_TRUE(producer.isCanceled());
    EXPECT_EQ(TASK_SR_CANCEL, producer.getInternalStatusRequest()->getStatus());

    ts.execute();
    EXPECT_EQ(1, s_runs);                 // the waiter runs, it is not aborted
    EXPECT_FALSE(waiter.isCanceled());
    EXPECT_EQ(TASK_SR_CANCEL, waiter.getStatusRequest()->getStatus());
}

TEST(DefectFixes, D3_AbortStillAbortsWaiters) {
    resetCounters();
    Scheduler ts;
    Task producer(1000, TASK_FOREVER, &cbNop, &ts, true);
    Task waiter(&cbCount, &ts, NULL, &odCount);

    ASSERT_TRUE(waiter.waitFor(producer.getInternalStatusRequest()));
    producer.abort();
    EXPECT_EQ(TASK_SR_ABORT, producer.getInternalStatusRequest()->getStatus());

    ts.execute();
    EXPECT_EQ(0, s_runs);
    EXPECT_FALSE(waiter.isEnabled());
    EXPECT_TRUE(waiter.isCanceled());
    EXPECT_EQ(0, s_ondisable);            // abort() does not call OnDisable
}

// ---------------------------------------------------------------------------
// D6: self-destruct flag initialized by every constructor
// ---------------------------------------------------------------------------
TEST(DefectFixes, D6_EventConstructorClearsSelfDestruct) {
    Scheduler ts;
    alignas(Task) unsigned char buf[sizeof(Task)];
    memset(buf, 0xFF, sizeof(buf));       // simulate dirty heap or stack memory

    Task* t = new (buf) Task(&cbNop, &ts);
    EXPECT_FALSE(t->getSelfDestruct());
    t->~Task();
}

TEST(DefectFixes, D6_MainConstructorHonorsSelfDestructArgument) {
    Scheduler ts;
    alignas(Task) unsigned char buf[sizeof(Task)];
    memset(buf, 0xFF, sizeof(buf));

    Task* t = new (buf) Task(100, 1, &cbNop, &ts, false, NULL, NULL, false);
    EXPECT_FALSE(t->getSelfDestruct());
    t->~Task();
}

// ---------------------------------------------------------------------------
// D7: setIntervalNodelay(..., TASK_INTERVAL_RECALC)
// ---------------------------------------------------------------------------
TEST(DefectFixes, D7_RecalcClampsPendingDelayAtZero) {
    resetCounters();
    Scheduler ts;
    Task t(1000, TASK_FOREVER, &cbCount, &ts, false);

    t.enableDelayed(10);                                  // pending delay 10 ms
    t.setIntervalNodelay(500, TASK_INTERVAL_RECALC);      // reduction 500 > 10
    EXPECT_EQ(500UL, t.getInterval());
    EXPECT_EQ(0, ts.timeUntilNextIteration(t));

    ts.execute();
    EXPECT_EQ(1, s_runs);                                 // due immediately, no stall
    g_ms += 499; ts.execute();
    EXPECT_EQ(1, s_runs);
    g_ms += 1;   ts.execute();
    EXPECT_EQ(2, s_runs);                                 // then every 500 ms
}

TEST(DefectFixes, D7_RecalcAdjustsPendingDelayByDifference) {
    resetCounters();
    Scheduler ts;
    Task t(1000, TASK_FOREVER, &cbCount, &ts, true);
    ts.execute();                                         // first run, next due in 1000 ms
    ASSERT_EQ(1, s_runs);

    g_ms += 100;
    t.setIntervalNodelay(400, TASK_INTERVAL_RECALC);      // pending 1000 -> 400
    EXPECT_EQ(300, ts.timeUntilNextIteration(t));

    t.setIntervalNodelay(700, TASK_INTERVAL_RECALC);      // pending 400 -> 700
    EXPECT_EQ(600, ts.timeUntilNextIteration(t));
}

// ---------------------------------------------------------------------------
// D8: tickless next run with _TASK_TIMEOUT
// ---------------------------------------------------------------------------
TEST(DefectFixes, D8_TicklessCountsTaskWithoutTimeout) {
    Scheduler ts;
    Task t(1000, TASK_FOREVER, &cbNop, &ts, true);

    ts.execute();                 // runs now
    g_ms += 100;
    ts.execute();                 // idle pass
    EXPECT_EQ(900UL, ts.getNextRun());
}

TEST(DefectFixes, D8_TicklessUsesSoonerTimeout) {
    Scheduler ts;
    Task t(1000, TASK_FOREVER, &cbNop, &ts, false);
    t.setTimeout(300);
    t.enable();                   // timeout counts from now

    ts.execute();
    g_ms += 100;
    ts.execute();
    EXPECT_EQ(201UL, ts.getNextRun());   // timeout fires once elapsed > 300 ms

    g_ms += 201;
    ts.execute();
    EXPECT_TRUE(t.isTimedOut());
}

TEST(DefectFixes, D8_TicklessPicksEarliestTask) {
    Scheduler ts;
    Task slow(1000, TASK_FOREVER, &cbNop, &ts, true);
    Task fast(250, TASK_FOREVER, &cbNop, &ts, true);

    ts.execute();
    g_ms += 50;
    ts.execute();
    EXPECT_EQ(200UL, ts.getNextRun());
}

TEST(DefectFixes, D8_TicklessSurvivesCounterRollover) {
    g_ms = (unsigned long)(-1) - 50;     // 51 time units before the counter wraps
    Scheduler ts;
    Task t(100, TASK_FOREVER, &cbNop, &ts, true);

    ts.execute();                        // next run is due after the wrap
    g_ms += 10;
    ts.execute();
    EXPECT_EQ(90UL, ts.getNextRun());
}

// ---------------------------------------------------------------------------
// D9: idle sleep with priority layers
// ---------------------------------------------------------------------------
TEST(DefectFixes, D9_BaseSleepsWhenHigherLayerConstructedLast) {
    resetCounters();
    Scheduler r, hpr;                    // hpr is constructed last and claims the sleep role
    r.setHighPriorityScheduler(&hpr);
    iSleepMethod = &countSleep;          // count sleeps, keep the role where it is
    Task tb(1000, TASK_FOREVER, &cbNop, &r, false);

    for (int i = 0; i < 5; i++) r.execute();
    EXPECT_EQ(5, s_sleeps);
    EXPECT_EQ(&r, iSleepScheduler);
}

TEST(DefectFixes, D9_BaseSleepsWithThreeLayers) {
    resetCounters();
    Scheduler r, hpr, cpr;
    r.setHighPriorityScheduler(&hpr);
    hpr.setHighPriorityScheduler(&cpr);
    iSleepMethod = &countSleep;
    Task tb(1000, TASK_FOREVER, &cbNop, &r, false);

    for (int i = 0; i < 5; i++) r.execute();
    EXPECT_EQ(5, s_sleeps);
    EXPECT_EQ(&r, iSleepScheduler);
}

TEST(DefectFixes, D9_IndependentSchedulersUnchanged) {
    resetCounters();
    Scheduler a, b;                      // not layered: only the last constructed sleeps
    iSleepMethod = &countSleep;

    for (int i = 0; i < 3; i++) { a.execute(); b.execute(); }
    EXPECT_EQ(3, s_sleeps);
    EXPECT_EQ(&b, iSleepScheduler);
}

TEST(DefectFixes, D9_NoSleepAfterProductivePass) {
    resetCounters();
    Scheduler r, hpr;
    r.setHighPriorityScheduler(&hpr);
    iSleepMethod = &countSleep;
    Task th(0, TASK_FOREVER, &cbCount, &hpr, true);   // higher layer works every pass
    Task tb(1000, TASK_FOREVER, &cbNop, &r, false);

    for (int i = 0; i < 3; i++) r.execute();
    EXPECT_EQ(3, s_runs);
    EXPECT_EQ(0, s_sleeps);
}

// ---------------------------------------------------------------------------
// D20: deleting a task inside its own OnDisable
// ---------------------------------------------------------------------------
TEST(DefectFixes, D20_DeleteInOnDisableAfterLastIteration) {
    resetCounters();
    Scheduler ts;
    Task* t = new Task(0, 1, &cbCount, &ts, true, NULL, &odDeleteVictim);
    Task after(0, TASK_FOREVER, &cbCount2, &ts, true);
    s_victim = t;

    ts.execute();                        // t runs its only iteration
    ts.execute();                        // t is disabled, OnDisable deletes it
    EXPECT_EQ(1, s_runs);
    EXPECT_EQ(1, s_ondisable);
    EXPECT_EQ(nullptr, s_victim);
    EXPECT_EQ(2, s_runs2);               // the next task still ran in both passes
    EXPECT_EQ(1UL, ts.getChainLength());
}

TEST(DefectFixes, D20_DeleteInOnDisableFromDirectDisable) {
    resetCounters();
    Scheduler ts;
    Task* t = new Task(1000, TASK_FOREVER, &cbNop, &ts, true, NULL, &odDeleteVictim);
    s_victim = t;

    EXPECT_TRUE(t->disable());           // previous state; the task is gone afterwards
    EXPECT_EQ(1, s_ondisable);
    EXPECT_EQ(0UL, ts.getChainLength());
    EXPECT_EQ(nullptr, ts.getCurrentTask());
}

TEST(DefectFixes, D20_DeleteInOnDisableFromDisableAll) {
    resetCounters();
    Scheduler ts;
    Task* t = new Task(1000, TASK_FOREVER, &cbNop, &ts, true, NULL, &odDeleteVictim);
    Task other(1000, TASK_FOREVER, &cbNop, &ts, true);
    s_victim = t;

    ts.disableAll();
    EXPECT_EQ(1, s_ondisable);
    EXPECT_FALSE(other.isEnabled());
    EXPECT_EQ(1UL, ts.getChainLength());
}

TEST(DefectFixes, D20_DeleteInOnDisableOnTimeout) {
    resetCounters();
    Scheduler ts;
    Task* t = new Task(1000, TASK_FOREVER, &cbNop, &ts, false, NULL, &odDeleteVictim);
    t->setTimeout(50);
    t->enable();
    s_victim = t;

    ts.execute();
    g_ms += 51;
    ts.execute();                        // times out, OnDisable deletes it
    EXPECT_EQ(1, s_ondisable);
    EXPECT_EQ(0UL, ts.getChainLength());
}

TEST(DefectFixes, D20_SelfDestructTaskDeletedInOnDisableIsNotDeletedTwice) {
    resetCounters();
    Scheduler ts;
    Task* t = new Task(0, 1, &cbCount, &ts, true, NULL, &odDeleteVictim, true);
    s_victim = t;

    ts.execute();
    ts.execute();
    ts.execute();
    EXPECT_EQ(1, s_runs);
    EXPECT_EQ(1, s_ondisable);
    EXPECT_EQ(0UL, ts.getChainLength());
}

TEST(DefectFixes, D20_NestedOnDisableDeletesOuterTask) {
    resetCounters();
    Scheduler ts;
    // a's OnDisable disables b; b's OnDisable deletes a while a's disable() is still running
    Task* a = new Task(1000, TASK_FOREVER, &cbNop, &ts, true, NULL, &odDisableNested);
    Task b(1000, TASK_FOREVER, &cbNop, &ts, true, NULL, &odDeleteVictim);
    s_victim = a;
    s_nested = &b;

    a->disable();
    EXPECT_EQ(2, s_ondisable);
    EXPECT_FALSE(b.isEnabled());
    EXPECT_EQ(1UL, ts.getChainLength());
}

// ---------------------------------------------------------------------------
// D27: setIntervalNodelay() KEEP and RESET
// Scenario: interval 1000, last run at T, interval changed to 500 at T+300.
// Documented results: KEEP -> T+1000, RECALC -> T+500, RESET -> T+800.
// ---------------------------------------------------------------------------
static Task* s_self = nullptr;
static void cbKeepSelf() {
    s_runs++;
    if (s_runs == 1) s_self->setIntervalNodelay(500, TASK_INTERVAL_KEEP);
}

TEST(DefectFixes, D27_KeepKeepsScheduledRun) {
    resetCounters();
    Scheduler ts;
    Task t(1000, TASK_FOREVER, &cbCount, &ts, true);
    ts.execute();                                         // run at T
    ASSERT_EQ(1, s_runs);

    g_ms += 300;
    t.setIntervalNodelay(500, TASK_INTERVAL_KEEP);
    EXPECT_EQ(500UL, t.getInterval());
    EXPECT_EQ(700, ts.timeUntilNextIteration(t));         // still T+1000

    g_ms += 699; ts.execute();
    EXPECT_EQ(1, s_runs);
    g_ms += 1;   ts.execute();
    EXPECT_EQ(2, s_runs);                                 // runs at T+1000
    g_ms += 499; ts.execute();
    EXPECT_EQ(2, s_runs);
    g_ms += 1;   ts.execute();
    EXPECT_EQ(3, s_runs);                                 // then every 500
}

TEST(DefectFixes, D27_KeepKeepsPendingDelay) {
    Scheduler ts;
    Task t(1000, TASK_FOREVER, &cbNop, &ts, true);
    ts.execute();
    t.delay(2000);                                        // next run at T+2000

    g_ms += 300;
    t.setIntervalNodelay(500, TASK_INTERVAL_KEEP);
    EXPECT_EQ(1700, ts.timeUntilNextIteration(t));        // still T+2000
}

TEST(DefectFixes, D27_KeepFromOwnCallback) {
    resetCounters();
    Scheduler ts;
    Task t(1000, TASK_FOREVER, &cbKeepSelf, &ts, true);
    s_self = &t;

    ts.execute();                                         // run 1 at T switches to 500
    ASSERT_EQ(1, s_runs);
    g_ms += 999; ts.execute();
    EXPECT_EQ(1, s_runs);
    g_ms += 1;   ts.execute();
    EXPECT_EQ(2, s_runs);                                 // run 2 keeps the old slot, T+1000
    g_ms += 500; ts.execute();
    EXPECT_EQ(3, s_runs);                                 // run 3 at T+1500
}

TEST(DefectFixes, D27_RecalcFromLastRun) {
    Scheduler ts;
    Task t(1000, TASK_FOREVER, &cbNop, &ts, true);
    ts.execute();

    g_ms += 300;
    t.setIntervalNodelay(500, TASK_INTERVAL_RECALC);
    EXPECT_EQ(200, ts.timeUntilNextIteration(t));         // T+500
}

TEST(DefectFixes, D27_ResetCountsFromNow) {
    resetCounters();
    Scheduler ts;
    Task t(1000, TASK_FOREVER, &cbCount, &ts, true);
    ts.execute();
    ASSERT_EQ(1, s_runs);

    g_ms += 300;
    t.setIntervalNodelay(500, TASK_INTERVAL_RESET);
    EXPECT_EQ(500UL, t.getInterval());
    EXPECT_EQ(500, ts.timeUntilNextIteration(t));         // T+800

    g_ms += 499; ts.execute();
    EXPECT_EQ(1, s_runs);
    g_ms += 1;   ts.execute();
    EXPECT_EQ(2, s_runs);
}

TEST(DefectFixes, D27_ResetDiscardsPendingDelay) {
    Scheduler ts;
    Task t(1000, TASK_FOREVER, &cbNop, &ts, true);
    ts.execute();
    t.delay(2000);

    g_ms += 300;
    t.setIntervalNodelay(500, TASK_INTERVAL_RESET);
    EXPECT_EQ(500, ts.timeUntilNextIteration(t));         // now + 500
}

// ---------------------------------------------------------------------------
// D21: Scheduler destructor and copy protection
// ---------------------------------------------------------------------------
TEST(DefectFixes, D21_SchedulerDestroyedBeforeItsTasks) {
    Scheduler* ts = new Scheduler();
    Task* t = new Task(100, TASK_FOREVER, &cbNop, ts, true);
    Task* u = new Task(100, TASK_FOREVER, &cbNop, ts, true);

    delete ts;                           // detaches both tasks
    EXPECT_FALSE(t->enable());           // no scheduler any more

    Scheduler other;                     // a detached task can join another scheduler
    other.addTask(*t);
    EXPECT_EQ(1UL, other.getChainLength());

    delete t;                            // must not touch the destroyed scheduler
    delete u;
    EXPECT_EQ(0UL, other.getChainLength());
}

TEST(DefectFixes, D21_SchedulerIsNotCopyable) {
    EXPECT_FALSE(std::is_copy_constructible<Scheduler>::value);
    EXPECT_FALSE(std::is_copy_assignable<Scheduler>::value);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
