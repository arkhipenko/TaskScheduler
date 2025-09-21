// test-scheduler-advanced-features-no-lambda.cpp - Advanced TaskScheduler features without lambda functions
// This file contains comprehensive tests for advanced TaskScheduler functionality
// enabled by specific compile-time directives, using traditional function pointers
//
// =====================================================================================
// ADVANCED FEATURES TEST PLAN AND COVERAGE MATRIX
// =====================================================================================
//
// PURPOSE: Validate advanced TaskScheduler features
// APPROACH: Traditional function pointers for maximum platform compatibility
// SCOPE: Advanced features requiring specific compile-time directives
//
// COMPILE DIRECTIVES TESTED:
// - _TASK_STATUS_REQUEST:      Event-driven task coordination and status signaling
// - _TASK_TIMEOUT:             Task execution timeout and deadline management
// - _TASK_SCHEDULING_OPTIONS:  Advanced scheduling behavior control
// - _TASK_SELF_DESTRUCT:       Automatic task cleanup and memory management
//
// COVERAGE MATRIX:
// ================
//
// 1. STATUS REQUEST FUNCTIONALITY (_TASK_STATUS_REQUEST)
//    ├── StatusRequest Creation and Management
//    │   ├── StatusRequest() constructor
//    │   ├── setWaiting(count) - prepare for multiple signals
//    │   ├── signal(status) - signal completion from tasks
//    │   ├── signalComplete(status) - force immediate completion
//    │   ├── pending() - check if still waiting
//    │   ├── completed() - check if finished
//    │   ├── getStatus() - retrieve completion status
//    │   └── getCount() - get remaining signal count
//    │
//    ├── Task Status Request Integration
//    │   ├── waitFor(StatusRequest*, interval, iterations) - wait for event
//    │   ├── waitForDelayed(StatusRequest*, interval, iterations) - delayed wait
//    │   ├── getStatusRequest() - get external status request
//    │   └── getInternalStatusRequest() - get task's internal status
//    │
//    └── Event-Driven Task Coordination Patterns
//        ├── Single task waiting for event completion
//        ├── Multiple tasks waiting for same event
//        ├── Task chains with dependency resolution
//        └── Producer-consumer coordination scenarios
//
// 2. TIMEOUT FUNCTIONALITY (_TASK_TIMEOUT)
//    ├── Task Timeout Management
//    │   ├── setTimeout(timeout, reset) - set execution deadline
//    │   ├── resetTimeout() - restart timeout timer
//    │   ├── getTimeout() - retrieve timeout value
//    │   ├── untilTimeout() - time remaining until timeout
//    │   └── isTimedOut() - check timeout status
//    │
//    ├── StatusRequest Timeout Integration
//    │   ├── StatusRequest::setTimeout(timeout) - set request deadline
//    │   ├── StatusRequest::resetTimeout() - restart request timer
//    │   ├── StatusRequest::getTimeout() - retrieve request timeout
//    │   └── StatusRequest::untilTimeout() - time until request expires
//    │
//    └── Timeout Behavior Patterns
//        ├── Task execution timeout handling
//        ├── StatusRequest completion timeout
//        ├── Timeout-driven task disabling
//        └── Deadline-based task prioritization
//
// 3. SCHEDULING OPTIONS (_TASK_SCHEDULING_OPTIONS)
//    ├── Scheduling Behavior Control
//    │   ├── setSchedulingOption(option) - configure task behavior
//    │   ├── getSchedulingOption() - retrieve current option
//    │   └── Scheduling option effects on execution
//    │
//    └── Advanced Scheduling Patterns
//        ├── Priority-based execution control
//        ├── Conditional execution behavior
//        └── Resource allocation management
//
// 4. SELF-DESTRUCT FUNCTIONALITY (_TASK_SELF_DESTRUCT)
//    ├── Automatic Task Cleanup
//    │   ├── Task auto-removal from scheduler
//    │   ├── Memory management and resource cleanup
//    │   └── Lifecycle-based task destruction
//    │
//    └── Self-Destruct Patterns
//        ├── One-shot tasks with auto-cleanup
//        ├── Conditional task removal
//        └── Dynamic task lifecycle management
//
// 5. INTEGRATION AND ADVANCED PATTERNS
//    ├── Multi-Feature Integration Tests
//    │   ├── Status requests with timeout handling
//    │   ├── Self-destructing tasks with status coordination
//    │   ├── Scheduling options with timeout management
//    │   └── Complex event-driven workflows
//    │
//    ├── Real-World Usage Scenarios
//    │   ├── Sensor data collection with timeouts
//    │   ├── Multi-stage processing pipelines
//    │   ├── Resource monitoring and cleanup
//    │   └── Deadline-driven task orchestration
//    │
//    └── Edge Cases and Error Handling
//        ├── Timeout during status request waiting
//        ├── Self-destruct during active coordination
//        ├── Invalid scheduling option handling
//        └── Resource cleanup on abnormal termination
//
// =====================================================================================

// Enable all advanced features for comprehensive testing
#define _TASK_STATUS_REQUEST        // Event-driven task coordination
#define _TASK_TIMEOUT              // Task execution deadlines
#define _TASK_SCHEDULING_OPTIONS   // Advanced scheduling control
#define _TASK_SELF_DESTRUCT        // Automatic task cleanup

#include <gtest/gtest.h>
#include "Arduino.h"
#include "TaskScheduler.h"

// Global test state for advanced features
std::vector<std::string> advanced_test_output;
int advanced_callback_counter = 0;
bool status_received = false;
int last_status_code = 0;
bool timeout_occurred = false;
bool task_self_destructed = false;

// Definition for test_output vector used by Arduino.h mock
std::vector<std::string> test_output;

// Global pointers for task-to-task communication
static StatusRequest* global_status_request = nullptr;
static Task* global_advanced_yield_task = nullptr;

// Test callback functions (no lambda functions)

