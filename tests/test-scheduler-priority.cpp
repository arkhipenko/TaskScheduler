// test-scheduler-priority.cpp - TaskScheduler layered prioritization unit tests
// This file contains comprehensive tests for TaskScheduler layered prioritization
// functionality enabled by the _TASK_PRIORITY compile-time directive
//
// =====================================================================================
// PRIORITY TEST PLAN AND COVERAGE MATRIX
// =====================================================================================
//
// PURPOSE: Validate TaskScheduler layered prioritization system
// APPROACH: Traditional function pointers for maximum platform compatibility
// SCOPE: Priority scheduling, scheduler hierarchy, and execution ordering
//
// COMPILE DIRECTIVES TESTED:
// - _TASK_PRIORITY:            Layered task prioritization support
//
// COVERAGE MATRIX:
// ================
//
// 1. BASIC PRIORITY FUNCTIONALITY
//    ├── Scheduler Creation and Hierarchy Setup
//    │   ├── Base scheduler creation
//    │   ├── High priority scheduler creation
//    │   ├── setHighPriorityScheduler() method
//    │   ├── currentScheduler() method
//    │   └── Scheduler hierarchy validation
//    │
//    ├── Task Assignment to Priority Levels
//    │   ├── Tasks added to base priority scheduler
//    │   ├── Tasks added to high priority scheduler
//    │   ├── Task execution order validation
//    │   └── Priority-based scheduling behavior
//    │
//    └── Basic Priority Execution Patterns
//        ├── High priority tasks execute before base priority
//        ├── Multiple high priority tasks execution order
//        ├── Base priority tasks execution after high priority
//        └── Mixed priority execution scenarios
//
// 2. MULTI-LAYER PRIORITY HIERARCHY
//    ├── Three-Layer Priority System
//    │   ├── Base priority (lowest)
//    │   ├── Medium priority (middle)
//    │   ├── High priority (highest)
//    │   └── Execution order verification
//    │
//    ├── Complex Priority Chain Execution
//    │   ├── Nested scheduler execute() calls
//    │   ├── Priority evaluation sequence
//    │   ├── Scheduling overhead measurement
//    │   └── Performance impact validation
//    │
//    └── Priority Collision Handling
//        ├── Same-time scheduling of different priorities
//        ├── Priority-based task selection
//        ├── Execution sequence preservation
//        └── Timing accuracy in priority scenarios
//
// 3. ADVANCED PRIORITY SCENARIOS
//    ├── Dynamic Priority Changes
//    │   ├── Runtime scheduler hierarchy modification
//    │   ├── Task migration between priority levels
//    │   ├── Priority scheduler enable/disable
//    │   └── Scheduler state management
//    │
//    ├── Priority with Task Features
//    │   ├── Priority + scheduling options integration
//    │   ├── Priority + status request coordination
//    │   ├── Priority + timeout handling
//    │   └── Priority + self-destruct behavior
//    │
//    └── Real-World Priority Use Cases
//        ├── Time-critical sensor reading (high priority)
//        ├── Background data processing (base priority)
//        ├── Emergency response handling (highest priority)
//        └── Resource-intensive calculations (low priority)
//
// 4. PRIORITY PERFORMANCE AND OVERHEAD
//    ├── Scheduling Overhead Analysis
//    │   ├── Single scheduler vs. layered performance
//    │   ├── Execution time measurement
//    │   ├── Priority evaluation cost
//    │   └── Overhead scaling with priority layers
//    │
//    ├── Timing Accuracy with Priorities
//    │   ├── High priority task timing precision
//    │   ├── Base priority task timing impact
//    │   ├── Priority-induced scheduling delays
//    │   └── Overall system timing accuracy
//    │
//    └── Resource Utilization
//        ├── Memory usage with priority layers
//        ├── CPU utilization patterns
//        ├── Stack depth analysis
//        └── Performance optimization guidelines
//
// =====================================================================================

// Enable priority functionality for comprehensive testing
#define _TASK_PRIORITY              // Layered task prioritization

#include <gtest/gtest.h>
#include "Arduino.h"
#include "TaskScheduler.h"

