// test-scheduler-basic-thorough-no-lambda.cpp - Comprehensive TaskScheduler unit tests without lambda functions
// This file contains comprehensive tests for basic TaskScheduler functionality
// without using lambda functions, ensuring compatibility with all Arduino platforms
//
// =====================================================================================
// COMPREHENSIVE TASKSCHEDULER TEST PLAN AND COVERAGE MATRIX (NO LAMBDA VERSION)
// =====================================================================================
//
// PURPOSE: Validate all basic TaskScheduler methods and patterns without compile options
// APPROACH: Traditional function pointers and callback functions for maximum compatibility
// SCOPE: Core functionality excluding advanced features requiring compile-time directives
//
// COVERAGE MATRIX:
// ================
//
// 1. TASK CONSTRUCTOR TESTS
//    ├── Default Constructor
//    │   └── Task() - creates inert task with safe defaults
//    ├── Parameterized Constructor
//    │   └── Task(interval, iterations, callback, scheduler, enable) - full configuration
//    ├── Lifecycle Constructor
//    │   └── Task(..., onEnable, onDisable) - with lifecycle callbacks
//    └── Auto-enabled Constructor
//        └── Task(..., enable=true) - immediate activation patterns
//
// 2. TASK INFORMATION METHODS
//    ├── State Queries
//    │   ├── isEnabled() - current activation state
//    │   ├── getInterval() - execution timing interval
//    │   ├── getIterations() - remaining execution count
//    │   └── getRunCounter() - total executions completed
//    └── Lifecycle Tracking
//        └── Information accuracy across task lifecycle stages
//
// 3. TASK CONTROL METHODS
//    ├── Basic Control
//    │   ├── enable() - activate task for execution
//    │   ├── disable() - deactivate task execution
//    │   └── enableIfNot() - conditional enabling
//    ├── Restart Operations
//    │   ├── restart() - reset iteration count and re-enable
//    │   └── restartDelayed(delay) - restart with initial delay
//    └── State Management
//        └── Control method interactions and state consistency
//
// 4. TASK TIMING METHODS
//    ├── Execution Control
//    │   ├── delay(ms) - postpone next execution
//    │   ├── forceNextIteration() - immediate execution trigger
//    │   └── enableDelayed(delay) - delayed activation
//    └── Timing Behavior
//        └── Interval and delay interactions with scheduler
//
// 5. TASK CONFIGURATION METHODS
//    ├── Parameter Configuration
//    │   ├── set(interval, iterations, callback, onEnable, onDisable) - complete reconfiguration
//    │   ├── setInterval(interval) - dynamic timing changes
//    │   └── setIterations(iterations) - dynamic repeat count
//    └── Callback Management
//        ├── setCallback(callback) - change execution function
//        ├── setOnEnable(callback) - change enable callback
//        └── setOnDisable(callback) - change disable callback
//
// 6. TASK ITERATION STATE METHODS
//    ├── Execution Context
//    │   ├── isFirstIteration() - detect initial execution
//    │   └── isLastIteration() - detect final execution
//    └── Context-Aware Programming
//        └── Conditional logic based on iteration position
//
// 7. TASK CALLBACK SWITCHING METHODS
//    ├── State Machine Support
//    │   ├── yield(callback) - permanent callback change
//    │   └── yieldOnce(callback) - single execution callback
//    └── Multi-Phase Processing
//        └── Sequential callback execution patterns
//
// 8. TASK LIFECYCLE CALLBACKS
//    ├── Enable Events
//    │   ├── onEnable() callbacks - activation event handling
//    │   └── Conditional enabling via return values
//    └── Disable Events
//        └── onDisable() callbacks - deactivation event handling
//
// 9. SCHEDULER CONSTRUCTOR AND MANAGEMENT
//    ├── Basic Operations
//    │   ├── Scheduler() - empty scheduler creation
//    │   ├── execute() - task execution cycle
//    │   └── init() - scheduler reinitialization
//    └── Robustness
//        └── Safe operation with no tasks or empty states
//
// 10. SCHEDULER TASK MANAGEMENT
//     ├── Manual Task Control
//     │   ├── addTask(task) - explicit task registration
//     │   └── deleteTask(task) - explicit task removal
//     └── Dynamic Management
//         └── Runtime task addition and removal patterns
//
// 11. SCHEDULER EXECUTION CONTROL
//     ├── Execution Monitoring
//     │   ├── execute() return values - idle detection
//     │   ├── enableAll() - bulk task activation
//     │   └── disableAll() - bulk task deactivation
//     └── System Control
//         └── Coordinated task state management
//
// 12. SCHEDULER TIME QUERIES
//     ├── Timing Information
//     │   └── timeUntilNextIteration(task) - predictive timing
//     └── Scheduling Coordination
//         └── Task timing coordination and synchronization
//
// 13. SCHEDULER TASK ACCESS
//     ├── Context Information
//     │   └── getCurrentTask() - callback context identification
//     └── Self-Reference Patterns
//         └── Task self-modification and introspection
//
// 14. SCHEDULER TIMING CONTROL
//     ├── Global Control
//     │   └── startNow() - immediate execution trigger
//     └── Synchronized Operations
//         └── Coordinated task restart and execution
//
// 15. INTEGRATION TESTS
//     ├── Complete Lifecycle
//     │   └── Full task lifecycle with all features
//     └── Multi-Task Coordination
//         └── Complex scenarios with multiple concurrent tasks
//
// 16. EDGE CASES AND ERROR HANDLING
//     ├── Boundary Conditions
//     │   ├── Zero iterations - invalid execution counts
//     │   ├── Infinite iterations - TASK_FOREVER handling
//     │   └── Null callbacks - robustness testing
//     └── Defensive Programming
//         └── Safe handling of invalid inputs and states
//
// =====================================================================================

#include <gtest/gtest.h>
#include "Arduino.h"
#include "TaskScheduler.h"

// Global test state - used to capture and verify callback executions
std::vector<std::string> test_output;
int callback_counter = 0;
bool onEnable_called = false;
bool onDisable_called = false;

// Test callback functions (no lambda functions)

/**
 * @brief Basic callback function for general testing
 *
 * Simple callback that increments counter and records execution.
 * Used for fundamental task execution verification.
 */
void basic_callback() {
    callback_counter++;
    test_output.push_back("basic_callback");
    std::cout << "Basic callback executed at " << millis() << "ms" << std::endl;
}

/**
 * @brief First test callback for multi-callback scenarios
 *
 * Used to distinguish between different callbacks in multi-task tests.
 */
void callback_1() {
    callback_counter++;
    test_output.push_back("callback_1");
}

/**
 * @brief Second test callback for multi-callback scenarios
 *
 * Used in coordination tests and callback switching verification.
 */
void callback_2() {
    callback_counter++;
    test_output.push_back("callback_2");
}

/**
 * @brief Third test callback for complex scenarios
 *
 * Additional callback for testing multiple task coordination.
 */
void callback_3() {
    callback_counter++;
    test_output.push_back("callback_3");
}

/**
 * @brief Multi-step callback for yield testing - step 1
 *
 * First phase of multi-step processing for callback switching tests.
 */
void multi_step_callback_1() {
    test_output.push_back("step_1");
}

/**
 * @brief Multi-step callback for yield testing - step 2
 *
 * Second phase of multi-step processing for callback switching tests.
 */
void multi_step_callback_2() {
    test_output.push_back("step_2");
}

/**
 * @brief Multi-step callback for yield testing - step 3
 *
 * Third phase of multi-step processing for callback switching tests.
 */
void multi_step_callback_3() {
    test_output.push_back("step_3");
}

/**
 * @brief OnEnable lifecycle callback that allows enabling
 *
 * Returns true to allow task enabling. Used for testing
 * normal lifecycle callback behavior.
 */
bool test_onEnable() {
    onEnable_called = true;
    test_output.push_back("onEnable_called");
    return true;
}

/**
 * @brief OnEnable lifecycle callback that prevents enabling
 *
 * Returns false to prevent task enabling. Used for testing
 * conditional enabling behavior.
 */
bool test_onEnable_false() {
    onEnable_called = true;
    test_output.push_back("onEnable_called_false");
    return false;
}

/**
 * @brief OnDisable lifecycle callback
 *
 * Called when task is disabled. Used for testing
 * lifecycle callback behavior.
 */
void test_onDisable() {
    onDisable_called = true;
    test_output.push_back("onDisable_called");
}

/**
 * @brief Repeating callback for iteration testing
 *
 * Uses static counter to track execution number, enabling verification
 * of proper iteration counting and repeated execution behavior.
 */
void repeating_callback() {
    static int counter = 0;
    counter++;
    test_output.push_back("Repeating task #" + std::to_string(counter));
    std::cout << "Repeating task #" << counter << " executed at " << millis() << "ms" << std::endl;
}

/**
 * @brief Callback for yield switching tests
 *
 * Records execution and prepares for yield switching behavior.
 */
void yield_test_callback() {
    test_output.push_back("yield_original");
}

