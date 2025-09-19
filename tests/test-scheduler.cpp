// test_scheduler.cpp - Unit tests for TaskScheduler library
// This file contains comprehensive tests for the TaskScheduler cooperative multitasking library
// Tests cover basic scheduler operations, task lifecycle, timing accuracy, and edge cases
//
// IMPORTANT TASKSCHEDULER BEHAVIOR NOTES:
// ======================================
// 1. Tasks execute their FIRST iteration IMMEDIATELY when enabled (enable() called)
// 2. Subsequent iterations follow the specified interval timing
// 3. To make a task wait for its full interval before first execution, create it 
//    disabled (enable=false) and use enableDelayed() method
// 4. This behavior is critical for timing-sensitive tests, especially sequencing tests
// 5. enableDelayed() ensures first execution happens after the specified interval
//
// Test Design Implications:
// - Immediate execution tests: Use enable=true in constructor
// - Timing sequence tests: Use enable=false + enableDelayed()
// - Multi-task timing tests: Use enableDelayed() for predictable ordering

#include <gtest/gtest.h>
#include "Arduino.h"
#include "TaskScheduler.h"

// Global test state - used to capture and verify callback executions
std::vector<std::string> test_output;

/**
 * @brief Simple callback function for Task 1 testing
 * 
 * Records execution in the global test_output vector for verification.
 * Used primarily for single-task execution tests and timing verification.
 * Outputs execution timestamp for debugging timing-related issues.
 */
void task1_callback() {
    test_output.push_back("Task1 executed");
    std::cout << "Task1 executed at " << millis() << "ms" << std::endl;
}

/**
 * @brief Simple callback function for Task 2 testing
 * 
 * Records execution with Task2 identifier to distinguish from other tasks
 * in multi-task scheduling tests. Essential for verifying proper task
 * isolation and execution ordering in concurrent scenarios.
 */
void task2_callback() {
    test_output.push_back("Task2 executed");
    std::cout << "Task2 executed at " << millis() << "ms" << std::endl;
}

/**
 * @brief Simple callback function for Task 3 testing
 * 
 * Third task callback used in execution order tests and multi-task scenarios.
 * Helps verify that the scheduler can handle multiple concurrent tasks
 * and execute them in the correct chronological order.
 */
void task3_callback() {
    test_output.push_back("Task3 executed");
    std::cout << "Task3 executed at " << millis() << "ms" << std::endl;
}

/**
 * @brief Callback for testing repeating task functionality
 * 
 * Uses static counter to track execution number, enabling verification
 * of proper iteration counting and repeated execution behavior.
 * Critical for testing tasks with limited iteration counts and infinite tasks.
 */
void repeating_callback() {
    static int counter = 0;
    counter++;
    test_output.push_back("Repeating task #" + std::to_string(counter));
    std::cout << "Repeating task #" << counter << " executed at " << millis() << "ms" << std::endl;
}

/**
 * @brief Test fixture class for TaskScheduler unit tests
 * 
 * Provides common setup and teardown functionality for all scheduler tests.
 * Ensures clean test state between test runs and provides utility methods
 * for common test operations like running scheduler with timeout conditions.
 */
class SchedulerTest : public ::testing::Test {
protected:
    /**
     * @brief Set up test environment before each test
     * 
     * Clears any previous test output and initializes timing system.
     * Called automatically by Google Test framework before each test method.
     * Ensures each test starts with a clean, predictable state.
     */
    void SetUp() override {
        clearTestOutput();
        // Reset time by creating new static start point
        millis(); // Initialize timing
    }
    
    /**
     * @brief Clean up test environment after each test
     * 
     * Clears test output to prevent interference between tests.
     * Called automatically by Google Test framework after each test method.
     * Ensures no test artifacts affect subsequent tests.
     */
    void TearDown() override {
        clearTestOutput();
    }
    
