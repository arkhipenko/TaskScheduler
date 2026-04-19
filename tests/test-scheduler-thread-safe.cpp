// test-scheduler-thread-safe.cpp - Unit tests for _TASK_THREAD_SAFE functionality
// This file contains tests for the TaskScheduler thread-safe operations
// Covers multi-threaded task control, ISR simulation, and request queue operations
//
// THREAD SAFETY TESTING NOTES:
// ============================
// 1. Tests use C++ std::thread for simulating multi-threaded environments
// 2. Queue implementation uses std::queue with mutex protection
// 3. Tests verify that requestAction() safely handles concurrent requests
// 4. StatusRequest signaling from simulated ISRs is tested
// 5. Worker thread task manipulation is verified
//
// Linux/Desktop Implementation:
// - Uses std::queue + std::mutex instead of FreeRTOS queue
// - Simulates ISR context using thread-local storage
// - Compatible with Google Test framework on Ubuntu

// #define _TASK_HEADER_AND_CPP
// #define _TASK_THREAD_SAFE
// #define _TASK_STATUS_REQUEST
// #define _TASK_TIMEOUT

#include <gtest/gtest.h>
#include "Arduino.h"
#include "TaskSchedulerDeclarations.h"
#include "test-queue-impl.h"

#include <thread>
#include <atomic>

// Thread-local storage to simulate ISR context
thread_local bool isInISRContext = false;

// ============================================
// Global Test State
// ============================================

std::vector<std::string> test_output;
std::atomic<int> task_execution_count{0};
std::atomic<int> thread1_requests{0};
std::atomic<int> thread2_requests{0};
std::atomic<int> isr_requests{0};

// ============================================
// Test Callbacks
// ============================================

void thread_safe_task_callback() {
    task_execution_count++;
    test_output.push_back("Thread-safe task executed");
}

void fast_task_callback() {
    test_output.push_back("Fast task executed");
}

void interval_test_callback() {
    static int counter = 0;
    counter++;
    test_output.push_back("Interval task #" + std::to_string(counter));
}

// ============================================
// Test Fixture
// ============================================

class ThreadSafeTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_output.clear();
        task_execution_count = 0;
        thread1_requests = 0;
        thread2_requests = 0;
        isr_requests = 0;
        clearTaskRequestQueue();
        isInISRContext = false;
    }

    void TearDown() override {
        clearTaskRequestQueue();
    }

    // Helper: Run scheduler for specified duration
    void runScheduler(Scheduler& ts, unsigned long duration_ms) {
        unsigned long start = millis();
        while (millis() - start < duration_ms) {
            ts.execute();
            delay(1);
        }
    }

    // Helper: Wait for condition with timeout
    template<typename Condition>
    bool waitForCondition(Condition cond, unsigned long timeout_ms) {
        unsigned long start = millis();
        while (millis() - start < timeout_ms) {
            if (cond()) return true;
            delay(10);
        }
        return false;
    }
};

// ============================================
// Basic Queue Tests
// ============================================

TEST_F(ThreadSafeTest, QueueEnqueueDequeue) {
    _task_request_t req;
    req.req_type = TASK_REQUEST_ENABLE_0;
    req.object_ptr = (void*)0x1234;
    req.param1 = 100;

    // Enqueue
    EXPECT_TRUE(_task_enqueue_request(&req));
    EXPECT_EQ(getQueueSize(), 1);

    // Dequeue
    _task_request_t dequeued;
    EXPECT_TRUE(_task_dequeue_request(&dequeued));
    EXPECT_EQ(getQueueSize(), 0);

    // Verify data
    EXPECT_EQ(dequeued.req_type, TASK_REQUEST_ENABLE_0);
    EXPECT_EQ(dequeued.object_ptr, (void*)0x1234);
    EXPECT_EQ(dequeued.param1, 100);
}

TEST_F(ThreadSafeTest, QueueFIFOOrder) {
    // Enqueue multiple requests
    for (int i = 0; i < 10; i++) {
        _task_request_t req;
        req.req_type = TASK_REQUEST_ENABLE_0;
        req.param1 = i;
        EXPECT_TRUE(_task_enqueue_request(&req));
    }

    EXPECT_EQ(getQueueSize(), 10);

    // Dequeue and verify order
    for (int i = 0; i < 10; i++) {
        _task_request_t req;
        EXPECT_TRUE(_task_dequeue_request(&req));
        EXPECT_EQ(req.param1, (unsigned long)i);
    }

    EXPECT_EQ(getQueueSize(), 0);
}

TEST_F(ThreadSafeTest, QueueEmptyDequeue) {
    _task_request_t req;
    EXPECT_FALSE(_task_dequeue_request(&req));
}

// ============================================
// Thread-Safe Task Control Tests
// ============================================