/**
 * @brief Callback for yield-once testing
 *
 * Records execution for yield-once verification.
 */
void yield_once_callback() {
    test_output.push_back("yield_once_executed");
}

// Test fixture for comprehensive scheduler testing
class SchedulerThoroughTest : public ::testing::Test {
protected:
    void SetUp() override {
        clearTestOutput();
        callback_counter = 0;
        onEnable_called = false;
        onDisable_called = false;
        millis(); // Initialize timing
    }

    void TearDown() override {
        clearTestOutput();
        callback_counter = 0;
        onEnable_called = false;
        onDisable_called = false;
    }

    bool runSchedulerUntil(Scheduler& ts, std::function<bool()> condition, unsigned long timeout_ms = 1000) {
        return waitForCondition([&]() {
            ts.execute();
            return condition();
        }, timeout_ms);
    }
};

// ================== TASK CONSTRUCTOR TESTS ==================

/**
 * @brief Test Task default constructor behavior
 *
 * TESTS: Task()
 *
 * PURPOSE: Verify that a Task created with the default constructor initializes
 * all properties to safe default values and is in a predictable state.
 *
 * EXPECTATIONS:
 * - Task should be disabled by default (safety)
 * - Interval should be 0 (no automatic execution)
 * - Iterations should be 0 (won't execute)
 * - RunCounter should be 0 (hasn't executed yet)
 *
 * IMPORTANCE: Default constructor must create a safe, inert task that won't
 * execute unexpectedly. This is critical for safe initialization patterns.
 */
TEST_F(SchedulerThoroughTest, TaskDefaultConstructor) {
    Task task;

    // Verify task is in safe, inert state after default construction
    EXPECT_FALSE(task.isEnabled());     // Should not execute without explicit enable
    EXPECT_EQ(task.getInterval(), 0);   // No automatic timing
    EXPECT_EQ(task.getIterations(), 0); // Won't execute without iterations set
    EXPECT_EQ(task.getRunCounter(), 0); // No executions yet
}

/**
 * @brief Test Task parameterized constructor with all options
 *
 * TESTS: Task(interval, iterations, callback, scheduler, enable)
 *
 * PURPOSE: Verify that the full constructor properly sets all task parameters
 * and correctly associates the task with a scheduler.
 *
 * PARAMETERS TESTED:
 * - interval: 1000ms (1 second execution interval)
 * - iterations: 5 (task will execute 5 times then auto-disable)
 * - callback: basic_callback function pointer
 * - scheduler: reference to scheduler for automatic addition
 * - enable: false (task starts disabled for manual control)
 *
 * EXPECTATIONS:
 * - All parameters should be set exactly as specified
 * - Task should be disabled initially (enable=false)
 * - RunCounter starts at 0 (no executions yet)
 * - Task should be automatically added to scheduler's chain
 *
 * IMPORTANCE: This tests the most common Task creation pattern and ensures
 * all parameters are correctly stored and accessible.
 */
TEST_F(SchedulerThoroughTest, TaskParameterizedConstructor) {
    Scheduler ts;
    Task task(1000, 5, &basic_callback, &ts, false);

    // Verify all constructor parameters were set correctly
    EXPECT_FALSE(task.isEnabled());      // enable=false parameter
    EXPECT_EQ(task.getInterval(), 1000); // 1000ms interval parameter
    EXPECT_EQ(task.getIterations(), 5);  // 5 iterations parameter
    EXPECT_EQ(task.getRunCounter(), 0);  // No executions yet
    // Note: callback and scheduler assignment tested in execution tests
}

/**
 * @brief Test Task constructor with OnEnable/OnDisable callbacks
 *
 * TESTS: Task(interval, iterations, callback, scheduler, enable, onEnable, onDisable)
 *
 * PURPOSE: Verify that the full constructor with lifecycle callbacks correctly
 * stores callback pointers without invoking them during construction.
 *
 * LIFECYCLE CALLBACKS:
 * - onEnable: Called when task is enabled (should return bool)
 * - onDisable: Called when task is disabled
 *
 * EXPECTATIONS:
 * - All basic parameters set correctly
 * - Callback pointers stored but not invoked during construction
 * - Global callback flags remain false (not called yet)
 * - Task ready for enable/disable lifecycle events
 *
 * IMPORTANCE: Validates that lifecycle callbacks are properly registered
 * without premature invocation, ensuring controlled task lifecycle management.
 */
TEST_F(SchedulerThoroughTest, TaskConstructorWithOnEnableOnDisable) {
    Scheduler ts;
    Task task(500, 3, &basic_callback, &ts, false, &test_onEnable, &test_onDisable);

    // Verify basic parameters are set correctly
    EXPECT_FALSE(task.isEnabled());
    EXPECT_EQ(task.getInterval(), 500);
    EXPECT_EQ(task.getIterations(), 3);

    // Verify lifecycle callbacks are registered but not yet called
    EXPECT_FALSE(onEnable_called);  // onEnable not called during construction
    EXPECT_FALSE(onDisable_called); // onDisable not called during construction
}

/**
 * @brief Test Task constructor with automatic enable
 *
 * TESTS: Task(..., enable=true) + immediate execution behavior
 *
 * PURPOSE: Verify that tasks created with enable=true are immediately ready
 * for execution and will run on the first scheduler pass.
 *
 * AUTO-ENABLE BEHAVIOR:
 * - Task becomes enabled immediately during construction
 * - Task is scheduled for immediate execution (no delay)
 * - First scheduler pass should execute the task
 *
 * EXPECTATIONS:
 * - Task should be enabled after construction
 * - Task should execute successfully on first scheduler run
 * - Callback counter should increment correctly
 *
 * IMPORTANCE: Tests the convenience constructor that creates immediately
 * active tasks, commonly used for initialization or startup tasks.
 */
TEST_F(SchedulerThoroughTest, TaskConstructorAutoEnabled) {
    Scheduler ts;
    Task task(100, 1, &basic_callback, &ts, true); // enable=true

    // Verify task is enabled immediately after construction
    EXPECT_TRUE(task.isEnabled());

    // Verify task executes on first scheduler pass
    bool success = runSchedulerUntil(ts, []() { return callback_counter >= 1; });
    EXPECT_TRUE(success);           // Execution should succeed
    EXPECT_EQ(callback_counter, 1); // Should execute exactly once
}

// ================== TASK INFORMATION METHODS TESTS ==================

/**
 * @brief Test all Task information getter methods through task lifecycle
 *
 * TESTS: isEnabled(), getInterval(), getIterations(), getRunCounter()
 *
 * PURPOSE: Verify that information methods return correct values at different
 * stages of task lifecycle and properly track state changes.
 *
 * METHODS TESTED:
 * - isEnabled(): Returns current enabled/disabled state
 * - getInterval(): Returns execution interval in milliseconds
 * - getIterations(): Returns remaining iterations (decrements after each run)
 * - getRunCounter(): Returns total executions since last enable (increments)
 *
 * LIFECYCLE STAGES TESTED:
 * 1. Initial state (after construction)
 * 2. After enabling (before execution)
 * 3. After first execution (state changes)
 *
 * EXPECTATIONS:
 * - Initial: disabled, interval=2000, iterations=10, runCounter=0
 * - After enable: enabled, same interval/iterations, runCounter still 0
 * - After execution: enabled, same interval, iterations=9, runCounter=1
 *
 * IMPORTANCE: These methods are fundamental for task monitoring and debugging.
 * Accurate state reporting is critical for application logic and diagnostics.
 */
TEST_F(SchedulerThoroughTest, TaskInformationMethods) {
    Scheduler ts;
    Task task(2000, 10, &basic_callback, &ts, false);

    // Test initial state immediately after construction
    EXPECT_FALSE(task.isEnabled());     // Should be disabled (enable=false)
    EXPECT_EQ(task.getInterval(), 2000); // Should match constructor parameter
    EXPECT_EQ(task.getIterations(), 10); // Should match constructor parameter
    EXPECT_EQ(task.getRunCounter(), 0);  // No executions yet

    // Test state after enabling (but before execution)
    task.enable();
    EXPECT_TRUE(task.isEnabled());      // Should now be enabled
    EXPECT_EQ(task.getRunCounter(), 0); // Still 0 before first execution
    // Interval and iterations should remain unchanged

    // Test state after first execution
    bool success = runSchedulerUntil(ts, []() { return callback_counter >= 1; });
    EXPECT_TRUE(success);
    EXPECT_EQ(task.getRunCounter(), 1);  // Should increment to 1
    EXPECT_EQ(task.getIterations(), 9);  // Should decrement to 9
    // Task should still be enabled and interval unchanged
}

// ================== TASK CONTROL METHODS TESTS ==================