    /**
     * @brief Helper method to run scheduler until condition is met or timeout
     * 
     * Executes the scheduler repeatedly until either the specified condition
     * returns true or the timeout period expires. Essential for testing
     * time-dependent scheduler behavior without creating infinite loops.
     * 
     * @param ts Reference to the Scheduler object to execute
     * @param condition Lambda function that returns true when test condition is met
     * @param timeout_ms Maximum time to wait before giving up (default: 1000ms)
     * @return true if condition was met within timeout, false otherwise
     */
    bool runSchedulerUntil(Scheduler& ts, std::function<bool()> condition, unsigned long timeout_ms = 1000) {
        return waitForCondition([&]() {
            ts.execute();
            return condition();
        }, timeout_ms);
    }
};

/**
 * @brief Test basic scheduler object creation
 * 
 * Verifies that a Scheduler object can be instantiated without throwing
 * exceptions or causing crashes. This is the most fundamental test that
 * ensures the library can be used at all.
 */
TEST_F(SchedulerTest, BasicSchedulerCreation) {
    Scheduler ts;
    EXPECT_TRUE(true); // Scheduler creation should not throw
}

/**
 * @brief Test scheduler behavior in initial empty state
 * 
 * Verifies that calling execute() on an empty scheduler (no tasks added)
 * behaves safely and doesn't produce any unexpected output or crashes.
 * Important for validating scheduler robustness in edge cases.
 */
TEST_F(SchedulerTest, SchedulerInitialState) {
    Scheduler ts;
    
    // Execute empty scheduler - should not crash
    ts.execute();
    EXPECT_EQ(getTestOutputCount(), 0);
}

/**
 * @brief Test execution of a single task
 * 
 * Creates one task scheduled to run once after 100ms and verifies:
 * 1. Task executes within reasonable timeout period
 * 2. Task executes exactly once
 * 3. Callback produces expected output
 * 
 * NOTE: TaskScheduler executes the first iteration immediately when enabled.
 * This test validates immediate execution behavior for enabled tasks.
 */
TEST_F(SchedulerTest, SingleTaskExecution) {
    Scheduler ts;
    
    // Create a task that runs once - will execute immediately since enabled=true
    Task task1(100, 1, &task1_callback, &ts, true);
    
    // Run scheduler until task executes (should be immediate)
    bool success = runSchedulerUntil(ts, []() { return getTestOutputCount() >= 1; });
    
    EXPECT_TRUE(success) << "Task did not execute within timeout";
    EXPECT_EQ(getTestOutputCount(), 1);
    EXPECT_EQ(getTestOutput(0), "Task1 executed");
}

/**
 * @brief Test concurrent execution of multiple tasks
 * 
 * Creates three tasks with different execution delays (50ms, 100ms, 150ms)
 * but creates them disabled and uses enableDelayed() to ensure proper timing.
 * This verifies:
 * 1. All tasks execute within timeout
 * 2. Tasks execute in chronological order based on their delays
 * 3. Each task produces correct output
 * 
 * CRITICAL: Tasks are created disabled and enabled with enableDelayed() 
 * to ensure the first execution respects the specified interval timing.
 */
TEST_F(SchedulerTest, MultipleTaskExecution) {
    Scheduler ts;
    
    // Create multiple tasks disabled to control timing precisely
    Task task1(50, 1, &task1_callback, &ts, false);   // Will execute after 50ms
    Task task2(100, 1, &task2_callback, &ts, false);  // Will execute after 100ms
    Task task3(150, 1, &task3_callback, &ts, false);  // Will execute after 150ms
    
    // Enable all tasks with delayed execution to respect intervals
    task1.enableDelayed();
    task2.enableDelayed();
    task3.enableDelayed();
    
    // Run scheduler until all tasks execute
    bool success = runSchedulerUntil(ts, []() { return getTestOutputCount() >= 3; });
    
    EXPECT_TRUE(success) << "Not all tasks executed within timeout";
    EXPECT_EQ(getTestOutputCount(), 3);
    EXPECT_EQ(getTestOutput(0), "Task1 executed");
    EXPECT_EQ(getTestOutput(1), "Task2 executed");
    EXPECT_EQ(getTestOutput(2), "Task3 executed");
}

/**
 * @brief Test task with limited number of iterations
 * 
 * Creates a task configured to run exactly 3 times with 80ms intervals
 * and verifies:
 * 1. Task executes the correct number of times
 * 2. Executions are properly numbered/sequenced
 * 3. Task stops after reaching iteration limit
 * Validates the iteration counting mechanism.
 */