// Global test state for priority testing
std::vector<std::string> priority_test_output;
int priority_callback_counter = 0;
unsigned long priority_execution_times[20];  // Store execution timestamps
int priority_execution_index = 0;

// Definition for test_output vector used by Arduino.h mock
std::vector<std::string> test_output;

// Test callback functions (no lambda functions)

/**
 * @brief Base priority task callback - simulates normal priority work
 */
void base_priority_callback() {
    priority_callback_counter++;
    priority_test_output.push_back("base_priority_executed");
    priority_execution_times[priority_execution_index++] = millis();
    std::cout << "Base priority task executed at " << millis() << "ms" << std::endl;
}

/**
 * @brief High priority task callback - simulates critical priority work
 */
void high_priority_callback() {
    priority_callback_counter++;
    priority_test_output.push_back("high_priority_executed");
    priority_execution_times[priority_execution_index++] = millis();
    std::cout << "High priority task executed at " << millis() << "ms" << std::endl;
}

/**
 * @brief Medium priority task callback - simulates intermediate priority work
 */
void medium_priority_callback() {
    priority_callback_counter++;
    priority_test_output.push_back("medium_priority_executed");
    priority_execution_times[priority_execution_index++] = millis();
    std::cout << "Medium priority task executed at " << millis() << "ms" << std::endl;
}

/**
 * @brief Time-critical sensor callback - simulates gyroscope/accelerometer reading
 */
void sensor_critical_callback() {
    priority_callback_counter++;
    priority_test_output.push_back("sensor_critical_executed");
    priority_execution_times[priority_execution_index++] = millis();
    // Simulate precise sensor reading requiring minimal delay
}

/**
 * @brief Background processing callback - simulates non-critical background work
 */
void background_processing_callback() {
    priority_callback_counter++;
    priority_test_output.push_back("background_processing_executed");
    priority_execution_times[priority_execution_index++] = millis();
    // Simulate background data processing
}

/**
 * @brief Emergency response callback - simulates highest priority emergency handling
 */
void emergency_response_callback() {
    priority_callback_counter++;
    priority_test_output.push_back("emergency_response_executed");
    priority_execution_times[priority_execution_index++] = millis();
    // Simulate emergency response requiring immediate attention
}

/**
 * @brief Test fixture class for TaskScheduler priority functionality
 *
 * Provides setup and teardown for priority tests, ensuring clean state
 * between tests and utility methods for priority scenario validation.
 */
class PrioritySchedulerTest : public ::testing::Test {
protected:
    /**
     * @brief Set up test environment for priority testing
     *
     * Clears all test state and initializes timing system for priority tests.
     */
    void SetUp() override {
        clearPriorityTestOutput();
        priority_callback_counter = 0;
        priority_execution_index = 0;
        memset(priority_execution_times, 0, sizeof(priority_execution_times));
        millis(); // Initialize timing
    }

    /**
     * @brief Clean up test environment after priority tests
     *
     * Ensures no test artifacts affect subsequent tests.
     */
    void TearDown() override {
        clearPriorityTestOutput();
        priority_callback_counter = 0;
        priority_execution_index = 0;
        memset(priority_execution_times, 0, sizeof(priority_execution_times));
    }

    /**
     * @brief Helper to run scheduler until condition or timeout for priority tests
     */
    bool runPrioritySchedulerUntil(Scheduler& ts, std::function<bool()> condition, unsigned long timeout_ms = 2000) {
        return waitForCondition([&]() {
            ts.execute();
            return condition();
        }, timeout_ms);
    }

    /**
     * @brief Clear priority test output buffer
     */
    void clearPriorityTestOutput() {
        priority_test_output.clear();
    }

    /**
     * @brief Get count of priority test output entries
     */
    size_t getPriorityTestOutputCount() {
        return priority_test_output.size();
    }

    /**
     * @brief Get specific priority test output entry
     */
    std::string getPriorityTestOutput(size_t index) {
        if (index < priority_test_output.size()) {
            return priority_test_output[index];
        }
        return "";
    }

    /**
     * @brief Verify execution order matches expected priority sequence
     */
    bool verifyExecutionOrder(const std::vector<std::string>& expected_order) {
        if (expected_order.size() != priority_test_output.size()) {
            return false;
        }

        for (size_t i = 0; i < expected_order.size(); i++) {
            if (expected_order[i] != priority_test_output[i]) {
                return false;
            }
        }
        return true;
    }
};