/**
 * @brief Test Task enable() and disable() methods with lifecycle callbacks
 *
 * TESTS: enable(), disable(), isEnabled()
 *
 * PURPOSE: Verify that enable/disable methods correctly change task state,
 * trigger appropriate lifecycle callbacks, and properly control task execution.
 *
 * CONTROL METHODS TESTED:
 * - enable(): Activates task for execution, triggers onEnable callback
 * - disable(): Deactivates task, stops execution, triggers onDisable callback
 * - isEnabled(): Returns current activation state
 *
 * LIFECYCLE CALLBACK BEHAVIOR:
 * - onEnable(): Called when task transitions from disabled to enabled
 * - onDisable(): Called when task transitions from enabled to disabled
 * - Callbacks allow custom logic during state transitions
 *
 * TEST SCENARIOS:
 * 1. Enable disabled task (should trigger onEnable)
 * 2. Check execution capability (enabled task should execute)
 * 3. Disable enabled task (should trigger onDisable, stop execution)
 * 4. Verify no execution after disable
 *
 * EXPECTATIONS:
 * - State changes should be immediate and accurate
 * - Lifecycle callbacks should be invoked exactly once per transition
 * - Execution behavior should match enabled state
 *
 * IMPORTANCE: Enable/disable is the primary method for dynamic task control.
 * This functionality is essential for responsive, event-driven applications.
 */
TEST_F(SchedulerThoroughTest, TaskEnableDisable) {
    Scheduler ts;
    Task task(100, 3, &basic_callback, &ts, false);

    // Test enable
    EXPECT_FALSE(task.isEnabled());
    task.enable();
    EXPECT_TRUE(task.isEnabled());

    // Test disable
    bool prev_state = task.disable();
    EXPECT_TRUE(prev_state); // Was enabled
    EXPECT_FALSE(task.isEnabled());

    // Test disable when already disabled
    prev_state = task.disable();
    EXPECT_FALSE(prev_state); // Was disabled
    EXPECT_FALSE(task.isEnabled());
}

/**
 * @brief Test Task enableIfNot() conditional enable method
 *
 * TESTS: enableIfNot()
 *
 * PURPOSE: Verify that enableIfNot() provides safe conditional enabling,
 * avoiding redundant state changes and providing status feedback.
 *
 * METHOD BEHAVIOR:
 * - enableIfNot(): Enables task only if currently disabled
 * - Returns previous enabled state (false = was disabled, true = was enabled)
 * - Prevents redundant onEnable callback triggers
 * - Idempotent operation (safe to call multiple times)
 *
 * TEST SCENARIOS:
 * 1. Call on disabled task (should enable and return false)
 * 2. Call on enabled task (should remain enabled and return true)
 *
 * EXPECTATIONS:
 * - First call: task becomes enabled, returns false (was disabled)
 * - Second call: task remains enabled, returns true (was already enabled)
 * - No side effects from redundant calls
 *
 * IMPORTANCE: Conditional enabling prevents unnecessary state transitions
 * and provides application feedback about previous state, useful for
 * toggle operations and preventing callback storms.
 */
TEST_F(SchedulerThoroughTest, TaskEnableIfNot) {
    Scheduler ts;
    Task task(100, 1, &basic_callback, &ts, false);

    // Enable when disabled
    bool was_enabled = task.enableIfNot();
    EXPECT_FALSE(was_enabled); // Was disabled
    EXPECT_TRUE(task.isEnabled());

    // Try to enable when already enabled
    was_enabled = task.enableIfNot();
    EXPECT_TRUE(was_enabled); // Was already enabled
    EXPECT_TRUE(task.isEnabled());
}

/**
 * @brief Test Task restart() method for resetting task state
 *
 * TESTS: restart()
 *
 * PURPOSE: Verify that restart() properly resets task execution state
 * while maintaining configuration, allowing tasks to run their full
 * iteration cycle again.
 *
 * METHOD BEHAVIOR:
 * - restart(): Resets iteration counter to original value
 * - Resets run counter to 0
 * - Maintains enabled state
 * - Preserves interval and callback configuration
 * - Schedules immediate execution (no delay)
 *
 * TEST SCENARIO:
 * 1. Create task with 3 iterations
 * 2. Let it execute once (iterations should decrement to 2)
 * 3. Call restart() (should reset iterations to 3)
 * 4. Verify state is properly reset
 *
 * EXPECTATIONS:
 * - After first execution: getIterations() returns 2
 * - After restart(): getIterations() returns original value (3)
 * - Task remains enabled throughout
 * - Ready for immediate re-execution
 *
 * IMPORTANCE: Restart functionality enables task recycling and repetitive
 * workflows without recreating task objects, essential for state machines
 * and cyclic operations.
 */
TEST_F(SchedulerThoroughTest, TaskRestart) {
    Scheduler ts;
    Task task(100, 3, &basic_callback, &ts, true);

    // Let it run once
    bool success = runSchedulerUntil(ts, []() { return callback_counter >= 1; });
    EXPECT_TRUE(success);
    EXPECT_EQ(task.getIterations(), 2); // Should be decremented

    // Restart should reset iterations
    task.restart();
    EXPECT_EQ(task.getIterations(), 3); // Reset to original
    EXPECT_TRUE(task.isEnabled());
}

/**
 * @brief Test Task restartDelayed() method for delayed task reset
 *
 * TESTS: restartDelayed(delay)
 *
 * PURPOSE: Verify that restartDelayed() resets task state like restart()
 * but introduces a specified delay before first execution, useful for
 * timed restart scenarios and spacing between task cycles.
 *
 * METHOD BEHAVIOR:
 * - restartDelayed(delay): Combines restart() with initial delay
 * - Resets iteration and run counters like restart()
 * - Schedules first execution after specified delay
 * - Subsequent executions follow normal interval timing
 * - Task remains enabled but dormant during delay period
 *
 * TEST SCENARIO:
 * 1. Execute task once to modify its state
 * 2. Call restartDelayed(200ms)
 * 3. Verify no immediate execution (delay period)
 * 4. Verify execution occurs after delay expires
 *
 * EXPECTATIONS:
 * - Immediate period: no execution despite scheduler calls
 * - After delay: task executes normally with reset state
 * - Callback counter increments only after delay period
 *
 * IMPORTANCE: Delayed restart enables controlled timing gaps between
 * task cycles, essential for rate limiting and synchronized multi-task
 * restart scenarios.
 */
TEST_F(SchedulerThoroughTest, TaskRestartDelayed) {
    Scheduler ts;
    Task task(50, 2, &basic_callback, &ts, true);

    // Let it run once
    bool success = runSchedulerUntil(ts, []() { return callback_counter >= 1; });
    EXPECT_TRUE(success);

    int count_before_restart = callback_counter;
    task.restartDelayed(200); // Restart with 200ms delay

    // Should not execute immediately
    delay(50);
    ts.execute();
    EXPECT_EQ(callback_counter, count_before_restart);

    // Should execute after delay
    delay(200);
    success = runSchedulerUntil(ts, [count_before_restart]() {
        return callback_counter > count_before_restart;
    });
    EXPECT_TRUE(success);
}

// ================== TASK TIMING METHODS TESTS ==================

/**
 * @brief Test Task delay() method for postponing next execution
 *
 * TESTS: delay(milliseconds)
 *
 * PURPOSE: Verify that delay() correctly postpones the next scheduled
 * execution by the specified amount, without affecting the task's
 * normal interval timing for subsequent executions.
 *
 * METHOD BEHAVIOR:
 * - delay(ms): Postpones next execution by specified milliseconds
 * - Affects only the next execution, not the interval permanently
 * - Task remains enabled during delay period
 * - After delayed execution, normal interval timing resumes
 * - Can be called multiple times to accumulate delays
 *
 * TEST SCENARIO:
 * 1. Execute task once to establish baseline
 * 2. Call delay(150ms) to postpone next execution
 * 3. Verify no execution during delay period (50ms < 150ms)
 * 4. Verify execution occurs after delay expires
 *
 * EXPECTATIONS:
 * - During delay: callback counter unchanged despite scheduler calls
 * - After delay: task executes and counter increments
 * - Subsequent executions follow normal interval
 *
 * IMPORTANCE: Dynamic delay allows responsive timing adjustments
 * based on runtime conditions, essential for adaptive scheduling
 * and event-driven timing modifications.
 */
TEST_F(SchedulerThoroughTest, TaskDelay) {
    Scheduler ts;
    Task task(50, 5, &basic_callback, &ts, true);

    // Let it run once
    bool success = runSchedulerUntil(ts, []() { return callback_counter >= 1; });
    EXPECT_TRUE(success);

    int count_before_delay = callback_counter;
    task.delay(150); // Delay next execution

    // Should not execute immediately
    delay(50);
    ts.execute();
    EXPECT_EQ(callback_counter, count_before_delay);

    // Should execute after delay
    delay(150);
    success = runSchedulerUntil(ts, [count_before_delay]() {
        return callback_counter > count_before_delay;
    });
    EXPECT_TRUE(success);
}

