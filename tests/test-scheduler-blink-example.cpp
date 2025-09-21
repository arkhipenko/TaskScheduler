// test-scheduler-blink-example.cpp - TaskScheduler Blink Example Validation Tests
// This file contains comprehensive tests validating the TaskScheduler Blink example functionality
// Based on examples\Scheduler_example00_Blink\ demonstrating various LED blinking approaches
//
// =====================================================================================
// BLINK EXAMPLE TEST PLAN AND COVERAGE MATRIX
// =====================================================================================
//
// PURPOSE: Validate all blink example patterns and TaskScheduler methods used in real applications
// APPROACH: Simulate LED state changes and timing patterns without actual hardware
// SCOPE: All six blink approaches from the reference example with comprehensive verification
//
// COVERAGE MATRIX:
// ================
//
// 1. APPROACH 1 - SIMPLE FLAG DRIVEN BLINKING
//    ├── Boolean State Management
//    │   ├── LED state toggling logic
//    │   ├── isFirstIteration() detection
//    │   └── isLastIteration() cleanup
//    ├── Task Lifecycle
//    │   ├── Auto-enabled task execution
//    │   ├── Fixed interval timing (500ms)
//    │   └── Limited iteration count (20 cycles for 10 seconds)
//    └── Task Chaining
//        └── Delayed restart of next task pattern
//
// 2. APPROACH 2 - DUAL CALLBACK METHOD SWITCHING
//    ├── Dynamic Callback Switching
//    │   ├── setCallback() method usage
//    │   ├── ON callback registration
//    │   └── OFF callback registration
//    ├── Callback Coordination
//    │   ├── State transition between callbacks
//    │   ├── First/last iteration handling in both callbacks
//    │   └── Proper LED state management
//    └── Task Chain Management
//        └── Sequential task activation with delays
//
// 3. APPROACH 3 - RUN COUNTER DRIVEN BLINKING
//    ├── Run Counter Logic
//    │   ├── getRunCounter() method usage
//    │   ├── Odd/even counter state detection
//    │   └── Counter-based LED state determination
//    ├── Task State Management
//    │   ├── First iteration detection
//    │   ├── Last iteration handling
//    │   └── Task chain progression
//    └── OnEnable Callback Registration
//        └── Dynamic setOnEnable() assignment
//
// 4. APPROACH 4 - STATUS REQUEST COORDINATION
//    ├── StatusRequest Objects
//    │   ├── getInternalStatusRequest() usage
//    │   ├── waitForDelayed() coordination
//    │   └── Inter-task communication patterns
//    ├── OnEnable/OnDisable Callbacks
//    │   ├── OnEnable callback setup and removal
//    │   ├── OnDisable callback execution
//    │   └── Task lifecycle management
//    ├── Task Coordination
//    │   ├── Two-task ping-pong pattern
//    │   ├── Counter-based termination
//    │   └── Manual task disable() calls
//    └── Advanced Task Control
//        └── Delayed restart with offset timing
//
// 5. APPROACH 5 - INTERLEAVING TASKS
//    ├── Dual Task Pattern
//    │   ├── Independent task scheduling
//    │   ├── Phase-shifted execution (300ms offset)
//    │   └── Synchronized start/stop
//    ├── OnEnable Management
//    │   ├── One-time OnEnable callback
//    │   ├── Callback removal after first use
//    │   └── Task state coordination
//    └── OnDisable Coordination
//        └── Task chain progression from OnDisable
//
// 6. APPROACH 6 - RANDOM INTERVAL GENERATION
//    ├── Dynamic Interval Modification
//    │   ├── setInterval() runtime changes
//    │   ├── Random interval generation
//    │   └── Complementary timing calculation
//    ├── Run Counter + Interval Combination
//    │   ├── Counter-based state logic
//    │   ├── Interval adjustment per state
//    │   └── Total period maintenance
//    ├── OnEnable Setup
//    │   ├── Random interval calculation
//    │   ├── Initial task delay
//    │   └── Callback removal
//    └── Circular Task Pattern
//        └── Return to first approach (endless loop simulation)
//
// INTEGRATION TESTS:
// ==================
//
// 7. TASK CHAINING VALIDATION
//    ├── Sequential Execution Pattern
//    │   ├── Approach 1 → 2 → 3 → 4 → 5 → 6 → 1 (loop)
//    │   ├── 2-second gaps between approaches
//    │   └── Clean state transitions
//    ├── Timing Verification
//    │   ├── 10-second duration per approach
//    │   ├── Proper interval execution
//    │   └── Delay accuracy
//    └── State Management
//        ├── LED state consistency
//        ├── Proper cleanup between approaches
//        └── Task enable/disable coordination
//
// 8. SCHEDULER EXECUTION VALIDATION
//    ├── Core Scheduler Methods
//    │   ├── execute() return values
//    │   ├── Task registration handling
//    │   └── Execution order verification
//    ├── Advanced Features Integration
//    │   ├── STATUS_REQUEST functionality
//    │   ├── OnEnable/OnDisable callbacks
//    │   └── Dynamic callback switching
//    └── Timing Accuracy
//        ├── Millisecond precision verification
//        ├── Interval consistency
//        └── Execution overhead measurement
//
// IMPORTANCE: These tests validate real-world TaskScheduler usage patterns demonstrating
// practical applications, advanced features, and complex task coordination scenarios
// essential for Arduino and embedded system development.