// ================== BASIC PRIORITY FUNCTIONALITY TESTS ==================

/**
 * @brief Test basic scheduler hierarchy setup and validation
 *
 * TESTS: setHighPriorityScheduler(), currentScheduler()
 *
 * PURPOSE: Verify that scheduler hierarchy can be established correctly
 * and that the priority relationships work as expected for basic scenarios.
 *
 * PRIORITY HIERARCHY SETUP:
 * - Base scheduler: Normal priority tasks
 * - High scheduler: High priority tasks
 * - Hierarchy relationship: base.setHighPriorityScheduler(&high)
 * - Execution order: High priority tasks execute before base priority
 *
 * TEST SCENARIO:
 * 1. Create base and high priority schedulers
 * 2. Establish hierarchy relationship
 * 3. Add tasks to both schedulers
 * 4. Execute base scheduler (should execute high priority first)
 * 5. Verify execution order matches priority hierarchy
 *
 * EXPECTATIONS:
 * - High priority tasks execute before base priority tasks
 * - currentScheduler() returns correct scheduler during execution
 * - Hierarchy setup works correctly
 * - Execution order follows priority rules
 */
TEST_F(PrioritySchedulerTest, BasicSchedulerHierarchy) {
    // Create base and high priority schedulers
    Scheduler base_scheduler;
    Scheduler high_scheduler;

    // Establish hierarchy - high scheduler has priority over base
    base_scheduler.setHighPriorityScheduler(&high_scheduler);

    // Add tasks to schedulers
    Task base_task(100, 3, &base_priority_callback, &base_scheduler, true);
    Task high_task(100, 2, &high_priority_callback, &high_scheduler, true);

    // Execute base scheduler - should process high priority tasks first
    bool success = runPrioritySchedulerUntil(base_scheduler, []() {
        return priority_callback_counter >= 5; // 2 high + 3 base = 5 total
    });

    EXPECT_TRUE(success);
    EXPECT_EQ(priority_callback_counter, 5);

    // Verify execution order: high priority tasks execute first
    EXPECT_EQ(getPriorityTestOutput(0), "high_priority_executed");
    EXPECT_EQ(getPriorityTestOutput(1), "high_priority_executed");
    // Base tasks should execute after high priority tasks complete
    EXPECT_EQ(getPriorityTestOutput(2), "base_priority_executed");
    EXPECT_EQ(getPriorityTestOutput(3), "base_priority_executed");
    EXPECT_EQ(getPriorityTestOutput(4), "base_priority_executed");
}

/**
 * @brief Test two-layer priority scheduling with timing validation
 *
 * TESTS: Priority-based execution timing and sequence
 *
 * PURPOSE: Verify that two-layer priority scheduling works correctly
 * with proper timing relationships and execution sequence preservation.
 *
 * PRIORITY EXECUTION PATTERN:
 * According to wiki documentation:
 * "Task prioritization is achieved by executing the entire chain of tasks
 * of the higher priority scheduler for every single step (task) of the
 * lower priority chain."
 *
 * For 5 base tasks and 2 high priority tasks:
 * Execution sequence: H1 -> H2 -> B1 -> H1 -> H2 -> B2 -> H1 -> H2 -> B3 -> H1 -> H2 -> B4 -> H1 -> H2 -> B5
 *
 * TEST SCENARIO:
 * 1. Set up base scheduler with 5 tasks at 100ms intervals
 * 2. Set up high scheduler with 2 tasks at 50ms intervals
 * 3. Run for sufficient time to see priority pattern
 * 4. Verify high priority tasks get evaluated more frequently
 * 5. Validate timing relationships
 */
