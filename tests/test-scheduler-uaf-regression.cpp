// test-scheduler-uaf-regression.cpp
//
// Regression tests for the use-after-free in Scheduler::execute() that occurs
// when a callback destroys the task that nextTask pre-captured.
//
// Root cause
// ----------
// execute() pre-captures `nextTask = iCurrent->iNext` before invoking the
// callback.  If that callback destroys the heap-allocated object that nextTask
// points to (e.g. a shared_ptr<Connection> going to zero ref-count, which calls
// ~Task() on value-member tasks, which in turn calls deleteTask()), `nextTask`
// becomes a dangling pointer.  On targets with heap poisoning (ESP-IDF) this
// manifests as a LoadProhibited crash at EXCVADDR 0xbaad567c.
//
// Fix
// ---
// Re-read `nextTask = iCurrent->iNext` immediately after the callback returns.
// deleteTask() relinks the list (updates iCurrent->iNext to skip the removed
// node) before freeing memory, so the re-read is safe and gives the correct
// next task.
//
// Test scenario
// -------------
// Scheduler chain: A -> B -> C  (construction/registration order)
// A's callback calls `delete taskB`, which triggers Task::~Task() ->
//   deleteTask(), relinks A->iNext = C, then frees B's memory.
// Expected: C runs in the same execute() pass (c_count == 1).
// Without the fix: nextTask still points to freed B; on desktop the freed
//   block is typically zeroed, so iStatus.enabled == 0 and iNext == NULL,
//   causing the loop to exit before reaching C (c_count == 0 => test FAILS).

#include <gtest/gtest.h>
#include "Arduino.h"
#include "TaskScheduler.h"

std::vector<std::string> test_output;

// ---------------------------------------------------------------------------
// Shared callback state (plain function pointers — TaskScheduler does not
// support std::function without _TASK_STD_FUNCTION on non-ESP platforms)
// ---------------------------------------------------------------------------

static int   s_a_count = 0;
static int   s_b_count = 0;
static int   s_c_count = 0;
static int   s_d_count = 0;
static Task* s_taskB   = nullptr;

static void cbA() {
    s_a_count++;
    // Simulate the painlessMesh BufferedConnection teardown: destroy the
    // heap-allocated task that is next in the scheduler chain.
    // Task::~Task() calls deleteTask(), which relinks A->iNext to C before
    // the memory is freed.
    delete s_taskB;
    s_taskB = nullptr;
}
static void cbB() { s_b_count++; }
static void cbC() { s_c_count++; }
static void cbD() { s_d_count++; }

// ---------------------------------------------------------------------------

class UafRegressionTest : public ::testing::Test {
protected:
    void SetUp() override {
        s_a_count = s_b_count = s_c_count = s_d_count = 0;
        s_taskB = nullptr;
        test_output.clear();
    }
    void TearDown() override {
        // Defensive cleanup in case the test failed before cbA could run.
        delete s_taskB;
        s_taskB = nullptr;
        test_output.clear();
    }
};

// ---------------------------------------------------------------------------
// Core regression: the task immediately after the deleted task must still run.
// ---------------------------------------------------------------------------
TEST_F(UafRegressionTest, TaskAfterDeletedTaskStillRuns) {
    Scheduler ts;

    // Register in A -> B -> C order — that is their order in the linked list.
    Task taskA(TASK_IMMEDIATE, TASK_ONCE, cbA, &ts, true);
    s_taskB = new Task(TASK_IMMEDIATE, TASK_ONCE, cbB, &ts, true);
    Task taskC(TASK_IMMEDIATE, TASK_ONCE, cbC, &ts, true);

    ts.execute();

    EXPECT_EQ(s_a_count, 1) << "Task A should have run";
    EXPECT_EQ(s_b_count, 0) << "Task B was deleted by A before it could run";
    EXPECT_EQ(s_c_count, 1) << "Task C must run despite B being deleted mid-iteration";
}

// ---------------------------------------------------------------------------
// All tasks beyond the deleted node must also run in the same pass.
// ---------------------------------------------------------------------------
TEST_F(UafRegressionTest, AllTasksAfterDeletedTaskStillRun) {
    Scheduler ts;

    // Chain: A -> B -> C -> D
    Task taskA(TASK_IMMEDIATE, TASK_ONCE, cbA, &ts, true);
    s_taskB = new Task(TASK_IMMEDIATE, TASK_ONCE, cbB, &ts, true);
    Task taskC(TASK_IMMEDIATE, TASK_ONCE, cbC, &ts, true);
    Task taskD(TASK_IMMEDIATE, TASK_ONCE, cbD, &ts, true);

    ts.execute();

    EXPECT_EQ(s_a_count, 1) << "A ran";
    EXPECT_EQ(s_b_count, 0) << "B was deleted";
    EXPECT_EQ(s_c_count, 1) << "C must run";
    EXPECT_EQ(s_d_count, 1) << "D must run";
}

// ---------------------------------------------------------------------------

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