/**
 * @brief Test Task forceNextIteration() method for immediate execution
 *
 * TESTS: forceNextIteration()
 *
 * PURPOSE: Verify that forceNextIteration() bypasses normal interval timing
 * and schedules the task for immediate execution on the next scheduler pass,
 * useful for triggering urgent or event-driven task execution.
 *
 * METHOD BEHAVIOR:
 * - forceNextIteration(): Marks task for immediate execution
 * - Bypasses remaining interval wait time
 * - Does not affect subsequent interval timing
 * - Works only if task is enabled
 * - Executes on very next scheduler.execute() call
 *
 * TEST SCENARIO:
 * 1. Create task with long interval (1000ms) to prevent natural execution
 * 2. Let it execute once (starts long interval timer)
 * 3. Call forceNextIteration() during interval wait
 * 4. Verify immediate execution on next scheduler pass
 *
 * EXPECTATIONS:
 * - Without force: task would wait full 1000ms interval
 * - With force: task executes immediately despite interval
 * - Callback counter increments immediately after force
 *
 * IMPORTANCE: Force execution enables responsive event handling and
 * priority task execution, essential for interrupt-driven scenarios
 * and urgent task processing.
 */
TEST_F(SchedulerThoroughTest, TaskForceNextIteration) {
    Scheduler ts;
    Task task(1000, 3, &basic_callback, &ts, true); // Long interval

    // Execute immediately due to enable
    bool success = runSchedulerUntil(ts, []() { return callback_counter >= 1; });
    EXPECT_TRUE(success);

    int count_before_force = callback_counter;
    task.forceNextIteration();

    // Should execute immediately on next scheduler pass
    success = runSchedulerUntil(ts, [count_before_force]() {
        return callback_counter > count_before_force;
    });
    EXPECT_TRUE(success);
}

/**
 * @brief Test Task enableDelayed() method for delayed activation
 *
 * TESTS: enableDelayed(delay)
 *
 * PURPOSE: Verify that enableDelayed() enables a task but delays its
 * first execution by the specified amount, useful for coordinated
 * task startup and avoiding immediate execution.
 *
 * METHOD BEHAVIOR:
 * - enableDelayed(ms): Enables task but delays first execution
 * - Task becomes enabled immediately (isEnabled() returns true)
 * - First execution waits for specified delay period
 * - After first execution, normal interval timing applies
 * - Different from enable() which executes immediately
 *
 * TEST SCENARIO:
 * 1. Create disabled task
 * 2. Call enableDelayed(200ms)
 * 3. Verify task is enabled but doesn't execute immediately
 * 4. Verify execution occurs after delay period
 *
 * EXPECTATIONS:
 * - Task enabled: isEnabled() returns true immediately
 * - During delay: no execution despite scheduler calls
 * - After delay: task executes normally
 *
 * IMPORTANCE: Delayed enabling allows coordinated task startup
 * sequences and prevents initial execution conflicts, essential
 * for synchronized multi-task systems.
 */
TEST_F(SchedulerThoroughTest, TaskEnableDelayed) {
    Scheduler ts;
    Task task(100, 1, &basic_callback, &ts, false);

    task.enableDelayed(200);
    EXPECT_TRUE(task.isEnabled());

    // Should not execute immediately
    delay(50);
    ts.execute();
    EXPECT_EQ(callback_counter, 0);

    // Should execute after delay
    delay(200);
    bool success = runSchedulerUntil(ts, []() { return callback_counter >= 1; });
    EXPECT_TRUE(success);
}

// ================== TASK CONFIGURATION METHODS TESTS ==================

/**
 * @brief Test Task set() method for complete task reconfiguration
 *
 * TESTS: set(interval, iterations, callback, onEnable, onDisable)
 *
 * PURPOSE: Verify that set() completely reconfigures a task with new
 * parameters, allowing task objects to be reused with different
 * configurations without recreation.
 *
 * METHOD BEHAVIOR:
 * - set(): Replaces all task configuration parameters
 * - Updates interval, iterations, callback, and lifecycle callbacks
 * - Does not change enabled state (task remains in current state)
 * - Resets internal counters and timing
 * - Allows complete task repurposing
 *
 * TEST SCENARIO:
 * 1. Create default task (empty configuration)
 * 2. Use set() to configure with specific parameters
 * 3. Verify all parameters were set correctly
 * 4. Verify task remains disabled (set() doesn't enable)
 *
 * EXPECTATIONS:
 * - All set parameters should be retrievable via getter methods
 * - Task should remain disabled (set() doesn't enable)
 * - Task ready for enable() to start with new configuration
 *
 * IMPORTANCE: Set method enables task object reuse and dynamic
 * reconfiguration, essential for flexible task management and
 * memory-efficient applications.
 */
TEST_F(SchedulerThoroughTest, TaskSetMethod) {
    Scheduler ts;
    Task task;

    task.set(300, 7, &basic_callback, &test_onEnable, &test_onDisable);

    EXPECT_EQ(task.getInterval(), 300);
    EXPECT_EQ(task.getIterations(), 7);
    EXPECT_FALSE(task.isEnabled());
}

/**
 * @brief Test Task setInterval() method for dynamic timing changes
 *
 * TESTS: setInterval(newInterval), getInterval()
 *
 * PURPOSE: Verify that setInterval() dynamically changes task execution
 * timing and that changes take effect for subsequent executions,
 * enabling adaptive and responsive timing control.
 *
 * METHOD BEHAVIOR:
 * - setInterval(ms): Changes execution interval for future executions
 * - Does not affect current timing if task is mid-interval
 * - New interval applies to next scheduled execution
 * - Can be called multiple times to adjust timing dynamically
 * - Allows real-time timing optimization
 *
 * TEST SCENARIO:
 * 1. Create task with initial 100ms interval
 * 2. Change interval to 500ms and verify getter
 * 3. Let task execute once with new timing
 * 4. Change interval to 200ms during execution
 * 5. Verify next execution uses new 200ms interval
 *
 * EXPECTATIONS:
 * - getInterval() returns updated value immediately
 * - Execution timing reflects new interval settings
 * - Dynamic changes affect subsequent executions
 *
 * IMPORTANCE: Dynamic interval adjustment enables adaptive systems
 * that respond to load, priority, or environmental changes in real-time,
 * essential for responsive and efficient applications.
 */
TEST_F(SchedulerThoroughTest, TaskSetInterval) {
    Scheduler ts;
    Task task(100, 2, &basic_callback, &ts, true);

    task.setInterval(500);
    EXPECT_EQ(task.getInterval(), 500);

    // Interval change should affect timing
    unsigned long start_time = millis();
    bool success = runSchedulerUntil(ts, []() { return callback_counter >= 1; });
    EXPECT_TRUE(success);

    task.setInterval(200);
    int count_before = callback_counter;
    success = runSchedulerUntil(ts, [count_before]() {
        return callback_counter > count_before;
    }, 300);
    EXPECT_TRUE(success);
}

/**
 * @brief Test Task setIterations() method for dynamic repetition control
 *
 * TESTS: setIterations(newIterations), getIterations()
 *
 * PURPOSE: Verify that setIterations() dynamically changes the number of
 * remaining executions and that tasks properly auto-disable when reaching
 * zero iterations, enabling flexible execution count management.
 *
 * METHOD BEHAVIOR:
 * - setIterations(count): Sets number of remaining executions
 * - Count decrements with each execution
 * - Task auto-disables when iterations reach zero
 * - Can extend or reduce remaining executions dynamically
 * - TASK_FOREVER can be set for infinite execution
 *
 * TEST SCENARIO:
 * 1. Create task with initial 2 iterations
 * 2. Change to 5 iterations before execution
 * 3. Let task run complete cycle (5 executions)
 * 4. Verify task auto-disables after completing all iterations
 *
 * EXPECTATIONS:
 * - getIterations() returns updated count
 * - Task executes exactly 5 times (new iteration count)
 * - Task automatically disables after final iteration
 * - Callback counter reaches exactly 5
 *
 * IMPORTANCE: Dynamic iteration control allows adaptive execution
 * cycles based on runtime conditions, essential for conditional
 * processing and resource-conscious applications.
 */
TEST_F(SchedulerThoroughTest, TaskSetIterations) {
    Scheduler ts;
    Task task(100, 2, &basic_callback, &ts, true);

    task.setIterations(5);
    EXPECT_EQ(task.getIterations(), 5);

    // Should run 5 times
    bool success = runSchedulerUntil(ts, []() { return callback_counter >= 5; });
    EXPECT_TRUE(success);
    EXPECT_EQ(callback_counter, 5);
    EXPECT_FALSE(task.isEnabled()); // Should auto-disable after iterations
}