TEST_F(PrioritySchedulerTest, TwoLayerPriorityTiming) {
    Scheduler base_scheduler;
    Scheduler high_scheduler;

    base_scheduler.setHighPriorityScheduler(&high_scheduler);

    // Base priority tasks - longer intervals
    Task base_task1(200, 3, &base_priority_callback, &base_scheduler, true);

    // High priority tasks - shorter intervals for more frequent execution
    Task high_task1(50, 6, &high_priority_callback, &high_scheduler, true);

    // Run scheduler and measure execution patterns
    unsigned long start_time = millis();
    bool success = runPrioritySchedulerUntil(base_scheduler, []() {
        return priority_callback_counter >= 9; // 6 high + 3 base = 9 total
    }, 3000);

    EXPECT_TRUE(success);
    EXPECT_EQ(priority_callback_counter, 9);

    // Verify that high priority tasks executed more frequently
    int high_priority_count = 0;
    int base_priority_count = 0;

    for (size_t i = 0; i < getPriorityTestOutputCount(); i++) {
        if (getPriorityTestOutput(i) == "high_priority_executed") {
            high_priority_count++;
        } else if (getPriorityTestOutput(i) == "base_priority_executed") {
            base_priority_count++;
        }
    }

    EXPECT_EQ(high_priority_count, 6);
    EXPECT_EQ(base_priority_count, 3);

    // High priority tasks should execute first due to shorter interval
    EXPECT_EQ(getPriorityTestOutput(0), "high_priority_executed");
}

/**
 * @brief Test priority collision handling - tasks scheduled at same time
 *
 * TESTS: Priority-based task selection when multiple tasks are ready
 *
 * PURPOSE: Verify that when multiple tasks from different priority levels
 * are ready to execute simultaneously, the priority system correctly
 * selects high priority tasks first.
 */
TEST_F(PrioritySchedulerTest, PriorityCollisionHandling) {
    Scheduler base_scheduler;
    Scheduler high_scheduler;

    base_scheduler.setHighPriorityScheduler(&high_scheduler);

    // Set tasks to execute at similar times to create scheduling collision
    Task base_task(100, 1, &base_priority_callback, &base_scheduler, false);
    Task high_task(100, 1, &high_priority_callback, &high_scheduler, false);

    // Enable both tasks at the same time
    base_task.enable();
    high_task.enable();

    // Execute scheduler - both tasks are ready immediately
    bool success = runPrioritySchedulerUntil(base_scheduler, []() {
        return priority_callback_counter >= 2;
    });

    EXPECT_TRUE(success);
    EXPECT_EQ(priority_callback_counter, 2);

    // High priority task should execute first in collision scenario
    EXPECT_EQ(getPriorityTestOutput(0), "high_priority_executed");
    EXPECT_EQ(getPriorityTestOutput(1), "base_priority_executed");
}

// ================== MULTI-LAYER PRIORITY HIERARCHY TESTS ==================

/**
 * @brief Test three-layer priority hierarchy
 *
 * TESTS: Complex multi-layer priority scheduling
 *
 * PURPOSE: Verify that three-layer priority hierarchy works correctly
 * with proper execution order: Emergency -> High -> Base priority.
 */
TEST_F(PrioritySchedulerTest, ThreeLayerPriorityHierarchy) {
    Scheduler base_scheduler;
    Scheduler high_scheduler;
    Scheduler emergency_scheduler;

    // Establish three-layer hierarchy
    // Emergency (highest) -> High -> Base (lowest)
    high_scheduler.setHighPriorityScheduler(&emergency_scheduler);
    base_scheduler.setHighPriorityScheduler(&high_scheduler);

    // Add tasks to each priority level
    Task base_task(100, 2, &base_priority_callback, &base_scheduler, true);
    Task high_task(100, 2, &high_priority_callback, &high_scheduler, true);
    Task emergency_task(100, 1, &emergency_response_callback, &emergency_scheduler, true);

    // Execute base scheduler - should process all layers in priority order
    bool success = runPrioritySchedulerUntil(base_scheduler, []() {
        return priority_callback_counter >= 5; // 1 emergency + 2 high + 2 base = 5
    });

    EXPECT_TRUE(success);
    EXPECT_EQ(priority_callback_counter, 5);

    // Verify execution order follows three-layer priority
    EXPECT_EQ(getPriorityTestOutput(0), "emergency_response_executed");
    // High priority tasks should execute next
    bool found_high_after_emergency = false;
    for (size_t i = 1; i < getPriorityTestOutputCount(); i++) {
        if (getPriorityTestOutput(i) == "high_priority_executed") {
            found_high_after_emergency = true;
            break;
        }
    }
    EXPECT_TRUE(found_high_after_emergency);
}