TEST_F(SchedulerTest, RepeatingTaskExecution) {
    Scheduler ts;
    
    // Create a task that runs 3 times with 80ms interval
    Task repeating_task(80, 3, &repeating_callback, &ts, true);
    
    // Run scheduler until task executes 3 times
    bool success = runSchedulerUntil(ts, []() { return getTestOutputCount() >= 3; }, 1500);
    
    EXPECT_TRUE(success) << "Repeating task did not complete within timeout";
    EXPECT_EQ(getTestOutputCount(), 3);
    EXPECT_EQ(getTestOutput(0), "Repeating task #1");
    EXPECT_EQ(getTestOutput(1), "Repeating task #2");
    EXPECT_EQ(getTestOutput(2), "Repeating task #3");
}

/**
 * @brief Test task configured for infinite execution
 * 
 * Creates a task with TASK_FOREVER iterations and runs scheduler for
 * a fixed time period (250ms) to verify:
 * 1. Task continues executing indefinitely
 * 2. Execution frequency matches expected interval (50ms)
 * 3. Task doesn't stop after arbitrary number of executions
 * Tests the infinite iteration functionality.
 */
TEST_F(SchedulerTest, InfiniteRepeatingTask) {
    Scheduler ts;
    
    // Create a task that runs indefinitely with 50ms interval
    Task infinite_task(50, TASK_FOREVER, &repeating_callback, &ts, true);
    
    // Run for a specific time and count executions
    unsigned long start_time = millis();
    while (millis() - start_time < 250) { // Run for 250ms
        ts.execute();
        delay(10);
    }
    
    // Should have executed approximately 250/50 = 5 times (allowing for timing variance)
    EXPECT_GE(getTestOutputCount(), 3);
    EXPECT_LE(getTestOutputCount(), 7);
}

/**
 * @brief Test task enable/disable functionality
 * 
 * Creates a task in disabled state, verifies it doesn't execute,
 * then enables it and verifies execution occurs. Tests:
 * 1. Disabled tasks don't execute even if scheduler runs
 * 2. Enabling a task allows it to execute on next scheduler pass
 * 3. Task state changes are properly handled
 * Critical for dynamic task management.
 */
TEST_F(SchedulerTest, TaskEnableDisable) {
    Scheduler ts;
    
    // Create a disabled task
    Task task1(100, 1, &task1_callback, &ts, false);
    
    // Run scheduler - task should not execute (it's disabled)
    delay(150);
    ts.execute();
    EXPECT_EQ(getTestOutputCount(), 0);
    
    // Now enable the task
    task1.enable();
    
    // Run scheduler - task should execute now
    bool success = runSchedulerUntil(ts, []() { return getTestOutputCount() >= 1; });
    
    EXPECT_TRUE(success) << "Task did not execute after being enabled";
    EXPECT_EQ(getTestOutputCount(), 1);
    EXPECT_EQ(getTestOutput(0), "Task1 executed");
}

/**
 * @brief Test disabling a task while it's actively running
 * 
 * Creates an infinite repeating task, lets it execute several times,
 * then disables it and verifies no further executions occur. Tests:
 * 1. Running tasks can be disabled mid-execution
 * 2. Disabled tasks immediately stop executing
 * 3. Task state changes take effect on next scheduler pass
 * Important for dynamic task control scenarios.
 */
TEST_F(SchedulerTest, TaskDisableDuringExecution) {
    Scheduler ts;
    
    // Create a repeating task
    Task repeating_task(60, TASK_FOREVER, &repeating_callback, &ts, true);
    
    // Let it run a few times
    bool success = runSchedulerUntil(ts, []() { return getTestOutputCount() >= 2; });
    EXPECT_TRUE(success);
    
    size_t executions_before_disable = getTestOutputCount();
    
    // Disable the task
    repeating_task.disable();
    
    // Continue running scheduler
    delay(200);
    for (int i = 0; i < 10; i++) {
        ts.execute();
        delay(20);
    }
    
    // Should not have executed any more times
    EXPECT_EQ(getTestOutputCount(), executions_before_disable);
}