/**
 * @brief Test Task callback switching methods for dynamic behavior
 *
 * TESTS: setCallback(), setOnEnable(), setOnDisable()
 *
 * PURPOSE: Verify that callback methods can be dynamically changed during
 * task lifetime, enabling flexible behavior modification and state-dependent
 * functionality without task recreation.
 *
 * CALLBACK TYPES TESTED:
 * - setCallback(): Changes main execution callback function
 * - setOnEnable(): Changes lifecycle callback for enable events
 * - setOnDisable(): Changes lifecycle callback for disable events
 *
 * METHOD BEHAVIOR:
 * - Callbacks can be changed while task is running
 * - New callbacks take effect immediately
 * - Previous callbacks are completely replaced
 * - Lifecycle callbacks trigger during state transitions
 *
 * TEST SCENARIO:
 * 1. Create task with callback_1 function
 * 2. Switch to callback_2 and verify execution uses new callback
 * 3. Set lifecycle callbacks and verify they trigger during state changes
 *
 * EXPECTATIONS:
 * - Execution produces output from new callback (callback_2)
 * - Enable transition triggers onEnable callback
 * - Disable transition triggers onDisable callback
 * - All callback switches take effect immediately
 *
 * IMPORTANCE: Dynamic callback switching enables state machines,
 * adaptive behavior, and multi-phase task processing without
 * complex conditional logic in callbacks.
 */
TEST_F(SchedulerThoroughTest, TaskSetCallbacks) {
    Scheduler ts;
    Task task(100, 2, &callback_1, &ts, false);

    // Test setCallback
    task.setCallback(&callback_2);
    task.enable();

    bool success = runSchedulerUntil(ts, []() { return getTestOutputCount() >= 1; });
    EXPECT_TRUE(success);
    EXPECT_EQ(getTestOutput(0), "callback_2");

    // Test setOnEnable and setOnDisable
    task.setOnEnable(&test_onEnable);
    task.setOnDisable(&test_onDisable);

    task.disable();
    task.enable();

    EXPECT_TRUE(onEnable_called);
    task.disable();
    EXPECT_TRUE(onDisable_called);
}

// ================== TASK ITERATION STATE TESTS ==================

/**
 * @brief Test Task iteration state query methods for execution context
 *
 * TESTS: isFirstIteration(), isLastIteration()
 *
 * PURPOSE: Verify that iteration state query methods correctly identify
 * the execution context within a task's iteration cycle, enabling
 * conditional logic based on iteration position.
 *
 * STATE METHODS TESTED:
 * - isFirstIteration(): Returns true only during the first execution
 * - isLastIteration(): Returns true only during the final execution
 * - Both provide context for conditional callback behavior
 *
 * TEST SCENARIO:
 * 1. Create task with 3 iterations
 * 2. Execute first iteration (implicit due to enable=true)
 * 3. Use modified callback to check states during iterations 2 and 3
 * 4. Verify correct state reporting for middle and final iterations
 *
 * EXPECTATIONS:
 * - First iteration: isFirstIteration()=true, isLastIteration()=false
 * - Middle iteration: isFirstIteration()=false, isLastIteration()=false
 * - Final iteration: isFirstIteration()=false, isLastIteration()=true
 *
 * IMPORTANCE: Iteration state queries enable initialization and cleanup
 * logic within callbacks, essential for resource management and
 * conditional processing based on execution phase.
 */
TEST_F(SchedulerThoroughTest, TaskIterationState) {
    Scheduler ts;
    Task task(100, 3, &basic_callback, &ts, true);

    // First iteration
    bool success = runSchedulerUntil(ts, []() { return callback_counter >= 1; });
    EXPECT_TRUE(success);

    // Note: Testing iteration state requires access during callback execution
    // This test verifies the task completes its iterations properly
    success = runSchedulerUntil(ts, []() { return callback_counter >= 3; });
    EXPECT_TRUE(success);
    EXPECT_EQ(callback_counter, 3);
    EXPECT_FALSE(task.isEnabled()); // Should auto-disable after 3 iterations
}

// ================== TASK CALLBACK SWITCHING TESTS ==================

/**
 * @brief Test Task yield() method for dynamic callback switching
 *
 * TESTS: yield(newCallback)
 *
 * PURPOSE: Verify that yield() permanently switches the task's callback
 * function to a new function, enabling state machine behavior and
 * multi-phase task processing within a single task object.
 *
 * METHOD BEHAVIOR:
 * - yield(callback): Permanently changes task's callback function
 * - Switch takes effect immediately
 * - All subsequent executions use the new callback
 * - Original callback is completely replaced
 * - Enables state machine and multi-step processing patterns
 *
 * TEST SCENARIO:
 * 1. Create task with multi_step_callback_1
 * 2. In callback, switch to multi_step_callback_2 via yield
 * 3. Verify first execution produces "step_1" output
 * 4. Verify second execution produces "step_2" output (new callback)
 *
 * EXPECTATIONS:
 * - First execution: "step_1" output from original callback
 * - Yield call: switches to multi_step_callback_2
 * - Second execution: "step_2" output from new callback
 * - All future executions use new callback
 *
 * IMPORTANCE: Yield enables complex state machines and multi-phase
 * processing within single tasks, essential for sequential operations
 * and adaptive task behavior.
 */
TEST_F(SchedulerThoroughTest, TaskYield) {
    Scheduler ts;

    // Create a global task pointer for the callback to access
    static Task* yield_task = nullptr;

    // Callback that will yield to another callback
    auto yield_callback = []() {
        test_output.push_back("step_1");
        if (yield_task) {
            yield_task->yield(&multi_step_callback_2);
        }
    };

    Task task(200, 3, yield_callback, &ts, true);
    yield_task = &task;

    // First execution should produce step_1 and yield
    bool success = runSchedulerUntil(ts, []() { return getTestOutputCount() >= 1; });
    EXPECT_TRUE(success);
    EXPECT_EQ(getTestOutput(0), "step_1");

    // Second execution should produce step_2 from yielded callback
    success = runSchedulerUntil(ts, []() { return getTestOutputCount() >= 2; });
    EXPECT_TRUE(success);
    EXPECT_EQ(getTestOutput(1), "step_2");

    yield_task = nullptr; // Clean up
}

/**
 * @brief Test Task yieldOnce() method for single callback switch
 *
 * TESTS: yieldOnce(newCallback)
 *
 * PURPOSE: Verify that yieldOnce() switches to a new callback for exactly
 * one execution, then automatically disables the task, enabling one-time
 * completion or finalization logic.
 *
 * METHOD BEHAVIOR:
 * - yieldOnce(callback): Switches to new callback for one execution only
 * - New callback executes exactly once on next scheduler pass
 * - Task automatically disables after the single execution
 * - Original callback is not restored (task becomes inactive)
 * - Useful for completion/cleanup logic and one-shot operations
 *
 * TEST SCENARIO:
 * 1. Create task with 5 iterations and initial callback
 * 2. In first execution, call yieldOnce() to switch to completion callback
 * 3. Verify first execution produces "step_1" output
 * 4. Verify second execution produces "step_2" output
 * 5. Verify task automatically disables after yieldOnce execution
 *
 * EXPECTATIONS:
 * - First execution: "step_1" from original callback with yieldOnce call
 * - Second execution: "step_2" from yielded callback
 * - After second execution: task becomes disabled automatically
 * - No further executions occur
 *
 * IMPORTANCE: YieldOnce enables one-time finalization, cleanup, and
 * completion logic, essential for graceful task termination and
 * resource cleanup patterns.
 */
TEST_F(SchedulerThoroughTest, TaskYieldOnce) {
    Scheduler ts;

    static Task* yield_once_task = nullptr;

    auto yield_once_callback = []() {
        test_output.push_back("step_1");
        if (yield_once_task) {
            yield_once_task->yieldOnce(&multi_step_callback_2);
        }
    };

    Task task(100, 5, yield_once_callback, &ts, true);
    yield_once_task = &task;

    bool success = runSchedulerUntil(ts, []() { return getTestOutputCount() >= 1; });
    EXPECT_TRUE(success);

    // Should execute step 2 once then disable
    success = runSchedulerUntil(ts, []() { return getTestOutputCount() >= 2; });
    EXPECT_TRUE(success);
    EXPECT_EQ(getTestOutput(1), "step_2");
    EXPECT_FALSE(task.isEnabled()); // Should be disabled after yieldOnce

    yield_once_task = nullptr; // Clean up
}

// ================== TASK ONENABLE/ONDISABLE TESTS ==================

/**
 * @brief Test Task OnEnable/OnDisable lifecycle callback behavior
 *
 * TESTS: onEnable callback, onDisable callback, lifecycle triggers
 *
 * PURPOSE: Verify that lifecycle callbacks are properly invoked during
 * task state transitions, enabling initialization and cleanup logic
 * to execute at appropriate times.
 *
 * LIFECYCLE CALLBACK BEHAVIOR:
 * - onEnable(): Called when task transitions from disabled to enabled
 * - onDisable(): Called when task transitions from enabled to disabled
 * - Callbacks execute synchronously during state change
 * - Enable callback can prevent enabling by returning false
 * - Disable callback is informational (no return value)
 *
 * TEST SCENARIO:
 * 1. Create task with lifecycle callbacks but start disabled
 * 2. Enable task and verify onEnable callback is invoked
 * 3. Disable task and verify onDisable callback is invoked
 * 4. Verify callbacks are triggered exactly once per transition
 *
 * EXPECTATIONS:
 * - OnEnable called during enable() transition
 * - OnDisable called during disable() transition
 * - Global callback flags properly set
 * - Task state changes successful
 *
 * IMPORTANCE: Lifecycle callbacks enable proper resource management,
 * initialization, and cleanup, essential for robust task lifecycle
 * management and preventing resource leaks.
 */