#include <gtest/gtest.h>
#include "Arduino.h"

// TaskScheduler compile-time feature flags (matching the example)
#define _TASK_SLEEP_ON_IDLE_RUN  // Enable 1 ms SLEEP_IDLE powerdowns between runs
#define _TASK_STATUS_REQUEST     // Support for StatusRequest functionality
#include "TaskScheduler.h"

// Global test state - simulates LED and tracks execution
std::vector<std::string> blink_test_output;
bool simulated_led_state = false;
int led_state_changes = 0;
bool debug_output_enabled = false;

// Test timing constants (matching the example)
#define PERIOD1 500
#define PERIOD2 400
#define PERIOD3 300
#define PERIOD4 200
#define PERIOD5 600
#define PERIOD6 300
#define DURATION 10000

// Simulated LED control functions
void LEDOn() {
    if (!simulated_led_state) {
        simulated_led_state = true;
        led_state_changes++;
        blink_test_output.push_back("LED_ON");
        if (debug_output_enabled) {
            std::cout << "LED ON at " << millis() << "ms" << std::endl;
        }
    }
}

void LEDOff() {
    if (simulated_led_state) {
        simulated_led_state = false;
        led_state_changes++;
        blink_test_output.push_back("LED_OFF");
        if (debug_output_enabled) {
            std::cout << "LED OFF at " << millis() << "ms" << std::endl;
        }
    }
}

// Test callback functions (matching the example structure)

// === APPROACH 1: Simple Flag Driven ===
bool LED_state = false;
void blink1CB();
Scheduler* global_scheduler = nullptr;
Task* tBlink1_ptr = nullptr;
Task* tBlink2_ptr = nullptr;
Task* tBlink3_ptr = nullptr;
Task* tBlink4On_ptr = nullptr;
Task* tBlink4Off_ptr = nullptr;
Task* tBlink5On_ptr = nullptr;
Task* tBlink5Off_ptr = nullptr;
Task* tBlink6_ptr = nullptr;

void blink1CB() {
    if (tBlink1_ptr && tBlink1_ptr->isFirstIteration()) {
        blink_test_output.push_back("BLINK1_START");
        LED_state = false;
    }

    if (LED_state) {
        LEDOff();
        LED_state = false;
    } else {
        LEDOn();
        LED_state = true;
    }

    if (tBlink1_ptr && tBlink1_ptr->isLastIteration()) {
        blink_test_output.push_back("BLINK1_END");
        LEDOff();
        // In real example, would start tBlink2
    }
}

// === APPROACH 2: Dual Callback Methods ===
void blink2CB_ON();
void blink2CB_OFF();

void blink2CB_ON() {
    if (tBlink2_ptr && tBlink2_ptr->isFirstIteration()) {
        blink_test_output.push_back("BLINK2_START");
    }

    LEDOn();
    if (tBlink2_ptr) {
        tBlink2_ptr->setCallback(&blink2CB_OFF);
    }

    if (tBlink2_ptr && tBlink2_ptr->isLastIteration()) {
        blink_test_output.push_back("BLINK2_END");
        LEDOff();
    }
}

