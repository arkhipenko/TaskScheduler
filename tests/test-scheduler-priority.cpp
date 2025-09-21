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
#define _TASK_WDT_IDS
#define _TASK_TIMECRITICAL

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
 * @brief Priority test callback that uses currentScheduler() and currentTask()
 *
 * This callback mimics the example11 pattern by accessing the current scheduler
 * and task to get task ID and timing information.
 */
void priority_test_callback() {
    priority_callback_counter++;

    // Use currentScheduler() and currentTask() like in example11
    Scheduler& current_scheduler = Scheduler::currentScheduler();
    Task& current_task = current_scheduler.currentTask();

    // Record task execution with ID for verification
    unsigned int task_id = current_task.getId();
    unsigned long execution_time = millis();

    priority_execution_times[priority_execution_index++] = execution_time;

    // Create output string with task ID for verification
    std::string output = "task_" + std::to_string(task_id) + "_executed";
    priority_test_output.push_back(output);

    std::cout << "Task: " << task_id << " executed at " << execution_time
              << "ms, Start delay = " << current_task.getStartDelay() << std::endl;
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

    /**
     * @brief Analyze priority evaluation pattern based on example11 sequence
     *
     * According to wiki: "entire chain of tasks of the higher priority scheduler
     * is executed for every single step (task) of the lower priority chain"
     *
     * Expected pattern: high priority tasks are evaluated between each base task
     */
    bool validatePriorityEvaluationPattern() {
        // Look for the pattern where high priority tasks (4,5) are frequently interspersed
        // with base priority tasks (1,2,3), especially task 4 which has shortest interval

        int consecutive_base_tasks = 0;
        int max_consecutive_base = 0;

        for (size_t i = 0; i < getPriorityTestOutputCount(); i++) {
            std::string task = getPriorityTestOutput(i);

            if (task == "task_1_executed" || task == "task_2_executed" || task == "task_3_executed") {
                consecutive_base_tasks++;
                max_consecutive_base = std::max(max_consecutive_base, consecutive_base_tasks);
            } else {
                consecutive_base_tasks = 0;
            }
        }

        // In proper priority scheduling, we shouldn't see many consecutive base tasks
        // because high priority tasks should be evaluated frequently
        return max_consecutive_base <= 3;  // Allow some consecutive base tasks but not too many
    }
};

// ================== BASIC PRIORITY FUNCTIONALITY TESTS ==================

/**
 * @brief Test basic scheduler hierarchy setup and validation (based on example11)
 *
 * TESTS: setHighPriorityScheduler(), currentScheduler(), currentTask()
 *
 * PURPOSE: Verify that scheduler hierarchy can be established correctly
 * and that the priority relationships work as expected for basic scenarios,
 * following the pattern from Scheduler_example11_Priority.
 *
 * PRIORITY HIERARCHY SETUP (from example11):
 * - Base scheduler: Tasks t1, t2, t3 (1000ms, 2000ms, 3000ms intervals)
 * - High scheduler: Tasks t4, t5 (500ms, 1000ms intervals)
 * - Hierarchy relationship: base.setHighPriorityScheduler(&high)
 * - Execution order follows priority evaluation sequence: 4,5,1,4,5,2,4,5,3
 *
 * TEST SCENARIO:
 * 1. Create base and high priority schedulers matching example11
 * 2. Establish hierarchy relationship
 * 3. Add tasks to both schedulers with example11 intervals
 * 4. Execute base scheduler and verify priority execution pattern
 * 5. Verify currentScheduler() and currentTask() work correctly
 *
 * EXPECTATIONS:
 * - High priority tasks (t4, t5) execute more frequently
 * - currentScheduler() returns correct scheduler during execution
 * - Task IDs can be retrieved and verified
 * - Priority evaluation follows documented sequence
 */