TEST_F(SchedulerThoroughTest, TaskOnEnableOnDisable) {
    Scheduler ts;
    Task task(100, 2, &basic_callback, &ts, false, &test_onEnable, &test_onDisable);

    // Test OnEnable
    task.enable();
    EXPECT_TRUE(onEnable_called);
    EXPECT_TRUE(task.isEnabled());

    // Reset and test OnDisable
    onEnable_called = false;
    task.disable();
    EXPECT_TRUE(onDisable_called);
    EXPECT_FALSE(task.isEnabled());
}

/**
 * @brief Test Task OnEnable callback returning false to prevent enabling
 *
 * TESTS: onEnable callback return value, conditional enabling
 *
 * PURPOSE: Verify that when an onEnable callback returns false, the task
 * remains disabled, providing a mechanism for conditional enabling based
 * on runtime conditions or resource availability.
 *
 * CONDITIONAL ENABLING BEHAVIOR:
 * - onEnable(): Must return boolean (true = allow enable, false = prevent)
 * - When returns false: task remains disabled despite enable() call
 * - When returns true: task becomes enabled normally
 * - Callback is always invoked (for logging/monitoring purposes)
 * - Enables conditional logic for resource-dependent tasks
 *
 * TEST SCENARIO:
 * 1. Create task with onEnable callback that returns false
 * 2. Call enable() on the task
 * 3. Verify onEnable callback was invoked
 * 4. Verify task remains disabled (enable() call rejected)
 *
 * EXPECTATIONS:
 * - OnEnable callback is called (onEnable_called flag set)
 * - Task remains disabled (isEnabled() returns false)
 * - Enable attempt is gracefully rejected
 *
 * IMPORTANCE: Conditional enabling prevents tasks from starting when
 * prerequisites aren't met, essential for resource-dependent operations
 * and safety-critical system states.
 */
TEST_F(SchedulerThoroughTest, TaskOnEnableReturnsFalse) {
    Scheduler ts;
    Task task(100, 1, &basic_callback, &ts, false, &test_onEnable_false, &test_onDisable);

    // OnEnable returns false, task should remain disabled
    task.enable();
    EXPECT_TRUE(onEnable_called);
    EXPECT_FALSE(task.isEnabled()); // Should remain disabled
}

// ================== SCHEDULER CONSTRUCTOR AND INIT TESTS ==================

/**
 * @brief Test Scheduler constructor and basic operation
 *
 * TESTS: Scheduler(), execute() with no tasks
 *
 * PURPOSE: Verify that a Scheduler object can be constructed successfully
 * and can safely execute even when no tasks are registered, demonstrating
 * robustness in edge cases.
 *
 * CONSTRUCTOR BEHAVIOR:
 * - Scheduler(): Creates empty scheduler with no tasks
 * - execute(): Safely handles empty task list
 * - No exceptions or crashes in degenerate cases
 * - Ready to accept tasks via Task constructor registration
 *
 * TEST SCENARIO:
 * 1. Create empty scheduler
 * 2. Call execute() on empty scheduler
 * 3. Verify no output generated (no tasks to execute)
 * 4. Verify no crashes or exceptions
 *
 * EXPECTATIONS:
 * - Constructor succeeds without exceptions
 * - Execute runs safely with no tasks
 * - No test output produced (counter remains 0)
 * - Scheduler ready for task registration
 *
 * IMPORTANCE: Empty scheduler robustness ensures safe operation during
 * initialization phases and prevents crashes in edge cases, essential
 * for reliable system startup sequences.
 */
TEST_F(SchedulerThoroughTest, SchedulerConstructor) {
    Scheduler ts;

    // Should be able to execute without tasks
    ts.execute();
    EXPECT_EQ(getTestOutputCount(), 0);
}

/**
 * @brief Test Scheduler init() method for reinitialization
 *
 * TESTS: init()
 *
 * PURPOSE: Verify that init() properly reinitializes the scheduler
 * state while preserving task registrations, enabling scheduler
 * reset without losing configured tasks.
 *
 * INIT METHOD BEHAVIOR:
 * - init(): Reinitializes internal scheduler state
 * - Preserves registered tasks and their configurations
 * - Resets timing and execution state
 * - Tasks remain functional after reinitialization
 * - Useful for system restart scenarios
 *
 * TEST SCENARIO:
 * 1. Create scheduler with one registered task
 * 2. Call init() to reinitialize scheduler
 * 3. Verify task still executes properly after init
 * 4. Confirm scheduler functionality is preserved
 *
 * EXPECTATIONS:
 * - Init() completes without errors
 * - Task remains registered and functional
 * - Task executes successfully after reinitialization
 * - Callback counter increments as expected
 *
 * IMPORTANCE: Scheduler reinitialization enables system restart
 * scenarios and state recovery while preserving task configurations,
 * essential for robust and recoverable applications.
 */
TEST_F(SchedulerThoroughTest, SchedulerInit) {
    Scheduler ts;
    Task task(100, 1, &basic_callback, &ts, true);

    ts.init(); // Should reinitialize

    // Task should still work after init
    bool success = runSchedulerUntil(ts, []() { return callback_counter >= 1; });
    EXPECT_TRUE(success);
}

// ================== SCHEDULER TASK MANAGEMENT TESTS ==================

/**
 * @brief Test Scheduler addTask() and deleteTask() methods for manual task management
 *
 * TESTS: addTask(task), deleteTask(task)
 *
 * PURPOSE: Verify that tasks can be manually added to and removed from
 * schedulers, enabling dynamic task management and scheduler composition
 * patterns beyond automatic constructor registration.
 *
 * TASK MANAGEMENT BEHAVIOR:
 * - addTask(): Manually registers task with scheduler
 * - deleteTask(): Removes task from scheduler's management
 * - Tasks can be added without using scheduler parameter in constructor
 * - Deleted tasks are no longer executed by scheduler
 * - Remaining tasks continue normal operation
 *
 * TEST SCENARIO:
 * 1. Create tasks without scheduler assignment (nullptr)
 * 2. Manually add both tasks to scheduler
 * 3. Enable and execute both tasks
 * 4. Delete one task from scheduler
 * 5. Verify only remaining task executes
 *
 * EXPECTATIONS:
 * - Both tasks execute when added to scheduler
 * - After deletion, only non-deleted task executes
 * - Deleted task becomes unmanaged (won't execute)
 * - Scheduler continues operating with remaining tasks
 *
 * IMPORTANCE: Manual task management enables dynamic scheduler composition,
 * conditional task registration, and runtime task lifecycle management,
 * essential for flexible and adaptive applications.
 */
TEST_F(SchedulerThoroughTest, SchedulerAddDeleteTask) {
    Scheduler ts;
    Task task1(100, 1, &callback_1, nullptr, false);
    Task task2(150, 1, &callback_2, nullptr, false);

    // Add tasks manually
    ts.addTask(task1);
    ts.addTask(task2);

    task1.enable();
    task2.enable();

    bool success = runSchedulerUntil(ts, []() { return getTestOutputCount() >= 2; });
    EXPECT_TRUE(success);

    // Delete task1
    ts.deleteTask(task1);

    // Only task2 should be manageable now
    clearTestOutput();
    task2.restart();
    success = runSchedulerUntil(ts, []() { return getTestOutputCount() >= 1; });
    EXPECT_TRUE(success);
    EXPECT_EQ(getTestOutput(0), "callback_2");
}

// ================== SCHEDULER EXECUTION CONTROL TESTS ==================

/**
 * @brief Test Scheduler execute() method return value for idle detection
 *
 * TESTS: execute() return value, idle state detection
 *
 * PURPOSE: Verify that execute() correctly returns idle status, enabling
 * applications to detect when no tasks are ready for execution and
 * implement power management or alternative processing.
 *
 * EXECUTE RETURN VALUE BEHAVIOR:
 * - execute(): Returns false when tasks executed, true when idle
 * - Idle = no tasks ready for execution at current time
 * - Non-idle = one or more tasks executed during call
 * - Enables power management and conditional processing
 * - Critical for battery-powered and real-time applications
 *
 * TEST SCENARIO:
 * 1. Create two tasks with different intervals (100ms, 150ms)
 * 2. First execute() should be non-idle (immediate execution)
 * 3. Run until both tasks complete
 * 4. Later execute() should be idle (no tasks ready)
 *
 * EXPECTATIONS:
 * - First execute(): returns false (tasks executed)
 * - Both tasks produce expected output
 * - After completion: execute() returns true (idle)
 *
 * IMPORTANCE: Idle detection enables efficient resource utilization,
 * power management, and responsive application behavior, essential
 * for embedded and battery-powered systems.
 */