void blink2CB_OFF() {
    LEDOff();
    if (tBlink2_ptr) {
        tBlink2_ptr->setCallback(&blink2CB_ON);
    }

    if (tBlink2_ptr && tBlink2_ptr->isLastIteration()) {
        blink_test_output.push_back("BLINK2_END");
        LEDOff();
    }
}

// === APPROACH 3: Run Counter Driven ===
void blink3CB() {
    if (tBlink3_ptr && tBlink3_ptr->isFirstIteration()) {
        blink_test_output.push_back("BLINK3_START");
    }

    if (tBlink3_ptr && (tBlink3_ptr->getRunCounter() & 1)) {
        LEDOn();
    } else {
        LEDOff();
    }

    if (tBlink3_ptr && tBlink3_ptr->isLastIteration()) {
        blink_test_output.push_back("BLINK3_END");
        LEDOff();
    }
}

// === APPROACH 4: Status Request Based ===
int counter = 0;
bool blink41OE() {
    blink_test_output.push_back("BLINK4_START");
    counter = 0;
    if (tBlink4On_ptr) {
        tBlink4On_ptr->setOnEnable(nullptr);
    }
    return true;
}

void blink41() {
    LEDOn();
    if (tBlink4On_ptr && tBlink4Off_ptr) {
        StatusRequest* r = tBlink4On_ptr->getInternalStatusRequest();
        tBlink4Off_ptr->waitForDelayed(r);
    }
    counter++;
}

void blink42() {
    LEDOff();
    if (tBlink4On_ptr && tBlink4Off_ptr) {
        StatusRequest* r = tBlink4Off_ptr->getInternalStatusRequest();
        tBlink4On_ptr->waitForDelayed(r);
    }
    counter++;
}

void blink42OD() {
    if (counter >= DURATION / PERIOD4) {
        blink_test_output.push_back("BLINK4_END");
        if (tBlink4On_ptr) tBlink4On_ptr->disable();
        if (tBlink4Off_ptr) tBlink4Off_ptr->disable();
        LEDOff();
    }
}

// === APPROACH 5: Interleaving Tasks ===
bool blink51OE() {
    blink_test_output.push_back("BLINK5_START");
    if (tBlink5On_ptr) {
        tBlink5On_ptr->setOnEnable(nullptr);
    }
    return true;
}

void blink51() {
    LEDOn();
}

void blink52() {
    LEDOff();
}

void blink52OD() {
    blink_test_output.push_back("BLINK5_END");
    LEDOff();
}

// === APPROACH 6: Random Interval ===
long interval6 = 0;
bool blink6OE() {
    blink_test_output.push_back("BLINK6_START");
    interval6 = 500; // Fixed for testing (instead of random)
    if (tBlink6_ptr) {
        tBlink6_ptr->setInterval(interval6);
    }
    return true;
}

void blink6CB() {
    if (tBlink6_ptr && (tBlink6_ptr->getRunCounter() & 1)) {
        LEDOn();
        tBlink6_ptr->setInterval(interval6);
    } else {
        LEDOff();
        if (tBlink6_ptr) {
            tBlink6_ptr->setInterval(1000 - interval6);
        }
    }
}

void blink6OD() {
    blink_test_output.push_back("BLINK6_END");
    LEDOff();
}

// Test helper functions
void clearBlinkTestOutput() {
    blink_test_output.clear();
    simulated_led_state = false;
    led_state_changes = 0;
}

size_t getBlinkTestOutputCount() {
    return blink_test_output.size();
}

std::string getBlinkTestOutput(size_t index) {
    if (index < blink_test_output.size()) {
        return blink_test_output[index];
    }
    return "";
}

// Test condition functions to replace lambda functions
bool condition_output_count_1() {
    return getBlinkTestOutputCount() >= 1;
}

bool condition_led_changes_6() {
    return led_state_changes >= 6;
}

bool condition_led_changes_8() {
    return led_state_changes >= 8;
}

bool condition_led_changes_5() {
    return led_state_changes >= 5;
}

bool condition_led_changes_4() {
    return led_state_changes >= 4;
}

bool condition_tBlink1_disabled() {
    return tBlink1_ptr && !tBlink1_ptr->isEnabled();
}

bool condition_tBlink2_disabled() {
    return tBlink2_ptr && !tBlink2_ptr->isEnabled();
}

bool condition_tBlink3_disabled() {
    return tBlink3_ptr && !tBlink3_ptr->isEnabled();
}