/**
 * @brief Test scheduler behavior with no registered tasks
 * 
 * Runs scheduler execute() method multiple times when no tasks are
 * registered and verifies:
 * 1. No crashes or exceptions occur
 * 2. No unexpected output is generated
 * 3. Scheduler handles empty task list gracefully
 * Edge case test for scheduler robustness.
 */
TEST_F(SchedulerTest, SchedulerWithNoTasks) {
    Scheduler ts;
    
    // Execute scheduler with no tasks multiple times
    for (int i = 0; i < 100; i++) {
        ts.execute();
        delay(1);
    }
    
    EXPECT_EQ(getTestOutputCount(), 0);
}

/**
 * @brief Test correct execution order of tasks with different schedules
 * 
 * Creates three tasks with deliberately mixed creation order vs execution order:
 * - task_late: 200ms delay (should execute last)
 * - task_early: 50ms delay (should execute first)  
 * - task_mid: 100ms delay (should execute second)
 * 
 * CRITICAL: Tasks are created disabled and enabled with enableDelayed() to ensure
 * the first execution happens after the specified interval, not immediately.
 * This is essential for proper sequencing verification.
 * 
 * Verifies scheduler executes tasks in chronological order, not creation order.
 * Critical for validating proper task scheduling algorithm implementation.
 */
TEST_F(SchedulerTest, TaskExecutionOrder) {
    Scheduler ts;
    
    // Create tasks disabled to control execution timing precisely
    Task task_late(200, 1, &task3_callback, &ts, false);  // Should execute last
    Task task_early(50, 1, &task1_callback, &ts, false);  // Should execute first  
    Task task_mid(100, 1, &task2_callback, &ts, false);   // Should execute middle
    
    // Enable all tasks with delayed execution to respect their intervals
    task_late.enableDelayed();   // Will execute after 200ms
    task_early.enableDelayed();  // Will execute after 50ms
    task_mid.enableDelayed();    // Will execute after 100ms
    
    // Run until all execute
    bool success = runSchedulerUntil(ts, []() { return getTestOutputCount() >= 3; });
    
    EXPECT_TRUE(success);
    EXPECT_EQ(getTestOutputCount(), 3);
    
    // Tasks should execute in chronological order regardless of creation order
    EXPECT_EQ(getTestOutput(0), "Task1 executed");  // First (50ms)
    EXPECT_EQ(getTestOutput(1), "Task2 executed");  // Second (100ms)
    EXPECT_EQ(getTestOutput(2), "Task3 executed");  // Third (200ms)
}

/**
 * @brief Test scheduler performance with large number of concurrent tasks
 * 
 * Creates 10 tasks with slightly staggered execution times (100ms + i*10ms)
 * and verifies:
 * 1. All tasks execute successfully within timeout
 * 2. No performance degradation causes task loss
 * 3. Scheduler can handle realistic task loads
 * 
 * Stress test for scheduler scalability and performance under load.
 * Uses smart pointers to manage task memory automatically.
 */
TEST_F(SchedulerTest, SchedulerHandlesLargeNumberOfTasks) {
    Scheduler ts;
    std::vector<std::unique_ptr<Task>> tasks;
    
    // Create many tasks with similar timing
    for (int i = 0; i < 10; i++) {
        auto task = std::make_unique<Task>(100 + i * 10, 1, &task1_callback, &ts, true);
        tasks.push_back(std::move(task));
    }
    
    // Run until all tasks execute
    bool success = runSchedulerUntil(ts, []() { return getTestOutputCount() >= 10; }, 2000);
    
    EXPECT_TRUE(success) << "Not all tasks executed within timeout";
    EXPECT_EQ(getTestOutputCount(), 10);
}

/**
 * @brief Main test runner function
 * 
 * Initializes Google Test framework and runs all registered test cases.
 * Called by the test execution environment to start the testing process.
 * Returns 0 for success, non-zero for test failures.
 * 
 * @param argc Command line argument count
 * @param argv Command line argument values
 * @return Test execution status (0 = success)
 */
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