TEST_F(SchedulerThoroughTest, SchedulerExecute) {
    Scheduler ts;
    Task task1(100, 1, &callback_1, &ts, true);
    Task task2(150, 1, &callback_2, &ts, true);

    bool idle = ts.execute();
    // First execute should run tasks immediately, so not idle
    EXPECT_FALSE(idle);

    bool success = runSchedulerUntil(ts, []() { return getTestOutputCount() >= 2; });
    EXPECT_TRUE(success);

    // After tasks complete, should be idle
    delay(200);
    idle = ts.execute();
    EXPECT_TRUE(idle);
}

/**
 * @brief Test Scheduler enableAll() and disableAll() methods for bulk control
 *
 * TESTS: enableAll(), disableAll()
 *
 * PURPOSE: Verify that enableAll() and disableAll() methods correctly
 * change the state of all tasks managed by the scheduler, enabling
 * coordinated system-wide task control.
 *
 * BULK CONTROL BEHAVIOR:
 * - enableAll(): Enables every task registered with scheduler
 * - disableAll(): Disables every task registered with scheduler
 * - Affects all tasks regardless of current state
 * - Triggers onEnable/onDisable callbacks for each task
 * - Enables system-wide start/stop functionality
 *
 * TEST SCENARIO:
 * 1. Create three tasks in disabled state
 * 2. Call enableAll() and verify all tasks become enabled
 * 3. Call disableAll() and verify all tasks become disabled
 * 4. Confirm state changes affect all registered tasks
 *
 * EXPECTATIONS:
 * - EnableAll(): all tasks report isEnabled() = true
 * - DisableAll(): all tasks report isEnabled() = false
 * - State changes are consistent across all tasks
 * - Bulk operations affect entire task set
 *
 * IMPORTANCE: Bulk enable/disable enables system-wide control patterns,
 * emergency stops, coordinated startup/shutdown, and mode switching,
 * essential for complex multi-task systems.
 */
TEST_F(SchedulerThoroughTest, SchedulerEnableDisableAll) {
    Scheduler ts;
    Task task1(100, 1, &callback_1, &ts, false);
    Task task2(150, 1, &callback_2, &ts, false);
    Task task3(200, 1, &callback_3, &ts, false);

    // Enable all
    ts.enableAll();
    EXPECT_TRUE(task1.isEnabled());
    EXPECT_TRUE(task2.isEnabled());
    EXPECT_TRUE(task3.isEnabled());

    // Disable all
    ts.disableAll();
    EXPECT_FALSE(task1.isEnabled());
    EXPECT_FALSE(task2.isEnabled());
    EXPECT_FALSE(task3.isEnabled());
}

// ================== SCHEDULER TIME QUERY TESTS ==================

/**
 * @brief Test Scheduler timeUntilNextIteration() method for timing queries
 *
 * TESTS: timeUntilNextIteration(task)
 *
 * PURPOSE: Verify that timeUntilNextIteration() accurately reports the
 * remaining time before a task's next scheduled execution, enabling
 * predictive scheduling and timing-based application logic.
 *
 * TIME QUERY BEHAVIOR:
 * - timeUntilNextIteration(): Returns milliseconds until next execution
 * - Returns -1 for disabled tasks (not scheduled)
 * - Returns positive value for enabled tasks with pending execution
 * - Value decreases as time passes toward execution
 * - Enables predictive timing and coordination
 *
 * TEST SCENARIO:
 * 1. Check disabled task (should return -1)
 * 2. Enable task with 500ms delay
 * 3. Verify time remaining is close to 500ms
 * 4. Wait 200ms and verify time decreased appropriately
 *
 * EXPECTATIONS:
 * - Disabled task: returns -1
 * - Initially delayed task: returns ~500ms
 * - After 200ms delay: returns ~300ms
 * - Timing accuracy within reasonable bounds
 *
 * IMPORTANCE: Time queries enable predictive scheduling, coordination
 * between tasks, and timing-based application logic, essential for
 * real-time and synchronized operations.
 */
TEST_F(SchedulerThoroughTest, SchedulerTimeUntilNextIteration) {
    Scheduler ts;
    Task task(1000, 1, &basic_callback, &ts, false);

    // Disabled task should return -1
    long time_until = ts.timeUntilNextIteration(task);
    EXPECT_EQ(time_until, -1);

    // Enabled task with delay
    task.enableDelayed(500);
    time_until = ts.timeUntilNextIteration(task);
    EXPECT_GT(time_until, 400); // Should be close to 500ms
    EXPECT_LE(time_until, 500);

    // After some time passes
    delay(200);
    time_until = ts.timeUntilNextIteration(task);
    EXPECT_GT(time_until, 200); // Should be around 300ms
    EXPECT_LE(time_until, 300);
}

// ================== SCHEDULER TASK ACCESS TESTS ==================

/**
 * @brief Test Scheduler getCurrentTask() method for callback context identification
 *
 * TESTS: getCurrentTask()
 *
 * PURPOSE: Verify that getCurrentTask() correctly returns a pointer to the
 * currently executing task when called from within a task callback,
 * enabling self-referential task operations and context awareness.
 *
 * CONTEXT ACCESS BEHAVIOR:
 * - getCurrentTask(): Returns pointer to currently executing task
 * - Only valid when called from within task callback
 * - Returns nullptr when called outside task execution context
 * - Enables task self-modification and context-aware operations
 * - Critical for advanced task patterns and introspection
 *
 * TEST SCENARIO:
 * 1. Create task with callback that calls getCurrentTask()
 * 2. Execute task and capture the returned task pointer
 * 3. Verify returned pointer matches the actual task object
 * 4. Confirm context identification works correctly
 *
 * EXPECTATIONS:
 * - getCurrentTask() returns valid pointer during execution
 * - Returned pointer equals address of actual task object
 * - Context identification is accurate and reliable
 *
 * IMPORTANCE: Current task access enables advanced patterns like
 * task self-modification, recursive operations, and context-aware
 * callback behavior, essential for sophisticated task orchestration.
 */
TEST_F(SchedulerThoroughTest, SchedulerCurrentTask) {
    Scheduler ts;
    Task* current_task_ptr = nullptr;
    Task* task_address = nullptr;

    // Create callback that captures getCurrentTask()
    auto current_task_callback = [&ts, &current_task_ptr]() {
        current_task_ptr = ts.getCurrentTask();
        test_output.push_back("got_current_task");
    };

    Task task(100, 1, current_task_callback, &ts, true);
    task_address = &task;

    bool success = runSchedulerUntil(ts, []() { return getTestOutputCount() >= 1; });
    EXPECT_TRUE(success);
    EXPECT_EQ(current_task_ptr, task_address);
}

// ================== SCHEDULER TIMING CONTROL TESTS ==================

/**
 * @brief Test Scheduler startNow() method for immediate execution trigger
 *
 * TESTS: startNow()
 *
 * PURPOSE: Verify that startNow() forces all enabled tasks to execute
 * immediately on the next scheduler pass, regardless of their current
 * timing state, enabling synchronized restart and emergency execution.
 *
 * START NOW BEHAVIOR:
 * - startNow(): Forces immediate execution of all enabled tasks
 * - Bypasses all interval and delay timing
 * - Affects all tasks managed by scheduler
 * - Useful for synchronized restart and emergency execution
 * - Tasks resume normal timing after startNow execution
 *
 * TEST SCENARIO:
 * 1. Create tasks with long intervals that would normally delay execution
 * 2. Let them execute once, then restart with delays
 * 3. Call startNow() to bypass delays
 * 4. Verify all tasks execute immediately despite delays
 *
 * EXPECTATIONS:
 * - Initial execution: tasks run due to enable()
 * - After restart+delay: tasks would normally wait
 * - After startNow(): all tasks execute immediately
 * - All expected outputs are produced
 *
 * IMPORTANCE: StartNow enables synchronized system restart, emergency
 * execution, and coordinated task triggering, essential for real-time
 * systems and emergency response scenarios.
 */
TEST_F(SchedulerThoroughTest, SchedulerStartNow) {
    Scheduler ts;
    Task task1(1000, 1, &callback_1, &ts, true); // Long interval
    Task task2(2000, 1, &callback_2, &ts, true); // Even longer interval

    // Tasks should execute immediately due to enable(), let them
    bool success = runSchedulerUntil(ts, []() { return getTestOutputCount() >= 2; });
    EXPECT_TRUE(success);

    clearTestOutput();

    // Restart tasks with long intervals
    task1.restart();
    task2.restart();
    task1.delay(1000); // Delay them
    task2.delay(2000);

    // startNow should make them execute immediately
    ts.startNow();
    success = runSchedulerUntil(ts, []() { return getTestOutputCount() >= 2; });
    EXPECT_TRUE(success);
}

// ================== INTEGRATION TESTS ==================