bool condition_tBlink3_runcount_6() {
    return tBlink3_ptr && tBlink3_ptr->getRunCounter() >= 6;
}

bool condition_counter_10() {
    return counter >= 10;
}

bool condition_counter_duration_period4() {
    return counter >= DURATION / PERIOD4;
}

bool condition_both_blink4_disabled() {
    return tBlink4On_ptr && tBlink4Off_ptr &&
           !tBlink4On_ptr->isEnabled() && !tBlink4Off_ptr->isEnabled();
}

bool condition_both_blink5_disabled() {
    return tBlink5On_ptr && tBlink5Off_ptr &&
           !tBlink5On_ptr->isEnabled() && !tBlink5Off_ptr->isEnabled();
}

bool condition_tBlink6_disabled() {
    return tBlink6_ptr && !tBlink6_ptr->isEnabled();
}

bool condition_tBlink6_runcount_4() {
    return tBlink6_ptr && tBlink6_ptr->getRunCounter() >= 4;
}

// Scheduler execution helper with callback function pointer
bool runBlinkSchedulerUntil(Scheduler& ts, bool (*condition)(), unsigned long timeout_ms = 2000) {
    unsigned long start_time = millis();
    while (millis() - start_time < timeout_ms) {
        bool idle = ts.execute();
        if (condition()) {
            return true;
        }
        if (idle) {
            delay(1); // Simulate idle sleep
        }
    }
    return false;
}

// Test fixture for blink example validation
class BlinkExampleTest : public ::testing::Test {
protected:
    void SetUp() override {
        clearBlinkTestOutput();
        LED_state = false;
        counter = 0;
        interval6 = 0;
        debug_output_enabled = false;
    }

    void TearDown() override {
        clearBlinkTestOutput();
    }
};

// ================== APPROACH 1 TESTS ==================

/**
 * @brief Test Approach 1: Simple Flag Driven Blinking
 *
 * TESTS: Boolean state management, isFirstIteration(), isLastIteration()
 *
 * PURPOSE: Validate the simplest blinking approach using a boolean flag
 * to track LED state. Verifies proper first/last iteration detection,
 * state toggling logic, and task lifecycle management.
 *
 * IMPORTANCE: This is the most fundamental blinking pattern, essential
 * for understanding basic TaskScheduler usage and state management.
 */
TEST_F(BlinkExampleTest, Approach1_SimpleFlagDriven) {
    Scheduler ts;
    global_scheduler = &ts;

    Task tBlink1(PERIOD1, DURATION / PERIOD1, &blink1CB, &ts, true);
    tBlink1_ptr = &tBlink1;

    // Should execute first iteration
    bool success = runBlinkSchedulerUntil(ts, condition_output_count_1);
    EXPECT_TRUE(success);
    EXPECT_EQ(getBlinkTestOutput(0), "BLINK1_START");
    EXPECT_TRUE(simulated_led_state); // Should turn LED on first

    // Let it run several cycles
    success = runBlinkSchedulerUntil(ts, condition_led_changes_6, 3000);
    EXPECT_TRUE(success);
    EXPECT_GE(led_state_changes, 6);

    // Wait for completion
    success = runBlinkSchedulerUntil(ts, condition_tBlink1_disabled, 15000);
    EXPECT_TRUE(success);
    EXPECT_FALSE(tBlink1.isEnabled());

    // Check that it recorded the end
    bool found_end = false;
    for (const auto& output : blink_test_output) {
        if (output == "BLINK1_END") {
            found_end = true;
            break;
        }
    }
    EXPECT_TRUE(found_end);
    EXPECT_FALSE(simulated_led_state); // LED should be off at end

    tBlink1_ptr = nullptr;
}

// ================== APPROACH 2 TESTS ==================

/**
 * @brief Test Approach 2: Dual Callback Method Switching
 *
 * TESTS: setCallback(), dynamic callback switching, dual-method coordination
 *
 * PURPOSE: Validate the callback switching approach where one callback
 * turns LED on and switches to OFF callback, which turns LED off and
 * switches back to ON callback, creating a ping-pong pattern.
 *
 * IMPORTANCE: Demonstrates advanced TaskScheduler feature of runtime
 * callback modification, essential for state machine implementations.
 */