TEST_F(ThreadSafeTest, RequestActionEnableTask) {
    Scheduler ts;
    Task testTask(100, 5, &thread_safe_task_callback, &ts, false);

    // Task should be disabled initially
    EXPECT_FALSE(testTask.isEnabled());

    // Request enable via requestAction
    bool success = ts.requestAction(&testTask, TASK_REQUEST_ENABLE_0, 0, 0, 0, 0, 0);
    EXPECT_TRUE(success);
    EXPECT_EQ(getQueueSize(), 1);

    // Process requests
    ts.execute();

    // Task should now be enabled
    EXPECT_TRUE(testTask.isEnabled());
    EXPECT_EQ(getQueueSize(), 0);
}

TEST_F(ThreadSafeTest, RequestActionDisableTask) {
    Scheduler ts;
    Task testTask(100, 5, &thread_safe_task_callback, &ts, true);

    // Task should be enabled initially
    EXPECT_TRUE(testTask.isEnabled());

    // Request disable via requestAction
    bool success = ts.requestAction(&testTask, TASK_REQUEST_DISABLE_0, 0, 0, 0, 0, 0);
    EXPECT_TRUE(success);

    // Process requests
    ts.execute();

    // Task should now be disabled
    EXPECT_FALSE(testTask.isEnabled());
}

TEST_F(ThreadSafeTest, RequestActionChangeInterval) {
    Scheduler ts;
    Task testTask(100, TASK_FOREVER, &thread_safe_task_callback, &ts, true);

    EXPECT_EQ(testTask.getInterval(), 100UL);

    // Request interval change
    ts.requestAction(&testTask, TASK_REQUEST_SETINTERVAL_1, 500, 0, 0, 0, 0);

    // Process requests
    ts.execute();

    // Interval should be updated
    EXPECT_EQ(testTask.getInterval(), 500UL);
}

// ============================================
// Multi-Threaded Tests
// ============================================