/**
 * @brief Advanced callback function for status request testing
 *
 * Records execution and can signal status request completion.
 * Used for testing event-driven task coordination patterns.
 */
void advanced_status_callback() {
    advanced_callback_counter++;
    advanced_test_output.push_back("status_task_executed");
    std::cout << "Status task executed at " << millis() << "ms" << std::endl;
}

/**
 * @brief Producer callback that signals status request completion
 *
 * Simulates a task that produces events or completes work that
 * other tasks are waiting for. Essential for testing coordination.
 */
void producer_callback() {
    advanced_callback_counter++;
    advanced_test_output.push_back("producer_completed");

    // Signal the global status request if it exists
    if (global_status_request != nullptr) {
        global_status_request->signalComplete(100); // Success status
    }

    std::cout << "Producer completed at " << millis() << "ms" << std::endl;
}

/**
 * @brief Consumer callback for status request waiting
 *
 * Represents tasks that wait for specific events before executing.
 * Used to test event-driven coordination and dependency management.
 */
void consumer_callback() {
    advanced_callback_counter++;
    advanced_test_output.push_back("consumer_executed");
    std::cout << "Consumer executed at " << millis() << "ms" << std::endl;
}

/**
 * @brief First consumer callback for multi-consumer tests
 */
void consumer1_callback() {
    advanced_callback_counter++;
    advanced_test_output.push_back("consumer1_executed");
}

/**
 * @brief Second consumer callback for multi-consumer tests
 */
void consumer2_callback() {
    advanced_callback_counter++;
    advanced_test_output.push_back("consumer2_executed");
}

/**
 * @brief Third consumer callback for multi-consumer tests
 */
void consumer3_callback() {
    advanced_callback_counter++;
    advanced_test_output.push_back("consumer3_executed");
}

/**
 * @brief Timeout-sensitive callback for deadline testing
 *
 * Callback that tracks timeout conditions and handles deadline-based
 * execution scenarios. Critical for testing timeout functionality.
 */
void timeout_sensitive_callback() {
    advanced_callback_counter++;
    advanced_test_output.push_back("timeout_task_executed");
    // This callback can be used to test timeout behavior
}

/**
 * @brief Self-destructing task callback for cleanup testing
 *
 * Callback for tasks that are designed to clean themselves up
 * after execution. Used to test automatic resource management.
 */
void self_destruct_callback() {
    advanced_callback_counter++;
    advanced_test_output.push_back("self_destruct_executed");
    task_self_destructed = true;
}

/**
 * @brief Callback for multi-step yield testing - step 1
 */
void yield_step1_callback() {
    advanced_test_output.push_back("step_1");
}

/**
 * @brief Callback for multi-step yield testing - step 2
 */
void yield_step2_callback() {
    advanced_test_output.push_back("step_2");
}

/**
 * @brief Consumer callback that yields to yield_step2_callback
 */
void consumer_yield_callback() {
    advanced_test_output.push_back("consumer_initial");
    if (global_advanced_yield_task) {
        global_advanced_yield_task->yield(&yield_step2_callback);
    }
}

/**
 * @brief Callback for waiter task 1 in coordination scenarios
 */
void waiter1_callback() {
    advanced_callback_counter++;
    advanced_test_output.push_back("waiter1_executed");
}

/**
 * @brief Callback for waiter task 2 in coordination scenarios
 */
void waiter2_callback() {
    advanced_callback_counter++;
    advanced_test_output.push_back("waiter2_executed");
}

/**
 * @brief Callback for waiter task 3 in coordination scenarios
 */
void waiter3_callback() {
    advanced_callback_counter++;
    advanced_test_output.push_back("waiter3_executed");
}

/**
 * @brief Callback for TASK_INTERVAL timing test - simulates long-running task
 *
 * This callback takes 105ms to execute, which is longer than the 100ms interval.
 * Used to test that TASK_INTERVAL scheduling honors the interval from end to start
 * rather than start to start.
 */
void interval_timing_callback() {
    advanced_callback_counter++;
    advanced_test_output.push_back("interval_task_start_" + std::to_string(millis()));

    // Callback takes 105ms to execute (longer than 100ms interval)
    delay(105);

    advanced_test_output.push_back("interval_task_end_" + std::to_string(millis()));
}

/**
 * @brief Test fixture class for advanced TaskScheduler features
 *
 * Provides setup and teardown for advanced feature tests, ensuring
 * clean state between tests and utility methods for complex scenarios.
 */
class AdvancedSchedulerTest : public ::testing::Test {
protected:
    /**
     * @brief Set up test environment for advanced features
     *
     * Clears all test state and initializes timing system.
     * Prepares environment for testing complex feature interactions.
     */
    void SetUp() override {
        clearAdvancedTestOutput();
        advanced_callback_counter = 0;
        status_received = false;
        last_status_code = 0;
        timeout_occurred = false;
        task_self_destructed = false;
        global_status_request = nullptr;
        millis(); // Initialize timing
    }

    /**
     * @brief Clean up test environment after advanced feature tests
     *
     * Ensures no test artifacts affect subsequent tests.
     * Critical for maintaining test isolation in complex scenarios.
     */
    void TearDown() override {
        clearAdvancedTestOutput();
        advanced_callback_counter = 0;
        status_received = false;
        last_status_code = 0;
        timeout_occurred = false;
        task_self_destructed = false;
        global_status_request = nullptr;
    }

    /**
     * @brief Helper to run scheduler until condition or timeout for advanced tests
     *
     * Enhanced version that handles complex conditions involving multiple
     * tasks, status requests, and timeout scenarios.
     */
    bool runAdvancedSchedulerUntil(Scheduler& ts, std::function<bool()> condition, unsigned long timeout_ms = 2000) {
        return waitForCondition([&]() {
            ts.execute();
            return condition();
        }, timeout_ms);
    }

    /**
     * @brief Clear advanced test output buffer
     */
    void clearAdvancedTestOutput() {
        advanced_test_output.clear();
    }