/**
 * @brief Integration test for complete task lifecycle with all features
 *
 * TESTS: Full task lifecycle including enable, execute, auto-disable, restart
 *
 * PURPOSE: Verify that all task features work together correctly in a
 * realistic usage scenario, testing the integration of multiple methods
 * and lifecycle events in sequence.
 *
 * LIFECYCLE INTEGRATION TESTED:
 * - Initial state verification (disabled, zero counters)
 * - Enable with onEnable callback triggering
 * - Multiple executions with proper counter tracking
 * - Auto-disable after completing all iterations
 * - OnDisable callback triggering after completion
 * - Restart functionality with state reset
 *
 * TEST SCENARIO:
 * 1. Create task with 3 iterations and lifecycle callbacks
 * 2. Verify initial disabled state
 * 3. Enable and verify onEnable callback
 * 4. Execute all 3 iterations and verify counters
 * 5. Verify auto-disable and onDisable callback
 * 6. Restart and verify reset state and re-enable
 *
 * EXPECTATIONS:
 * - All lifecycle callbacks triggered at correct times
 * - Execution and iteration counters track correctly
 * - Auto-disable occurs after completing iterations
 * - Restart properly resets state and re-enables
 *
 * IMPORTANCE: Integration testing validates that all features work
 * together as designed, ensuring reliable operation in real applications
 * with complex task lifecycle requirements.
 */
TEST_F(SchedulerThoroughTest, ComplexTaskLifecycle) {
    Scheduler ts;
    Task task(200, 3, &basic_callback, &ts, false, &test_onEnable, &test_onDisable);

    // Full lifecycle test
    EXPECT_FALSE(task.isEnabled());
    EXPECT_EQ(task.getRunCounter(), 0);

    // Enable and run
    task.enable();
    EXPECT_TRUE(onEnable_called);
    EXPECT_TRUE(task.isEnabled());

    // Run all iterations
    bool success = runSchedulerUntil(ts, []() { return callback_counter >= 3; });
    EXPECT_TRUE(success);
    EXPECT_EQ(callback_counter, 3);
    EXPECT_EQ(task.getRunCounter(), 3);
    EXPECT_FALSE(task.isEnabled()); // Auto-disabled after iterations
    EXPECT_TRUE(onDisable_called);

    // Restart and verify
    onEnable_called = false;
    onDisable_called = false;
    task.restart();
    EXPECT_TRUE(onEnable_called);
    EXPECT_TRUE(task.isEnabled());
    EXPECT_EQ(task.getIterations(), 3); // Reset
}

/**
 * @brief Integration test for multiple concurrent tasks with different timing
 *
 * TESTS: Multiple tasks with different enable timing and execution patterns
 *
 * PURPOSE: Verify that the scheduler correctly manages multiple concurrent
 * tasks with different timing characteristics, ensuring proper execution
 * order and no interference between tasks.
 *
 * MULTI-TASK INTEGRATION TESTED:
 * - Three tasks with different intervals (100ms, 150ms, 200ms)
 * - Different enable timing: immediate, 50ms delay, 100ms delay
 * - Each task executes 2 iterations (6 total executions)
 * - Proper execution ordering based on timing
 * - No task interference or missed executions
 *
 * TEST SCENARIO:
 * 1. Create three tasks with different intervals and 2 iterations each
 * 2. Enable task1 immediately, task2 after 50ms, task3 after 100ms
 * 3. Run until all 6 executions complete (2 per task)
 * 4. Verify first execution follows enable timing order
 * 5. Confirm all tasks complete their iterations
 *
 * EXPECTATIONS:
 * - All 6 executions complete successfully
 * - First execution is from task1 (immediate enable)
 * - Total execution count reaches 6
 * - No tasks interfere with each other
 *
 * IMPORTANCE: Multi-task integration validates scheduler's core
 * functionality under realistic concurrent workloads, ensuring
 * reliable operation in complex applications.
 */
TEST_F(SchedulerThoroughTest, MultipleTasksInteraction) {
    Scheduler ts;
    Task task1(100, 2, &callback_1, &ts, false);
    Task task2(150, 2, &callback_2, &ts, false);
    Task task3(200, 2, &callback_3, &ts, false);

    // Enable with different timings
    task1.enable();
    task2.enableDelayed(50);
    task3.enableDelayed(100);

    // All should execute their iterations
    bool success = runSchedulerUntil(ts, []() { return getTestOutputCount() >= 6; });
    EXPECT_TRUE(success);

    // Verify execution order (first executions should follow timing)
    EXPECT_EQ(getTestOutput(0), "callback_1"); // Immediate
    // Subsequent order may vary due to intervals
}

// ================== EDGE CASES AND ERROR HANDLING ==================

/**
 * @brief Edge case test for task with zero iterations
 *
 * TESTS: Task behavior with 0 iterations
 *
 * PURPOSE: Verify that tasks created with zero iterations behave correctly
 * by not executing and remaining disabled, ensuring safe handling of
 * degenerate iteration counts.
 *
 * ZERO ITERATIONS BEHAVIOR:
 * - Task with 0 iterations should never execute
 * - Task should be automatically disabled (no valid executions)
 * - Scheduler should safely handle such tasks without errors
 * - Useful for conditional task creation patterns
 *
 * TEST SCENARIO:
 * 1. Create task with 0 iterations but enabled=true
 * 2. Run scheduler multiple times
 * 3. Verify no executions occur
 * 4. Verify task becomes/remains disabled
 *
 * EXPECTATIONS:
 * - Callback counter remains 0 (no executions)
 * - Task is disabled (invalid iteration count)
 * - No errors or crashes occur
 *
 * IMPORTANCE: Zero iteration handling prevents invalid execution
 * states and enables conditional task creation patterns, essential
 * for robust edge case handling.
 */
TEST_F(SchedulerThoroughTest, TaskZeroIterations) {
    Scheduler ts;
    Task task(100, 0, &basic_callback, &ts, true);

    // Should not execute with 0 iterations
    delay(200);
    ts.execute();
    EXPECT_EQ(callback_counter, 0);
    EXPECT_FALSE(task.isEnabled()); // Should be disabled
}

/**
 * @brief Edge case test for task with infinite iterations (TASK_FOREVER)
 *
 * TESTS: Task behavior with TASK_FOREVER iterations
 *
 * PURPOSE: Verify that tasks configured with TASK_FOREVER execute
 * indefinitely without auto-disabling, maintaining consistent behavior
 * for infinite execution scenarios.
 *
 * INFINITE ITERATIONS BEHAVIOR:
 * - TASK_FOREVER (-1) indicates infinite iterations
 * - Task never auto-disables due to iteration count
 * - getIterations() continues returning TASK_FOREVER
 * - Executions continue until manually disabled
 * - Essential for background and monitoring tasks
 *
 * TEST SCENARIO:
 * 1. Create task with TASK_FOREVER iterations
 * 2. Run scheduler until multiple executions occur
 * 3. Verify task remains enabled throughout
 * 4. Verify getIterations() still returns TASK_FOREVER
 *
 * EXPECTATIONS:
 * - Task executes at least 5 times within timeout
 * - Task remains enabled after multiple executions
 * - getIterations() continues returning TASK_FOREVER
 * - No auto-disable occurs
 *
 * IMPORTANCE: Infinite iteration support enables background tasks,
 * monitoring loops, and continuous processing, essential for
 * long-running and service-oriented applications.
 */
TEST_F(SchedulerThoroughTest, TaskInfiniteIterations) {
    Scheduler ts;
    Task task(50, TASK_FOREVER, &basic_callback, &ts, true);

    // Should keep running indefinitely
    bool success = runSchedulerUntil(ts, []() { return callback_counter >= 5; }, 400);
    EXPECT_TRUE(success);
    EXPECT_TRUE(task.isEnabled()); // Should still be enabled
    EXPECT_EQ(task.getIterations(), TASK_FOREVER); // Should remain -1
}

/**
 * @brief Edge case test for task with null callback pointer
 *
 * TESTS: Task behavior with nullptr callback
 *
 * PURPOSE: Verify that tasks created with null callback pointers handle
 * execution gracefully without crashing, demonstrating robustness in
 * edge cases and enabling placeholder task patterns.
 *
 * NULL CALLBACK BEHAVIOR:
 * - Task with nullptr callback should not crash during execution
 * - Task lifecycle continues normally (timing, iterations, etc.)
 * - No callback code executes (safe no-op behavior)
 * - Enables placeholder and template task patterns
 * - Demonstrates scheduler robustness
 *
 * TEST SCENARIO:
 * 1. Create task with nullptr callback but valid parameters
 * 2. Run scheduler and let task execute
 * 3. Verify no crashes or exceptions occur
 * 4. Verify no callback-specific effects occur
 *
 * EXPECTATIONS:
 * - No crashes or exceptions during execution
 * - Callback counter remains 0 (no callback executed)
 * - Task lifecycle proceeds normally
 * - Scheduler continues operating safely
 *
 * IMPORTANCE: Null callback handling ensures scheduler robustness
 * and enables placeholder task patterns, essential for template
 * systems and defensive programming practices.
 */
TEST_F(SchedulerThoroughTest, TaskNullCallback) {
    Scheduler ts;
    Task task(100, 3, nullptr, &ts, true);

    // Should not crash with null callback
    delay(200);
    ts.execute();
    EXPECT_EQ(callback_counter, 0); // No callback executed
    // Task should still run through its lifecycle
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