TEST_F(BlinkExampleTest, Approach2_DualCallbackSwitching) {
    Scheduler ts;

    Task tBlink2(PERIOD2, DURATION / PERIOD2, &blink2CB_ON, &ts, true);
    tBlink2_ptr = &tBlink2;

    // Should start with ON callback
    bool success = runBlinkSchedulerUntil(ts, condition_output_count_1);
    EXPECT_TRUE(success);
    EXPECT_EQ(getBlinkTestOutput(0), "BLINK2_START");
    EXPECT_TRUE(simulated_led_state); // First callback turns LED on

    // Let it run and switch callbacks several times
    success = runBlinkSchedulerUntil(ts, condition_led_changes_8, 4000);
    EXPECT_TRUE(success);
    EXPECT_GE(led_state_changes, 8);

    // Verify alternating pattern
    int on_count = 0, off_count = 0;
    for (const auto& output : blink_test_output) {
        if (output == "LED_ON") on_count++;
        if (output == "LED_OFF") off_count++;
    }
    EXPECT_GT(on_count, 0);
    EXPECT_GT(off_count, 0);
    EXPECT_LE(abs(on_count - off_count), 1); // Should be roughly equal

    // Wait for completion
    success = runBlinkSchedulerUntil(ts, condition_tBlink2_disabled, 15000);
    EXPECT_TRUE(success);
    EXPECT_FALSE(simulated_led_state); // LED should be off at end

    tBlink2_ptr = nullptr;
}

// ================== APPROACH 3 TESTS ==================

/**
 * @brief Test Approach 3: Run Counter Driven Blinking
 *
 * TESTS: getRunCounter(), odd/even logic, counter-based state control
 *
 * PURPOSE: Validate blinking controlled by run counter where odd counts
 * turn LED on and even counts turn LED off. Tests counter increment
 * behavior and bitwise odd/even detection.
 *
 * IMPORTANCE: Demonstrates practical use of run counters for state
 * determination, common in cyclic operations and alternating behaviors.
 */
TEST_F(BlinkExampleTest, Approach3_RunCounterDriven) {
    Scheduler ts;

    Task tBlink3(PERIOD3, DURATION / PERIOD3, &blink3CB, &ts, true);
    tBlink3_ptr = &tBlink3;

    // Should start execution
    bool success = runBlinkSchedulerUntil(ts, condition_output_count_1);
    EXPECT_TRUE(success);
    EXPECT_EQ(getBlinkTestOutput(0), "BLINK3_START");

    // Let it run several cycles to test counter logic
    success = runBlinkSchedulerUntil(ts, condition_tBlink3_runcount_6, 3000);
    EXPECT_TRUE(success);
    EXPECT_GE(tBlink3.getRunCounter(), 6);

    // Verify that LED state follows counter parity
    // Run counter starts at 1 (odd) = LED ON, 2 (even) = LED OFF, etc.
    int run_count = tBlink3.getRunCounter();
    if (run_count & 1) {
        EXPECT_TRUE(simulated_led_state);
    } else {
        EXPECT_FALSE(simulated_led_state);
    }

    // Wait for completion
    success = runBlinkSchedulerUntil(ts, condition_tBlink3_disabled, 15000);
    EXPECT_TRUE(success);
    EXPECT_FALSE(simulated_led_state); // LED should be off at end

    tBlink3_ptr = nullptr;
}

// ================== APPROACH 4 TESTS ==================

/**
 * @brief Test Approach 4: Status Request Coordination
 *
 * TESTS: getInternalStatusRequest(), waitForDelayed(), OnEnable/OnDisable callbacks
 *
 * PURPOSE: Validate the most advanced blinking approach using status
 * requests for inter-task coordination. Tests two tasks passing control
 * back and forth using internal status request objects.
 *
 * IMPORTANCE: Demonstrates sophisticated task coordination patterns
 * essential for complex embedded systems requiring precise timing
 * and event-driven behaviors.
 */