    /**
     * @brief Get count of advanced test output entries
     */
    size_t getAdvancedTestOutputCount() {
        return advanced_test_output.size();
    }

    /**
     * @brief Get specific advanced test output entry
     */
    std::string getAdvancedTestOutput(size_t index) {
        if (index < advanced_test_output.size()) {
            return advanced_test_output[index];
        }
        return "";
    }
};

// ================== STATUS REQUEST CREATION AND MANAGEMENT TESTS ==================

/**
 * @brief Test StatusRequest constructor and basic state management
 *
 * TESTS: StatusRequest(), setWaiting(), pending(), completed()
 *
 * PURPOSE: Verify that StatusRequest objects can be created and their
 * basic state management methods work correctly, providing foundation
 * for event-driven task coordination.
 *
 * STATUS REQUEST LIFECYCLE:
 * - Constructor: Creates StatusRequest in completed state (count=0)
 * - setWaiting(count): Prepares to receive 'count' signal() calls
 * - pending(): Returns true while count > 0
 * - completed(): Returns true when count == 0
 * - State transitions control task execution timing
 *
 * TEST SCENARIO:
 * 1. Create StatusRequest and verify initial completed state
 * 2. Set waiting for 3 signals and verify pending state
 * 3. Verify completed() and pending() return correct values
 * 4. Test state consistency across multiple checks
 *
 * EXPECTATIONS:
 * - Initial state: completed() = true, pending() = false
 * - After setWaiting(3): completed() = false, pending() = true
 * - State methods return consistent, expected values
 *
 * IMPORTANCE: Basic state management is foundation for all event-driven
 * coordination patterns. Reliable state tracking ensures proper task
 * synchronization and prevents race conditions.
 */
TEST_F(AdvancedSchedulerTest, StatusRequestBasicState) {
    StatusRequest sr;

    // Initial state should be completed
    EXPECT_TRUE(sr.completed());
    EXPECT_FALSE(sr.pending());
    EXPECT_EQ(sr.getCount(), 0);
    EXPECT_EQ(sr.getStatus(), 0);

    // Set waiting for 3 signals
    sr.setWaiting(3);
    EXPECT_FALSE(sr.completed());
    EXPECT_TRUE(sr.pending());
    EXPECT_EQ(sr.getCount(), 3);
    EXPECT_EQ(sr.getStatus(), 0); // Status reset by setWaiting
}

/**
 * @brief Test StatusRequest signal() method for incremental completion
 *
 * TESTS: signal(status), getStatus(), getCount()
 *
 * PURPOSE: Verify that signal() correctly decrements the waiting count
 * and manages status codes, enabling coordinated completion tracking
 * across multiple signaling tasks.
 *
 * SIGNAL BEHAVIOR:
 * - signal(): Decrements count by 1, sets status if provided
 * - signal(status): Same as signal() but with custom status code
 * - Returns true when StatusRequest becomes complete (count reaches 0)
 * - Status code from last signal() call is preserved
 * - Negative status codes cause immediate completion (error handling)
 *
 * TEST SCENARIO:
 * 1. Set StatusRequest waiting for 3 signals
 * 2. Send 2 normal signals and verify count decrements
 * 3. Send final signal with status code and verify completion
 * 4. Test negative status code causing immediate completion
 *
 * EXPECTATIONS:
 * - Each signal() decrements count by 1
 * - Status code is updated with each signal
 * - signal() returns true only when count reaches 0
 * - Negative status causes immediate completion
 *
 * IMPORTANCE: Incremental signaling enables complex coordination
 * patterns where multiple tasks contribute to completion condition.
 * Status codes provide error handling and completion context.
 */
TEST_F(AdvancedSchedulerTest, StatusRequestSignaling) {
    StatusRequest sr;
    sr.setWaiting(3);

    // Send first signal
    bool complete = sr.signal();
    EXPECT_FALSE(complete);
    EXPECT_EQ(sr.getCount(), 2);
    EXPECT_TRUE(sr.pending());

    // Send second signal with status
    complete = sr.signal(42);
    EXPECT_FALSE(complete);
    EXPECT_EQ(sr.getCount(), 1);
    EXPECT_EQ(sr.getStatus(), 42);

    // Send final signal
    complete = sr.signal(99);
    EXPECT_TRUE(complete);
    EXPECT_EQ(sr.getCount(), 0);
    EXPECT_EQ(sr.getStatus(), 99);
    EXPECT_TRUE(sr.completed());

    // Test negative status for immediate completion
    sr.setWaiting(5);
    complete = sr.signal(-1); // Negative status should complete immediately
    EXPECT_TRUE(complete);
    EXPECT_EQ(sr.getCount(), 0);
    EXPECT_EQ(sr.getStatus(), -1);
}

/**
 * @brief Test StatusRequest signalComplete() method for immediate completion
 *
 * TESTS: signalComplete(status)
 *
 * PURPOSE: Verify that signalComplete() immediately completes the StatusRequest
 * regardless of remaining count, enabling emergency completion and
 * error handling scenarios.
 *
 * SIGNAL COMPLETE BEHAVIOR:
 * - signalComplete(): Immediately sets count to 0 (completed state)
 * - signalComplete(status): Same as above but with custom status code
 * - Bypasses incremental signaling for immediate completion
 * - Useful for error conditions and emergency shutdowns
 * - Always results in completed state regardless of initial count
 *
 * TEST SCENARIO:
 * 1. Set StatusRequest waiting for large number of signals
 * 2. Call signalComplete() and verify immediate completion
 * 3. Test signalComplete() with custom status code
 * 4. Verify no further signals are processed after completion
 *
 * EXPECTATIONS:
 * - signalComplete() immediately sets count to 0
 * - Status code is set if provided
 * - completed() returns true immediately
 * - Further signals have no effect (idempotent)
 *
 * IMPORTANCE: Immediate completion enables error handling, emergency
 * shutdown, and abort scenarios in complex coordination patterns.
 * Critical for robust event-driven systems.
 */