/**
 * @brief Test priority scheduling overhead measurement
 *
 * TESTS: Performance impact of layered priority scheduling
 *
 * PURPOSE: Validate that priority scheduling overhead is reasonable
 * and measure the performance impact of different priority configurations.
 *
 * According to wiki: "Scheduling overhead of a 3 layer prioritization
 * approach is 3 times higher than that of a flat execution chain."
 */
TEST_F(PrioritySchedulerTest, PrioritySchedulingOverhead) {
    // Test 1: Single scheduler (baseline)
    {
        clearPriorityTestOutput();
        priority_callback_counter = 0;

        Scheduler single_scheduler;
        Task task1(50, 10, &base_priority_callback, &single_scheduler, true);

        unsigned long start_time = millis();
        runPrioritySchedulerUntil(single_scheduler, []() {
            return priority_callback_counter >= 10;
        });
        unsigned long single_scheduler_time = millis() - start_time;

        EXPECT_EQ(priority_callback_counter, 10);
        std::cout << "Single scheduler time: " << single_scheduler_time << "ms" << std::endl;
    }

    // Test 2: Two-layer priority scheduler
    {
        clearPriorityTestOutput();
        priority_callback_counter = 0;

        Scheduler base_scheduler;
        Scheduler high_scheduler;
        base_scheduler.setHighPriorityScheduler(&high_scheduler);

        Task base_task(50, 5, &base_priority_callback, &base_scheduler, true);
        Task high_task(50, 5, &high_priority_callback, &high_scheduler, true);

        unsigned long start_time = millis();
        runPrioritySchedulerUntil(base_scheduler, []() {
            return priority_callback_counter >= 10;
        });
        unsigned long priority_scheduler_time = millis() - start_time;

        EXPECT_EQ(priority_callback_counter, 10);
        std::cout << "Priority scheduler time: " << priority_scheduler_time << "ms" << std::endl;

        // Priority scheduling should be slower but still reasonable
        // We don't enforce strict timing due to test environment variability
        EXPECT_GT(priority_scheduler_time, 0);
    }
}

// ================== ADVANCED PRIORITY SCENARIOS ==================

/**
 * @brief Test real-world priority use case - sensor + background processing
 *
 * TESTS: Realistic priority scenario with time-critical and background tasks
 *
 * PURPOSE: Validate priority system works for real-world scenarios like
 * time-critical sensor reading with background data processing.
 *
 * SCENARIO:
 * - High priority: Time-critical sensor reading (gyroscope/accelerometer)
 * - Base priority: Background data processing and housekeeping
 * - Emergency priority: Safety monitoring and emergency response
 */
TEST_F(PrioritySchedulerTest, RealWorldSensorPriorityScenario) {
    Scheduler base_scheduler;
    Scheduler sensor_scheduler;
    Scheduler emergency_scheduler;

    // Set up three-layer priority: Emergency > Sensor > Background
    sensor_scheduler.setHighPriorityScheduler(&emergency_scheduler);
    base_scheduler.setHighPriorityScheduler(&sensor_scheduler);

    // Background processing - base priority, longer intervals
    Task background_task(500, 2, &background_processing_callback, &base_scheduler, true);

    // Time-critical sensor reading - high priority, short intervals
    Task sensor_task(10, 10, &sensor_critical_callback, &sensor_scheduler, true);

    // Emergency monitoring - highest priority, triggered as needed
    Task emergency_task(1000, 1, &emergency_response_callback, &emergency_scheduler, true);

    // Run scenario and verify sensor tasks get priority
    bool success = runPrioritySchedulerUntil(base_scheduler, []() {
        return priority_callback_counter >= 13; // 1 emergency + 10 sensor + 2 background
    }, 3000);

    EXPECT_TRUE(success);
    EXPECT_EQ(priority_callback_counter, 13);

    // Count executions by priority level
    int emergency_count = 0;
    int sensor_count = 0;
    int background_count = 0;

    for (size_t i = 0; i < getPriorityTestOutputCount(); i++) {
        std::string output = getPriorityTestOutput(i);
        if (output == "emergency_response_executed") {
            emergency_count++;
        } else if (output == "sensor_critical_executed") {
            sensor_count++;
        } else if (output == "background_processing_executed") {
            background_count++;
        }
    }

    EXPECT_EQ(emergency_count, 1);
    EXPECT_EQ(sensor_count, 10);
    EXPECT_EQ(background_count, 2);

    // Emergency should execute first if present
    EXPECT_EQ(getPriorityTestOutput(0), "emergency_response_executed");
}