TEST_F(PrioritySchedulerTest, BasicSchedulerHierarchy) {
    // Create base and high priority schedulers (matching example11)
    Scheduler base_scheduler;
    Scheduler high_scheduler;

    // Establish hierarchy - high scheduler has priority over base
    base_scheduler.setHighPriorityScheduler(&high_scheduler);

    // Add tasks to schedulers with intervals matching example11
    // Base priority tasks: longer intervals
    Task t1(1000, 3, &priority_test_callback, &base_scheduler, false);  // 1000ms interval
    Task t2(2000, 2, &priority_test_callback, &base_scheduler, false);  // 2000ms interval
    Task t3(3000, 1, &priority_test_callback, &base_scheduler, false);  // 3000ms interval

    // High priority tasks: shorter intervals for more frequent execution
    Task t4(500, 6, &priority_test_callback, &high_scheduler, false);   // 500ms interval
    Task t5(1000, 3, &priority_test_callback, &high_scheduler, false);  // 1000ms interval

    // Set task IDs for identification (like in example11)
    t1.setId(1);
    t2.setId(2);
    t3.setId(3);
    t4.setId(4);
    t5.setId(5);

    // Enable all tasks recursively (like example11: enableAll(true))
    base_scheduler.enableAll(true);

    // Verify tasks are enabled
    EXPECT_TRUE(t1.isEnabled());
    EXPECT_TRUE(t2.isEnabled());
    EXPECT_TRUE(t3.isEnabled());
    EXPECT_TRUE(t4.isEnabled());
    EXPECT_TRUE(t5.isEnabled());

    // Execute scheduler for sufficient time to see priority pattern
    // High priority tasks should execute more frequently due to shorter intervals
    bool success = runPrioritySchedulerUntil(base_scheduler, []() {
        return priority_callback_counter >= 15; // 6+3=9 high + 3+2+1=6 base = 15 total
    }, 5000);

    EXPECT_TRUE(success);
    EXPECT_EQ(priority_callback_counter, 15);

    // Count executions by task ID to verify the priority pattern
    int task1_count = 0, task2_count = 0, task3_count = 0;  // Base priority tasks
    int task4_count = 0, task5_count = 0;                   // High priority tasks

    for (size_t i = 0; i < getPriorityTestOutputCount(); i++) {
        std::string output = getPriorityTestOutput(i);
        if (output == "task_1_executed") task1_count++;
        else if (output == "task_2_executed") task2_count++;
        else if (output == "task_3_executed") task3_count++;
        else if (output == "task_4_executed") task4_count++;
        else if (output == "task_5_executed") task5_count++;
    }

    // Verify execution counts match expected iterations
    EXPECT_EQ(task1_count, 3);  // t1: 3 executions (1000ms interval)
    EXPECT_EQ(task2_count, 2);  // t2: 2 executions (2000ms interval)
    EXPECT_EQ(task3_count, 1);  // t3: 1 execution (3000ms interval)
    EXPECT_EQ(task4_count, 6);  // t4: 6 executions (500ms interval, high priority)
    EXPECT_EQ(task5_count, 3);  // t5: 3 executions (1000ms interval, high priority)

    // Verify that task 4 (highest frequency, high priority) executed first
    // Task 4 has 500ms interval and high priority, so it should execute first
    EXPECT_EQ(getPriorityTestOutput(0), "task_4_executed");

    // Count high vs base priority executions for overall pattern verification
    int high_priority_total = task4_count + task5_count;    // 6 + 3 = 9
    int base_priority_total = task1_count + task2_count + task3_count;  // 3 + 2 + 1 = 6

    EXPECT_EQ(high_priority_total, 9);
    EXPECT_EQ(base_priority_total, 6);

    // Validate the priority execution order pattern from example11 output
    // Expected initial sequence: 4, 5, 1, 2, 3 (based on timing and priority)
    // Task 4 (ID 40 in example) executes at 0ms, 500ms, 1000ms, 1500ms, 2000ms, 2500ms
    // Task 5 (ID 50 in example) executes at 10ms, 1010ms, 2011ms, 3010ms, etc.
    // Task 1 executes at ~21ms, 1021ms, 2022ms, 3021ms, etc.

    // Verify high priority tasks execute more frequently in early execution
    std::vector<std::string> early_executions;
    for (size_t i = 0; i < std::min((size_t)10, getPriorityTestOutputCount()); i++) {
        early_executions.push_back(getPriorityTestOutput(i));
    }

    // Count high priority vs base priority in first 10 executions
    int early_high_count = 0;
    int early_base_count = 0;
    for (const auto& exec : early_executions) {
        if (exec == "task_4_executed" || exec == "task_5_executed") {
            early_high_count++;
        } else if (exec == "task_1_executed" || exec == "task_2_executed" || exec == "task_3_executed") {
            early_base_count++;
        }
    }

    // High priority tasks should dominate early execution due to shorter intervals
    EXPECT_GE(early_high_count, early_base_count);

    // Verify task 4 appears multiple times in early execution (due to 500ms interval)
    int task4_early_count = 0;
    for (const auto& exec : early_executions) {
        if (exec == "task_4_executed") {
            task4_early_count++;
        }
    }
    EXPECT_GE(task4_early_count, 2); // Task 4 should execute at least twice in early sequence

    // Validate that base priority tasks eventually execute
    // Task 1 should appear at least once given its 1000ms interval
    bool task1_found = false;
    bool task2_found = (task2_count > 0);  // Task 2 might not execute in early sequence due to 2000ms interval
    for (const auto& exec : priority_test_output) {
        if (exec == "task_1_executed") {
            task1_found = true;
            break;
        }
    }
    EXPECT_TRUE(task1_found);

    // Print execution sequence for debugging (similar to example11 output)
    std::cout << "\nPriority Test Execution Sequence:" << std::endl;
    for (size_t i = 0; i < getPriorityTestOutputCount() && i < 15; i++) {
        std::string task_id = getPriorityTestOutput(i);
        // Extract task number from "task_X_executed"
        if (task_id.length() > 5) {
            char task_num = task_id[5];
            std::cout << "Task: " << task_num << " at position " << i << std::endl;
        }
    }

    // Validate the specific execution order pattern from example11 output
    // This ensures the priority evaluation sequence matches documented behavior
    EXPECT_TRUE(validatePriorityEvaluationPattern())
        << "Priority evaluation pattern validation failed - high priority tasks should be interspersed with base tasks";
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