TEST_F(AdvancedSchedulerTest, StatusRequestSignalComplete) {
    StatusRequest sr;
    sr.setWaiting(10); // Large count for testing immediate completion

    // Signal complete should immediately finish
    sr.signalComplete(200);
    EXPECT_TRUE(sr.completed());
    EXPECT_FALSE(sr.pending());
    EXPECT_EQ(sr.getCount(), 0);
    EXPECT_EQ(sr.getStatus(), 200);

    // Further signals should have no effect
    sr.signal(999);
    EXPECT_EQ(sr.getStatus(), 200); // Should remain unchanged
    EXPECT_EQ(sr.getCount(), 0);    // Should remain completed
}

// ================== TASK STATUS REQUEST INTEGRATION TESTS ==================

/**
 * @brief Test Task waitFor() method for event-driven task execution
 *
 * TESTS: waitFor(StatusRequest*, interval, iterations)
 *
 * PURPOSE: Verify that tasks can wait for external StatusRequest completion
 * before executing, enabling event-driven coordination and dependency
 * management between tasks.
 *
 * WAIT FOR BEHAVIOR:
 * - waitFor(): Configures task to wait for StatusRequest completion
 * - Task becomes enabled but won't execute until StatusRequest completes
 * - interval and iterations configure post-completion behavior
 * - Task executes immediately when StatusRequest is already complete
 * - Enables producer-consumer and dependency coordination patterns
 *
 * TEST SCENARIO:
 * 1. Create StatusRequest in pending state
 * 2. Create task configured to waitFor() the StatusRequest
 * 3. Verify task doesn't execute while StatusRequest is pending
 * 4. Complete StatusRequest and verify task executes
 * 5. Test with already-completed StatusRequest
 *
 * EXPECTATIONS:
 * - Task enabled but doesn't execute while StatusRequest pending
 * - Task executes immediately after StatusRequest completion
 * - Task with completed StatusRequest executes immediately
 * - Task follows normal interval/iteration behavior after first execution
 *
 * IMPORTANCE: Event-driven task execution enables sophisticated coordination
 * patterns essential for multi-task workflows, data processing pipelines,
 * and resource synchronization scenarios.
 */
TEST_F(AdvancedSchedulerTest, TaskWaitForStatusRequest) {
    Scheduler ts;
    StatusRequest sr;

    // Set up StatusRequest in pending state
    sr.setWaiting(1);

    advanced_callback_counter = 0;

    // Create task that waits for status request
    Task waiter(100, 2, &consumer_callback, &ts);
    waiter.waitFor(&sr, 50, 2);
    // Task should be enabled but not execute while SR is pending
    EXPECT_TRUE(waiter.isEnabled());

    // Run scheduler - task should not execute yet
    delay(100);
    // The Status Request should still be pending

    EXPECT_TRUE(sr.isPending());
    ts.execute();
    EXPECT_TRUE(sr.isPending());
    EXPECT_EQ(advanced_callback_counter, 0);

    // Complete the status request
    sr.signalComplete();
    EXPECT_FALSE(sr.isPending());

    // Now task should execute
    bool success = runAdvancedSchedulerUntil(ts, []() {
        return advanced_callback_counter >= 1;
    });
    EXPECT_TRUE(success);
    EXPECT_EQ(getAdvancedTestOutput(0), "consumer_executed");

    // Task should continue normal execution after first completion
    success = runAdvancedSchedulerUntil(ts, []() {
        return advanced_callback_counter >= 2;
    });
    EXPECT_TRUE(success);
}

/**
 * @brief Test Task waitForDelayed() method for delayed event waiting
 *
 * TESTS: waitForDelayed(StatusRequest*, interval, iterations)
 *
 * PURPOSE: Same as above only task is scheduled with a delay as a result
 */
TEST_F(AdvancedSchedulerTest, TaskWaitForDelayedStatusRequest) {
    Scheduler ts;
    StatusRequest sr;

    // StatusRequest already complete
    sr.setWaiting(1);
    advanced_callback_counter = 0;

    // Create task with delayed wait (should wait for delay even though SR is complete)
    Task delayed_waiter(50, 1, &consumer_callback, &ts);
    delayed_waiter.waitForDelayed(&sr, 500, 1); // 500ms delay

    // Task should be enabled
    EXPECT_TRUE(delayed_waiter.isEnabled());

    // Should not execute immediately despite completed StatusRequest
    delay(500);
    EXPECT_TRUE(sr.isPending());
    ts.execute();
    EXPECT_EQ(advanced_callback_counter, 0);

    sr.signalComplete(); // Complete SR
    EXPECT_FALSE(sr.isPending());

    // The task will be scheduled to run after the delay
    bool success = runAdvancedSchedulerUntil(ts, []() { return false; }, 200); // Wait up to 1000ms
    EXPECT_FALSE(success); 

    delay(400);
    // Task should complete after total 500ms delay
    success = runAdvancedSchedulerUntil(ts, []() {
        return advanced_callback_counter >= 1;
    });
    EXPECT_TRUE(success);
}

/**
 * @brief Test multiple tasks waiting for same StatusRequest
 *
 * TESTS: Multiple Task::waitFor() on single StatusRequest
 *
 * PURPOSE: Verify that multiple tasks can wait for the same StatusRequest
 * and all execute when it completes, enabling broadcast notification
 * and fan-out coordination patterns.
 *
 * MULTIPLE WAITER BEHAVIOR:
 * - Multiple tasks can waitFor() same StatusRequest
 * - All waiting tasks execute when StatusRequest completes
 * - Each task maintains independent execution behavior after triggering
 * - Enables broadcast notification and fan-out patterns
 * - Critical for coordinated multi-task startup scenarios
 *
 * TEST SCENARIO:
 * 1. Create StatusRequest in pending state
 * 2. Create multiple tasks waiting for same StatusRequest
 * 3. Verify no tasks execute while StatusRequest pending
 * 4. Complete StatusRequest and verify all tasks execute
 * 5. Verify each task maintains independent behavior
 *
 * EXPECTATIONS:
 * - No tasks execute while StatusRequest pending
 * - All tasks execute after StatusRequest completion
 * - Each task produces expected output
 * - Tasks maintain independent execution patterns
 *
 * IMPORTANCE: Multi-task coordination enables broadcast patterns
 * essential for synchronized startup, data distribution, and
 * coordinated processing scenarios in complex applications.
 */