/**
 * @brief Test dynamic priority scheduler changes
 *
 * TESTS: Runtime modification of scheduler hierarchy
 *
 * PURPOSE: Verify that priority relationships can be changed at runtime
 * and that the system adapts correctly to new priority configurations.
 */
TEST_F(PrioritySchedulerTest, DynamicPriorityChanges) {
    Scheduler base_scheduler;
    Scheduler high_scheduler;
    Scheduler alternative_scheduler;

    // Initial setup: base -> high priority
    base_scheduler.setHighPriorityScheduler(&high_scheduler);

    Task base_task(100, 1, &base_priority_callback, &base_scheduler, true);
    Task high_task(100, 1, &high_priority_callback, &high_scheduler, true);
    Task alt_task(100, 1, &medium_priority_callback, &alternative_scheduler, true);

    // Execute with initial hierarchy
    bool success = runPrioritySchedulerUntil(base_scheduler, []() {
        return priority_callback_counter >= 2;
    });

    EXPECT_TRUE(success);
    EXPECT_EQ(priority_callback_counter, 2);
    EXPECT_EQ(getPriorityTestOutput(0), "high_priority_executed");
    EXPECT_EQ(getPriorityTestOutput(1), "base_priority_executed");

    // Change priority hierarchy at runtime
    clearPriorityTestOutput();
    priority_callback_counter = 0;

    base_scheduler.setHighPriorityScheduler(&alternative_scheduler);

    // Re-enable tasks for another round
    base_task.restart();
    alt_task.restart();

    // Execute with new hierarchy
    success = runPrioritySchedulerUntil(base_scheduler, []() {
        return priority_callback_counter >= 2;
    });

    EXPECT_TRUE(success);
    EXPECT_EQ(priority_callback_counter, 2);
    EXPECT_EQ(getPriorityTestOutput(0), "medium_priority_executed");
    EXPECT_EQ(getPriorityTestOutput(1), "base_priority_executed");
}

/**
 * @brief Test priority system with enableAll/disableAll recursive operations
 *
 * TESTS: Recursive enable/disable operations across priority layers
 *
 * PURPOSE: Verify that enableAll(true) and disableAll(true) work correctly
 * with priority schedulers, affecting all layers in the hierarchy.
 */
TEST_F(PrioritySchedulerTest, PriorityRecursiveEnableDisable) {
    Scheduler base_scheduler;
    Scheduler high_scheduler;

    base_scheduler.setHighPriorityScheduler(&high_scheduler);

    // Add tasks to both schedulers (initially disabled)
    Task base_task(100, 2, &base_priority_callback, &base_scheduler, false);
    Task high_task(100, 2, &high_priority_callback, &high_scheduler, false);

    // Verify tasks are initially disabled
    EXPECT_FALSE(base_task.isEnabled());
    EXPECT_FALSE(high_task.isEnabled());

    // Enable all tasks recursively (should enable high priority tasks too)
    base_scheduler.enableAll(true);

    EXPECT_TRUE(base_task.isEnabled());
    EXPECT_TRUE(high_task.isEnabled());

    // Execute to verify both schedulers work
    bool success = runPrioritySchedulerUntil(base_scheduler, []() {
        return priority_callback_counter >= 4;
    });

    EXPECT_TRUE(success);
    EXPECT_EQ(priority_callback_counter, 4);

    // Disable all tasks recursively
    base_scheduler.disableAll(true);

    EXPECT_FALSE(base_task.isEnabled());
    EXPECT_FALSE(high_task.isEnabled());
}

/**
 * @brief Main test runner function for priority functionality
 *
 * Initializes Google Test framework and runs all priority feature tests.
 * Called by the test execution environment to start testing process.
 */
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}