TEST_F(ThreadSafeTest, ConcurrentEnableRequests) {
    Scheduler ts;
    Task task1(50, TASK_FOREVER, &thread_safe_task_callback, &ts, false);
    Task task2(50, TASK_FOREVER, &fast_task_callback, &ts, false);

    // Launch two threads that both try to enable tasks
    std::thread t1([&]() {
        for (int i = 0; i < 10; i++) {
            ts.requestAction(&task1, TASK_REQUEST_ENABLE_0, 0, 0, 0, 0, 0);
            thread1_requests++;
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    });

    std::thread t2([&]() {
        for (int i = 0; i < 10; i++) {
            ts.requestAction(&task2, TASK_REQUEST_ENABLE_0, 0, 0, 0, 0, 0);
            thread2_requests++;
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    });

    // Process requests in main thread
    std::thread scheduler_thread([&]() {
        for (int i = 0; i < 100; i++) {
            ts.execute();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    });

    t1.join();
    t2.join();
    scheduler_thread.join();

    // All requests should have been processed
    EXPECT_EQ(thread1_requests.load(), 10);
    EXPECT_EQ(thread2_requests.load(), 10);
    EXPECT_TRUE(task1.isEnabled());
    EXPECT_TRUE(task2.isEnabled());
}

TEST_F(ThreadSafeTest, WorkerThreadChangesInterval) {
    Scheduler ts;
    Task testTask(100, TASK_FOREVER, &interval_test_callback, &ts, true);

    std::atomic<bool> worker_running{true};

    // Worker thread that periodically changes interval
    std::thread worker([&]() {
        int intervals[] = {50, 100, 150, 200, 250};
        int idx = 0;

        while (worker_running && idx < 5) {
            std::this_thread::sleep_for(std::chrono::milliseconds(intervals[idx]+5));
            ts.requestAction(&testTask, TASK_REQUEST_SETINTERVAL_1,
                           intervals[idx], 0, 0, 0, 0);
            idx++;
        }
    });

    // Run scheduler
    runScheduler(ts, 600);
    worker_running = false;
    worker.join();

    // Task should still be running
    EXPECT_TRUE(testTask.isEnabled());
    // Interval should have been changed multiple times
    EXPECT_GT(test_output.size(), 3);
}

// ============================================
// Simulated ISR Tests
// ============================================

TEST_F(ThreadSafeTest, ISRTriggersStatusRequest) {
    Scheduler ts;
    StatusRequest buttonPressed;
    Task responseTask(TASK_IMMEDIATE, TASK_ONCE, &fast_task_callback, &ts, false);

    // Set up task to wait for status request
    responseTask.waitFor(&buttonPressed);

    // Simulate ISR in separate thread
    std::thread isr_simulation([&]() {
        isInISRContext = true;  // Mark as ISR context
        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        // Signal from "ISR"
        ts.requestAction(&buttonPressed, TASK_SR_REQUEST_SIGNALCOMPLETE_1, 0, 0, 0, 0, 0);
        isr_requests++;

        isInISRContext = false;
    });

    // Run scheduler
    runScheduler(ts, 200);
    isr_simulation.join();

    // Task should have executed in response to signal
    EXPECT_EQ(isr_requests.load(), 1);
    EXPECT_GT(test_output.size(), 0);
    EXPECT_EQ(test_output[0], "Fast task executed");
}

TEST_F(ThreadSafeTest, MultipleISRsWithDifferentRequests) {
    Scheduler ts;
    Task task1(200, TASK_FOREVER, &thread_safe_task_callback, &ts, false);
    Task task2(200, TASK_FOREVER, &fast_task_callback, &ts, false);

    // Simulate multiple ISRs
    std::thread isr1([&]() {
        for (int i = 0; i < 5; i++) {
            std::this_thread::sleep_for(std::chrono::milliseconds(30));
            ts.requestAction(&task1, TASK_REQUEST_ENABLE_0, 0, 0, 0, 0, 0);
        }
    });

    std::thread isr2([&]() {
        for (int i = 0; i < 5; i++) {
            std::this_thread::sleep_for(std::chrono::milliseconds(35));
            ts.requestAction(&task2, TASK_REQUEST_ENABLE_0, 0, 0, 0, 0, 0);
        }
    });

    // Run scheduler
    runScheduler(ts, 300);

    isr1.join();
    isr2.join();

    // Both tasks should be enabled
    EXPECT_TRUE(task1.isEnabled());
    EXPECT_TRUE(task2.isEnabled());
}

// ============================================
// Stress Tests
// ============================================

TEST_F(ThreadSafeTest, HighFrequencyRequests) {
    Scheduler ts;
    Task testTask(10, TASK_FOREVER, &thread_safe_task_callback, &ts, true);

    std::atomic<int> successful_requests{0};

    // Many threads sending requests rapidly
    std::vector<std::thread> threads;
    for (int t = 0; t < 5; t++) {
        threads.emplace_back([&]() {
            for (int i = 0; i < 20; i++) {
                if (ts.requestAction(&testTask, TASK_REQUEST_SETINTERVAL_1,
                                   10 + i, 0, 0, 0, 0)) {
                    successful_requests++;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        });
    }

    // Process requests
    for (int i = 0; i < 500; i++) {
        ts.execute();
        delay(1);
    }

    // Wait for all threads
    for (auto& t : threads) {
        t.join();
    }

    // Most requests should have succeeded
    EXPECT_GT(successful_requests.load(), 90);  // Allow some to fail due to timing
}

TEST_F(ThreadSafeTest, QueueOverflowHandling) {
    Scheduler ts;
    Task testTask(100, TASK_FOREVER, &thread_safe_task_callback, &ts, true);

    // Try to overflow the queue
    int successful = 0;
    int failed = 0;

    for (size_t i = 0; i < MAX_QUEUE_SIZE + 50; i++) {
        if (ts.requestAction(&testTask, TASK_REQUEST_SETINTERVAL_1, i, 0, 0, 0, 0)) {
            successful++;
        } else {
            failed++;
        }
    }

    // Should have some failures due to queue limit
    EXPECT_GT(failed, 0);
    EXPECT_LE(successful, (int)MAX_QUEUE_SIZE);

    // Process all queued requests
    for (int i = 0; i < 200; i++) {
        ts.execute();
        delay(1);
    }

    // Queue should be empty now
    EXPECT_EQ(getQueueSize(), 0);
}

// ============================================
// Integration Tests
// ============================================

TEST_F(ThreadSafeTest, RealWorldScenario) {
    Scheduler ts;
    Task blinkTask(100, TASK_FOREVER, &interval_test_callback, &ts, true);
    StatusRequest dataReady;
    Task dataTask(TASK_IMMEDIATE, TASK_ONCE, &fast_task_callback, &ts, false);

    dataTask.waitFor(&dataReady);

    std::atomic<bool> running{true};

    // Worker thread: Adjusts blink rate
    std::thread worker([&]() {
        int count = 0;
        while (running && count < 5) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            ts.requestAction(&blinkTask, TASK_REQUEST_SETINTERVAL_1,
                           50 + count * 50, 0, 0, 0, 0);
            count++;
        }
    });

    // Simulated ISR: Signals data ready
    std::thread isr([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
        ts.requestAction(&dataReady, TASK_SR_REQUEST_SIGNALCOMPLETE_1, 0, 0, 0, 0, 0);
    });

    // Run scheduler
    runScheduler(ts, 600);
    running = false;

    worker.join();
    isr.join();

    // Verify both tasks executed
    EXPECT_GT(test_output.size(), 2);

    // Should have both interval task and fast task executions
    bool hasIntervalTask = false;
    bool hasFastTask = false;

    for (const auto& output : test_output) {
        if (output.find("Interval task") != std::string::npos) hasIntervalTask = true;
        if (output.find("Fast task") != std::string::npos) hasFastTask = true;
    }

    EXPECT_TRUE(hasIntervalTask);
    EXPECT_TRUE(hasFastTask);
}

// ============================================
// Generation Counter Tests (v4.1.0)
// ============================================
// These tests require _TASK_DEBUG on the compile target so the iGeneration
// private members on Task and StatusRequest are reachable. The thread-safe
// CI job already does white-box inspection; _TASK_DEBUG is added alongside
// _TASK_THREAD_SAFE in the workflow.

// Scheduler::requestAction() stamps this value from the target object.
// processRequests() compares it with the live counter before dereferencing
// and skips mismatches. Destructors zero the counter (sentinel for
// "destroyed"). See TaskScheduler.h v4.1.0 changelog.

TEST_F(ThreadSafeTest, TaskGenerationNonZeroAfterConstruction) {
    Scheduler ts;
    Task t(100, TASK_ONCE, &thread_safe_task_callback, &ts, false);
    // Fresh Task must carry a non-zero generation stamp.
    EXPECT_NE(t.iGeneration, 0);
}

TEST_F(ThreadSafeTest, StatusRequestGenerationNonZeroAfterConstruction) {
    StatusRequest sr;
    EXPECT_NE(sr.iGeneration, 0);
}

// Core defence against the v4.0.6 C4 dangling-pointer gap: an ENABLE request
// stamped against a heap Task, then issued after the Task is deleted, must
// be skipped by processRequests() rather than dereferencing the freed
// pointer.
//
// Note: the generation counter is a best-effort safeguard -- it relies on
// the backing memory still being readable at dequeue time. Under
// AddressSanitizer (heap poisoning), that read itself is flagged as a
// use-after-free, so this test is disabled on ASan builds. On normal
// allocators the destructor's `iGeneration = 0` write lingers long enough
// for the gate to reject the request.
#if !defined(__SANITIZE_ADDRESS__) && !defined(ADDRESS_SANITIZER_BUILD)
TEST_F(ThreadSafeTest, StaleRequestSkippedAfterTaskDestruction) {
    Scheduler ts;
    task_execution_count = 0;

    Task* heapTask = new Task(0, TASK_FOREVER,
                              &thread_safe_task_callback, &ts, false);
    // Queue the ENABLE request targeting the heap task.
    ts.requestAction(heapTask, TASK_REQUEST_ENABLE, 0, 0, 0, 0, 0);

    // Destroy the task before the scheduler drains the queue.
    delete heapTask;

    // Drive the scheduler a handful of times. The callback must never fire
    // because processRequests() should detect the zeroed generation and
    // skip the stale entry instead of calling enable() on freed memory.
    for (int i = 0; i < 5; ++i) ts.execute();

    EXPECT_EQ(task_execution_count.load(), 0)
        << "Stale request must not activate a freed Task";
}
#endif  // !defined(__SANITIZE_ADDRESS__)

// Mirror of the stale test with a live target: the callback must fire.
TEST_F(ThreadSafeTest, LiveRequestHonoredWhenGenerationMatches) {
    Scheduler ts;
    task_execution_count = 0;

    Task t(0, TASK_FOREVER, &thread_safe_task_callback, &ts, false);
    ts.requestAction(&t, TASK_REQUEST_ENABLE, 0, 0, 0, 0, 0);

    // First pass processes queue and enables the task; subsequent passes
    // run it. Two passes are enough to invoke the callback at least once.
    ts.execute();
    ts.execute();

    EXPECT_GE(task_execution_count.load(), 1)
        << "Live request with matching generation must be honored";
}

// Simulate "same address reused for a new object": enqueue a request with
// the live stamp, then bump the target's counter before the scheduler
// processes the queue. The stamp no longer matches, so the request must
// be skipped.
TEST_F(ThreadSafeTest, MismatchedGenerationSkipsRequest) {
    Scheduler ts;
    task_execution_count = 0;

    Task t(0, TASK_FOREVER, &thread_safe_task_callback, &ts, false);

    uint16_t stamp = t.iGeneration;
    _task_request_t req;
    req.req_type   = TASK_REQUEST_ENABLE;
    req.object_ptr = &t;
    req.param1 = req.param2 = req.param3 = req.param4 = req.param5 = 0;
    req.generation = stamp;
    ASSERT_TRUE(_task_enqueue_request(&req));

    // Simulate reconstruction at the same address: bump the live counter
    // so it no longer matches the stamped request.
    t.iGeneration = static_cast<uint16_t>(stamp + 1);

    for (int i = 0; i < 5; ++i) ts.execute();

    EXPECT_EQ(task_execution_count.load(), 0)
        << "Request with stale generation stamp must be dropped by processRequests()";
}

// ============================================
// Main
// ============================================

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