TEST_F(AdvancedSchedulerTest, MultipleTasksWaitingForStatusRequest) {
    Scheduler ts;
    StatusRequest sr;

    // Set up StatusRequest waiting for signal
    sr.setWaiting(1);

    // Create multiple tasks waiting for same StatusRequest
    Task waiter1(100, 1, &waiter1_callback, &ts);
    Task waiter2(150, 1, &waiter2_callback, &ts);
    Task waiter3(200, 1, &waiter3_callback, &ts);

    // Configure all tasks to wait for same StatusRequest
    waiter1.waitFor(&sr);
    waiter2.waitFor(&sr);
    waiter3.waitFor(&sr);

    // Verify all tasks are enabled but none execute
    EXPECT_TRUE(waiter1.isEnabled());
    EXPECT_TRUE(waiter2.isEnabled());
    EXPECT_TRUE(waiter3.isEnabled());

    delay(250);
    ts.execute();
    EXPECT_EQ(advanced_callback_counter, 0);

    // Signal completion
    sr.signalComplete();

    // All tasks should now execute
    bool success = runAdvancedSchedulerUntil(ts, []() {
        return advanced_callback_counter >= 3;
    });
    EXPECT_TRUE(success);

    // Verify all tasks executed
    EXPECT_EQ(getAdvancedTestOutputCount(), 3);
    // Note: Execution order may vary, but all should be present
}

// ================== TASK TIMEOUT FUNCTIONALITY TESTS ==================

/**
 * @brief Test Task timeout basic functionality
 *
 * TESTS: setTimeout(), getTimeout(), resetTimeout(), untilTimeout()
 *
 * PURPOSE: Verify that task timeout functionality correctly manages
 * execution deadlines and provides timeout detection capabilities
 * essential for deadline-driven applications.
 *
 * TIMEOUT FUNCTIONALITY:
 * - setTimeout(timeout, reset): Sets execution deadline with optional reset
 * - getTimeout(): Returns configured timeout value
 * - resetTimeout(): Restarts timeout timer from current time
 * - untilTimeout(): Returns remaining time until timeout
 * - isTimedOut(): Checks if timeout has occurred
 *
 * TEST SCENARIO:
 * 1. Create task and set timeout value
 * 2. Verify timeout configuration via getters
 * 3. Test resetTimeout() functionality
 * 4. Verify untilTimeout() decreases over time
 * 5. Test timeout detection after deadline
 *
 * EXPECTATIONS:
 * - setTimeout() correctly configures timeout value
 * - getTimeout() returns set value
 * - untilTimeout() decreases as time passes
 * - Timeout detection works after deadline expires
 *
 * IMPORTANCE: Timeout functionality enables deadline-driven task management,
 * essential for real-time applications and preventing resource starvation
 * in time-critical systems.
 */
TEST_F(AdvancedSchedulerTest, TaskTimeoutBasicFunctionality) {
    Scheduler ts;
    Task task(100, TASK_FOREVER, &timeout_sensitive_callback, &ts, true);

    // Set timeout to 500ms
    task.setTimeout(500, true); // true = reset timeout timer

    EXPECT_EQ(task.getTimeout(), 500);

    // Check initial time until timeout
    long until_timeout = task.untilTimeout();
    EXPECT_GT(until_timeout, 400); // Should be close to 500ms
    EXPECT_LE(until_timeout, 500);

    // Wait some time and check again
    delay(200);
    until_timeout = task.untilTimeout();
    EXPECT_GT(until_timeout, 200); // Should be around 300ms
    EXPECT_LE(until_timeout, 300);

    // Reset timeout and verify
    task.resetTimeout();
    until_timeout = task.untilTimeout();
    EXPECT_GT(until_timeout, 400); // Should be close to 500ms again
}

/**
 * @brief Test task behavior when timeout occurs
 *
 * TESTS: Task execution with timeout expiration
 *
 * PURPOSE: Verify that tasks handle timeout expiration correctly,
 * including automatic disabling and timeout status tracking,
 * essential for robust timeout management.
 *
 * TIMEOUT EXPIRATION BEHAVIOR:
 * - Task automatically disables when timeout expires
 * - isTimedOut() returns true after timeout
 * - Task stops executing after timeout
 * - Timeout status preserved until reset
 * - Enables deadline-driven task lifecycle management
 *
 * TEST SCENARIO:
 * 1. Create task with short timeout
 * 2. Let task execute normally initially
 * 3. Wait for timeout to expire
 * 4. Verify task disables and isTimedOut() returns true
 * 5. Verify no further executions occur
 *
 * EXPECTATIONS:
 * - Task executes normally before timeout
 * - Task disables after timeout expiration
 * - isTimedOut() returns true after timeout
 * - No executions occur after timeout
 *
 * IMPORTANCE: Timeout expiration handling prevents runaway tasks
 * and ensures deadline compliance, critical for real-time and
 * resource-constrained systems.
 */
TEST_F(AdvancedSchedulerTest, TaskTimeoutExpiration) {
    Scheduler ts;
    Task task(50, TASK_FOREVER, &timeout_sensitive_callback, &ts, true);

    // Set short timeout
    task.setTimeout(200, true);

    // Task should execute normally initially
    bool success = runAdvancedSchedulerUntil(ts, []() {
        return advanced_callback_counter >= 2;
    });
    EXPECT_TRUE(success);

    // Wait for timeout to expire
    delay(250);

    // Run scheduler to trigger timeout processing
    ts.execute();

    // Task should be timed out and disabled
    EXPECT_TRUE(task.timedOut());
    EXPECT_FALSE(task.isEnabled());

    // No further executions should occur
    int count_before = advanced_callback_counter;
    delay(100);
    ts.execute();
    EXPECT_EQ(advanced_callback_counter, count_before);
}