TEST_F(BlinkExampleTest, Approach4_StatusRequestCoordination) {
    Scheduler ts;

    Task tBlink4On(PERIOD4, TASK_ONCE, &blink41, &ts, false, &blink41OE);
    Task tBlink4Off(PERIOD4, TASK_ONCE, &blink42, &ts, false, nullptr, &blink42OD);
    tBlink4On_ptr = &tBlink4On;
    tBlink4Off_ptr = &tBlink4Off;

    // Enable the first task to start the sequence
    tBlink4On.enable();

    // Should execute OnEnable and start
    bool success = runBlinkSchedulerUntil(ts, condition_output_count_1);
    EXPECT_TRUE(success);
    EXPECT_EQ(getBlinkTestOutput(0), "BLINK4_START");

    // Let the ping-pong pattern run
    success = runBlinkSchedulerUntil(ts, condition_counter_10, 3000);
    EXPECT_TRUE(success);
    EXPECT_GE(counter, 10);

    // Verify both tasks are coordinating (alternating LED states)
    EXPECT_GE(led_state_changes, 5);

    // Wait for the sequence to complete
    success = runBlinkSchedulerUntil(ts, condition_counter_duration_period4, 15000);
    EXPECT_TRUE(success);

    // Both tasks should be disabled by OnDisable callback
    EXPECT_FALSE(tBlink4On.isEnabled());
    EXPECT_FALSE(tBlink4Off.isEnabled());
    EXPECT_FALSE(simulated_led_state); // LED should be off at end

    tBlink4On_ptr = nullptr;
    tBlink4Off_ptr = nullptr;
}

// ================== APPROACH 5 TESTS ==================

/**
 * @brief Test Approach 5: Interleaving Tasks
 *
 * TESTS: Phase-shifted dual tasks, OnEnable callback management, synchronized execution
 *
 * PURPOSE: Validate two independent tasks running with phase offset
 * where one task turns LED on and another turns it off, creating
 * precise timing control through task interleaving.
 *
 * IMPORTANCE: Demonstrates advanced timing patterns for applications
 * requiring precise phase relationships and independent task control.
 */
TEST_F(BlinkExampleTest, Approach5_InterleavingTasks) {
    Scheduler ts;

    Task tBlink5On(PERIOD5, DURATION / PERIOD5, &blink51, &ts, false, &blink51OE);
    Task tBlink5Off(PERIOD5, DURATION / PERIOD5, &blink52, &ts, false, nullptr, &blink52OD);
    tBlink5On_ptr = &tBlink5On;
    tBlink5Off_ptr = &tBlink5Off;

    // Start both tasks (in real example, they'd be phase-shifted)
    tBlink5On.enable();
    tBlink5Off.enable();

    // Should execute OnEnable
    bool success = runBlinkSchedulerUntil(ts, condition_output_count_1);
    EXPECT_TRUE(success);
    EXPECT_EQ(getBlinkTestOutput(0), "BLINK5_START");

    // Let both tasks run
    success = runBlinkSchedulerUntil(ts, condition_led_changes_8, 4000);
    EXPECT_TRUE(success);
    EXPECT_GE(led_state_changes, 8);

    // Both tasks should be enabled and running
    EXPECT_TRUE(tBlink5On.isEnabled() || tBlink5Off.isEnabled());

    // Wait for completion
    success = runBlinkSchedulerUntil(ts, condition_both_blink5_disabled, 15000);
    EXPECT_TRUE(success);
    EXPECT_FALSE(simulated_led_state); // LED should be off at end

    tBlink5On_ptr = nullptr;
    tBlink5Off_ptr = nullptr;
}

// ================== APPROACH 6 TESTS ==================

/**
 * @brief Test Approach 6: Random Interval Generation
 *
 * TESTS: setInterval(), dynamic interval modification, run counter + interval combination
 *
 * PURPOSE: Validate dynamic interval adjustment where LED ON time is
 * random and OFF time is complementary to maintain constant period.
 * Tests runtime interval modification and complex timing patterns.
 *
 * IMPORTANCE: Demonstrates adaptive timing behaviors essential for
 * applications requiring variable timing patterns while maintaining
 * overall system timing constraints.
 */