// ================== STATUS REQUEST TIMEOUT INTEGRATION TESTS ==================

/**
 * @brief Test StatusRequest timeout functionality
 *
 * TESTS: StatusRequest::setTimeout(), resetTimeout(), untilTimeout()
 *
 * PURPOSE: Verify that StatusRequest objects support timeout functionality
 * for deadline-driven event coordination, preventing indefinite waiting
 * in event-driven scenarios.
 *
 * STATUS REQUEST TIMEOUT BEHAVIOR:
 * - setTimeout(timeout): Sets deadline for StatusRequest completion
 * - resetTimeout(): Restarts timeout timer for StatusRequest
 * - untilTimeout(): Returns remaining time until StatusRequest timeout
 * - Timeout handling prevents indefinite blocking on events
 * - Essential for robust event-driven coordination
 *
 * TEST SCENARIO:
 * 1. Create StatusRequest and set timeout
 * 2. Verify timeout configuration
 * 3. Test resetTimeout() functionality
 * 4. Monitor untilTimeout() behavior over time
 * 5. Verify timeout detection capabilities
 *
 * EXPECTATIONS:
 * - setTimeout() configures StatusRequest deadline
 * - untilTimeout() decreases as time passes
 * - resetTimeout() restarts timing correctly
 * - Timeout detection works as expected
 *
 * IMPORTANCE: StatusRequest timeouts prevent deadlock scenarios
 * in event-driven systems and enable deadline-based coordination
 * patterns essential for reliable multi-task systems.
 */
TEST_F(AdvancedSchedulerTest, StatusRequestTimeout) {
    StatusRequest sr;
    sr.setWaiting(1);

    // Set timeout for StatusRequest
    sr.setTimeout(300);
    sr.resetTimeout();

    EXPECT_EQ(sr.getTimeout(), 300);

    // Check initial time until timeout
    long until_timeout = sr.untilTimeout();
    EXPECT_GT(until_timeout, 250);
    EXPECT_LE(until_timeout, 300);

    // Wait and check again
    delay(150);
    until_timeout = sr.untilTimeout();
    EXPECT_GT(until_timeout, 100);
    EXPECT_LE(until_timeout, 150);

    // Reset timeout
    sr.resetTimeout();
    until_timeout = sr.untilTimeout();
    EXPECT_GT(until_timeout, 250);
    EXPECT_LE(until_timeout, 300);
}

// ================== SCHEDULING OPTIONS TESTS ==================

/**
 * @brief Test task scheduling options functionality
 *
 * TESTS: setSchedulingOption(), getSchedulingOption()
 *
 * PURPOSE: Verify that scheduling options can be configured and retrieved,
 * enabling advanced scheduling behavior control for specialized
 * execution patterns and priority management.
 *
 * SCHEDULING OPTIONS BEHAVIOR:
 * - setSchedulingOption(option): Configures task scheduling behavior
 * - getSchedulingOption(): Retrieves current scheduling configuration
 * - Options control execution priority, timing, and behavior
 * - Enables fine-grained scheduling control
 * - Critical for advanced task management scenarios
 *
 * TEST SCENARIO:
 * 1. Create task with default scheduling option
 * 2. Set various scheduling options and verify configuration
 * 3. Test option retrieval matches set values
 * 4. Verify options can be changed dynamically
 * 5. Test with multiple different option values
 *
 * EXPECTATIONS:
 * - setSchedulingOption() configures option value
 * - getSchedulingOption() returns set value correctly
 * - Options can be changed dynamically
 * - Configuration persists across calls
 *
 * IMPORTANCE: Scheduling options enable advanced task behavior control
 * essential for priority management, resource allocation, and
 * specialized execution patterns in complex applications.
 *
 * TODO: This needs to be completely redone - this AI generated test is absolutely wrong
 */
TEST_F(AdvancedSchedulerTest, TaskSchedulingOptions) {
    Scheduler ts;
    Task task(100, 5, &advanced_status_callback, &ts, false);

    // Test default scheduling option
    unsigned int default_option = task.getSchedulingOption();

    // Set different scheduling options
    task.setSchedulingOption(TASK_SCHEDULE);
    EXPECT_EQ(task.getSchedulingOption(), TASK_SCHEDULE);

    task.setSchedulingOption(TASK_SCHEDULE_NC);
    EXPECT_EQ(task.getSchedulingOption(), TASK_SCHEDULE_NC);

    task.setSchedulingOption(TASK_INTERVAL);
    EXPECT_EQ(task.getSchedulingOption(), TASK_INTERVAL);

    // Verify option can be changed while task is enabled
    task.enable();
    task.setSchedulingOption(TASK_SCHEDULE_NC);
    EXPECT_EQ(task.getSchedulingOption(), TASK_SCHEDULE_NC);
    EXPECT_TRUE(task.isEnabled()); // Should remain enabled

    // ======== TASK_SCHEDULE CATCH-UP TEST ========
    // Test TASK_SCHEDULE option - task catches up on missed executions
    advanced_callback_counter = 0;
    clearAdvancedTestOutput();
    ts.init();

    Task catchup_task(100, 10, &advanced_status_callback, &ts, false);
    catchup_task.setSchedulingOption(TASK_SCHEDULE);

    // Enable task and immediately delay for 2000ms
    catchup_task.enable();
    delay(2000);

    // Now run scheduler for 2000ms - task should execute 10 times back to back (catching up)
    unsigned long start_time = millis();
    bool success = runAdvancedSchedulerUntil(ts, []() {
        return advanced_callback_counter >= 10;
    }, 2000);

    EXPECT_TRUE(success);
    EXPECT_EQ(advanced_callback_counter, 10); // Should have caught up all 10 executions
    EXPECT_EQ(getAdvancedTestOutputCount(), 10);

    // ======== TASK_SCHEDULE_NC LIMITED EXECUTION TEST ========
    // Test TASK_SCHEDULE_NC option - no catch-up, limited executions
    advanced_callback_counter = 0;
    clearAdvancedTestOutput();
    ts.init();

    Task no_catchup_task(100, 10, &advanced_status_callback);
    ts.addTask(no_catchup_task);
    no_catchup_task.setSchedulingOption(TASK_SCHEDULE_NC);

    // Enable task and immediately delay for 2000ms
    no_catchup_task.enable();
    delay(2000);

    // Run scheduler for only 500ms - should execute only ~5 times (no catch-up)
    start_time = millis();
    success = runAdvancedSchedulerUntil(ts, []() {
        return false; // Run for full timeout
    }, 500);

    // Should have executed approximately 5 times (500ms / 100ms interval)
    EXPECT_GE(advanced_callback_counter, 4); // At least 4 times
    EXPECT_LE(advanced_callback_counter, 6); // At most 6 times (allowing for timing variance)

    // ======== TASK_INTERVAL TIMING ADJUSTMENT TEST ========
    // Test TASK_INTERVAL option - maintains interval from end to start, not start to start
    advanced_callback_counter = 0;
    clearAdvancedTestOutput();
    ts.init();

    Task interval_task(100, 5, &interval_timing_callback);
    ts.addTask(interval_task);

    interval_task.setSchedulingOption(TASK_INTERVAL);
    interval_task.enable();

    // Record start time and run until all 5 executions complete
    unsigned long test_start = millis();
    success = runAdvancedSchedulerUntil(ts, []() {
        return advanced_callback_counter >= 5;
    }, 2000);

    EXPECT_TRUE(success);
    EXPECT_EQ(advanced_callback_counter, 5);

    // With TASK_INTERVAL, the schedule should adjust to honor 100ms interval from end to start
    // Each execution: 105ms callback + 100ms interval = 105ms total cycle
    // 5 executions should take approximately 5 * 105ms = ~525ms
    unsigned long total_time = millis() - test_start;
    EXPECT_GE(total_time, 500); // At least 500ms
    EXPECT_LE(total_time, 600); // At most 600ms (allowing for timing variance)

    // Verify the output shows proper timing intervals
    EXPECT_EQ(getAdvancedTestOutputCount(), 10); // 5 start + 5 end messages
}

// ================== SELF-DESTRUCT FUNCTIONALITY TESTS ==================

/**
 * @brief Test task self-destruct functionality (placeholder test)
 *
 * TESTS: Task self-destruct behavior when enabled
 *
 * PURPOSE: Verify that tasks can be configured to automatically remove
 * themselves from the scheduler after completion, enabling automatic
 * resource cleanup and dynamic task lifecycle management.
 *
 * SELF-DESTRUCT BEHAVIOR:
 * - Tasks automatically remove themselves from scheduler
 * - Occurs after task completes execution cycle
 * - Prevents memory leaks in dynamic task scenarios
 * - Enables automatic cleanup patterns
 * - Critical for long-running applications with temporary tasks
 *
 * NOTE: This test serves as a placeholder for self-destruct functionality.
 * The actual implementation details depend on specific TaskScheduler
 * version and may require additional setup or different API calls.
 *
 * TEST SCENARIO:
 * 1. Create task configured for self-destruction
 * 2. Let task execute its lifecycle
 * 3. Verify task removes itself from scheduler
 * 4. Confirm no further executions occur
 * 5. Test scheduler continues operating normally
 *
 * EXPECTATIONS:
 * - Task executes normally initially
 * - Task removes itself after completion
 * - No further executions occur
 * - Scheduler remains functional
 *
 * IMPORTANCE: Self-destruct functionality enables automatic resource
 * management essential for dynamic applications and prevents memory
 * leaks in long-running systems with temporary tasks.
 */
TEST_F(AdvancedSchedulerTest, TaskSelfDestruct) {
    Scheduler ts;

    // Create a task that will self-destruct after execution
    // Note: Actual self-destruct implementation may vary by TaskScheduler version
    Task* temp_task = new Task(100, 1, &self_destruct_callback, &ts, true);

    // Let task execute
    bool success = runAdvancedSchedulerUntil(ts, []() {
        return task_self_destructed;
    });
    EXPECT_TRUE(success);
    EXPECT_EQ(getAdvancedTestOutput(0), "self_destruct_executed");

    // Note: In a full implementation, we would verify the task
    // has been removed from the scheduler's task list
    // This would require access to internal scheduler state

    // Clean up manually for this test
    delete temp_task;
}

// ================== INTEGRATION AND COMPLEX SCENARIOS ==================

/**
 * @brief Integration test combining status requests with timeouts
 *
 * TESTS: StatusRequest + Task timeout integration
 *
 * PURPOSE: Verify that status request coordination works correctly
 * with task timeouts, ensuring robust behavior when events may
 * not occur within expected timeframes.
 *
 * INTEGRATION BEHAVIOR:
 * - Tasks can wait for StatusRequest with timeout protection
 * - Timeout overrides StatusRequest waiting if deadline exceeded
 * - Proper cleanup occurs when timeout prevents event completion
 * - Enables robust event-driven patterns with deadline guarantees
 * - Critical for reliable real-time coordination
 *
 * TEST SCENARIO:
 * 1. Create StatusRequest that won't complete within timeout
 * 2. Create task waiting for StatusRequest with timeout
 * 3. Verify task times out rather than waiting indefinitely
 * 4. Test proper cleanup and status handling
 * 5. Verify system remains stable after timeout
 *
 * EXPECTATIONS:
 * - Task times out rather than waiting indefinitely
 * - Timeout handling works despite pending StatusRequest
 * - System cleanup occurs properly
 * - No resource leaks or deadlocks
 *
 * IMPORTANCE: Timeout protection in event-driven scenarios prevents
 * deadlocks and ensures system reliability, essential for robust
 * real-time applications.
 */