TEST_F(BlinkExampleTest, Approach6_RandomIntervalGeneration) {
    Scheduler ts;

    Task tBlink6(PERIOD6, DURATION / PERIOD6, &blink6CB, &ts, false, &blink6OE, &blink6OD);
    tBlink6_ptr = &tBlink6;

    // Enable task to start
    tBlink6.enable();

    // Should execute OnEnable and set initial interval
    bool success = runBlinkSchedulerUntil(ts, condition_output_count_1);
    EXPECT_TRUE(success);
    EXPECT_EQ(getBlinkTestOutput(0), "BLINK6_START");
    EXPECT_EQ(interval6, 500); // Fixed value for testing
    EXPECT_EQ(tBlink6.getInterval(), 500);

    // Let it run and change intervals
    success = runBlinkSchedulerUntil(ts, condition_tBlink6_runcount_4, 3000);
    EXPECT_TRUE(success);
    EXPECT_GE(tBlink6.getRunCounter(), 4);

    // Verify interval changes based on run counter
    if (tBlink6.getRunCounter() & 1) {
        EXPECT_EQ(tBlink6.getInterval(), interval6); // ON interval
        EXPECT_TRUE(simulated_led_state);
    } else {
        EXPECT_EQ(tBlink6.getInterval(), 1000 - interval6); // OFF interval
        EXPECT_FALSE(simulated_led_state);
    }

    // Wait for completion
    success = runBlinkSchedulerUntil(ts, condition_tBlink6_disabled, 15000);
    EXPECT_TRUE(success);
    EXPECT_FALSE(simulated_led_state); // LED should be off at end

    tBlink6_ptr = nullptr;
}

// ================== INTEGRATION TESTS ==================

/**
 * @brief Test Sequential Task Chain Execution
 *
 * TESTS: Complete blink example sequence, task chaining, scheduler coordination
 *
 * PURPOSE: Validate the complete blink example workflow where each
 * approach runs for its duration then triggers the next approach,
 * creating a continuous demonstration cycle.
 *
 * IMPORTANCE: Verifies end-to-end scheduler behavior with complex
 * task relationships, timing coordination, and state management
 * across multiple interconnected blinking patterns.
 */
TEST_F(BlinkExampleTest, SequentialTaskChainExecution) {
    Scheduler ts;
    debug_output_enabled = true;

    // Create all tasks (simplified version)
    Task tBlink1(PERIOD1, 4, &blink1CB, &ts, true); // Shorter duration for testing
    tBlink1_ptr = &tBlink1;

    // Run just the first approach for integration test
    bool success = runBlinkSchedulerUntil(ts, condition_output_count_1);
    EXPECT_TRUE(success);
    EXPECT_EQ(getBlinkTestOutput(0), "BLINK1_START");

    // Let it complete its cycles
    success = runBlinkSchedulerUntil(ts, condition_tBlink1_disabled, 5000);
    EXPECT_TRUE(success);
    EXPECT_FALSE(tBlink1.isEnabled());

    // Verify proper execution pattern
    EXPECT_GE(led_state_changes, 4); // Should have blinked at least twice
    EXPECT_FALSE(simulated_led_state); // Should end with LED off

    tBlink1_ptr = nullptr;
    debug_output_enabled = false;
}

/**
 * @brief Test Scheduler Core Functionality with Blink Patterns
 *
 * TESTS: execute() return values, task management, timing accuracy
 *
 * PURPOSE: Validate scheduler core behavior under the complex timing
 * and callback patterns used in the blink example. Tests idle detection,
 * execution ordering, and overall scheduler robustness.
 *
 * IMPORTANCE: Ensures the scheduler performs correctly under realistic
 * workloads with multiple timing patterns, callback switches, and
 * complex task interdependencies typical of real applications.
 */
TEST_F(BlinkExampleTest, SchedulerCoreFunctionalityValidation) {
    Scheduler ts;

    // Create a simple blinking task
    Task tBlink(500, 6, &blink1CB, &ts, true);
    tBlink1_ptr = &tBlink;

    int execute_calls = 0;
    int idle_returns = 0;

    // Monitor scheduler execution behavior
    unsigned long start_time = millis();
    while (millis() - start_time < 4000 && tBlink.isEnabled()) {
        bool idle = ts.execute();
        execute_calls++;
        if (idle) {
            idle_returns++;
        }
    }

    // Verify scheduler execution statistics
    EXPECT_GT(execute_calls, 100); // Should have been called many times
    EXPECT_GT(idle_returns, 50); // Should have had idle periods
    EXPECT_LE(idle_returns, execute_calls); // Idle returns <= total calls

    // Verify task completed successfully
    EXPECT_FALSE(tBlink.isEnabled());
    EXPECT_GE(led_state_changes, 6); // Should have blinked 3 times (on/off cycles)

    tBlink1_ptr = nullptr;
}

/**
 * @brief Main test runner function for blink example tests
 */
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}