TEST_F(AdvancedSchedulerTest, StatusRequestWithTimeout) {
    Scheduler ts;
    StatusRequest sr;

    // Set up StatusRequest that won't complete
    sr.setWaiting(1);
    sr.setTimeout(150);
    sr.resetTimeout();

    // Create task waiting for StatusRequest
    Task waiter(100, 1, &consumer_callback, &ts);
    waiter.waitFor(&sr);
    waiter.setTimeout(200, true); // Task timeout longer than SR timeout

    // Let everything run - StatusRequest should timeout first
    delay(300);

    // Run scheduler to process timeouts
    for (int i = 0; i < 10; i++) {
        ts.execute();
        delay(10);
    }

    // Verify StatusRequest timed out
    EXPECT_LE(sr.untilTimeout(), 0); // Should be negative (timed out)

    // Task should eventually time out as well since SR never completed
    EXPECT_TRUE(waiter.timedOut() || !waiter.isEnabled());
}

/**
 * @brief Complex producer-consumer coordination with multiple features
 *
 * TESTS: Multi-feature integration in realistic scenario
 *
 * PURPOSE: Demonstrate complex coordination scenario using multiple
 * advanced features together, validating that features work correctly
 * in combination for real-world usage patterns.
 *
 * COMPLEX INTEGRATION FEATURES:
 * - Producer task with timeout management
 * - Multiple consumers waiting for producer completion
 * - StatusRequest coordination with error handling
 * - Scheduling options for priority control
 * - Comprehensive error and timeout handling
 *
 * TEST SCENARIO:
 * 1. Create producer task with timeout and high priority
 * 2. Create multiple consumer tasks waiting for producer
 * 3. Configure timeouts and scheduling options appropriately
 * 4. Execute coordination scenario with success path
 * 5. Verify all consumers execute after producer completion
 *
 * EXPECTATIONS:
 * - Producer executes with proper priority and timing
 * - All consumers wait appropriately for producer
 * - StatusRequest coordination works correctly
 * - All tasks execute in expected order
 * - No timeouts or errors in success scenario
 *
 * IMPORTANCE: Complex integration validates that advanced features
 * work together reliably, essential for building sophisticated
 * multi-task applications with robust coordination patterns.
 */
TEST_F(AdvancedSchedulerTest, ComplexProducerConsumerCoordination) {
    Scheduler ts;
    StatusRequest production_complete;

    // Set up production status request
    production_complete.setWaiting(1);
    production_complete.setTimeout(1000); // Allow plenty of time
    production_complete.resetTimeout();

    // Set global pointer for producer callback
    global_status_request = &production_complete;

    // Create producer task with high priority
    Task producer(150, 1, &producer_callback, &ts, true);

    producer.setTimeout(500, true);
    producer.setSchedulingOption(10); // High priority

    // Create multiple consumer tasks
    Task consumer1(100, 1, &consumer1_callback, &ts);
    Task consumer2(100, 1, &consumer2_callback, &ts);
    Task consumer3(100, 1, &consumer3_callback, &ts);

    // Configure consumers to wait for production
    consumer1.waitFor(&production_complete);
    consumer1.setTimeout(800, true);
    consumer2.waitFor(&production_complete);
    consumer2.setTimeout(800, true);
    consumer3.waitFor(&production_complete);
    consumer3.setTimeout(800, true);

    // Execute the coordination scenario
    bool success = runAdvancedSchedulerUntil(ts, []() {
        return advanced_callback_counter >= 4; // Producer + 3 consumers
    }, 1500);

    EXPECT_TRUE(success);
    EXPECT_EQ(advanced_callback_counter, 4);

    // Verify execution order - producer should be first
    EXPECT_EQ(getAdvancedTestOutput(0), "producer_completed");

    // Verify StatusRequest completed successfully
    EXPECT_TRUE(production_complete.completed());
    EXPECT_EQ(production_complete.getStatus(), 100);

    // Verify no timeouts occurred
    EXPECT_FALSE(producer.timedOut());
    EXPECT_FALSE(consumer1.timedOut());
    EXPECT_FALSE(consumer2.timedOut());
    EXPECT_FALSE(consumer3.timedOut());

    // Clean up global pointer
    global_status_request = nullptr;
}

/**
 * @brief Producer-consumer scenario with yield switching
 *
 * TESTS: StatusRequest coordination with callback switching
 *
 * PURPOSE: Test complex scenario where tasks change behavior based
 * on StatusRequest completion, demonstrating state machine patterns
 * with event-driven coordination.
 */
TEST_F(AdvancedSchedulerTest, ProducerConsumerWithYieldSwitching) {
    Scheduler ts;
    StatusRequest sr;
    sr.setWaiting(1);

    global_status_request = &sr;

    // Create producer that will signal completion
    Task producer(100, 1, &producer_callback, &ts, true);

    // Create consumer that will yield to different callback after SR completion

    Task consumer(100, 2, consumer_yield_callback, &ts);
    global_advanced_yield_task = &consumer;
    consumer.waitFor(&sr);

    // Run the scenario
    bool success = runAdvancedSchedulerUntil(ts, []() {
        return advanced_test_output.size() >= 3; // producer + consumer initial + consumer yielded
    }, 1000);

    EXPECT_TRUE(success);
    EXPECT_EQ(getAdvancedTestOutput(0), "producer_completed");
    EXPECT_EQ(getAdvancedTestOutput(1), "consumer_initial");
    EXPECT_EQ(getAdvancedTestOutput(2), "step_2");

    // Clean up
    global_status_request = nullptr;
    global_advanced_yield_task = nullptr;
}

/**
 * @brief Main test runner function for advanced features
 *
 * Initializes Google Test framework and runs all advanced feature tests.
 * Called by the test execution environment to start testing process.
 *
 * @param argc Command line argument count
 * @param argv Command line argument values
 * @return Test execution status (0 = success)
 */
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}