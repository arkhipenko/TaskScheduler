// test_scheduler.cpp
#include <gtest/gtest.h>
#include "Arduino.h"
#include "TaskScheduler.h"

// Define the global test output vector
std::vector<std::string> test_output;

// Test callback functions
void task1_callback() {
    test_output.push_back("Task1 executed");
    std::cout << "Task1 executed at " << millis() << "ms" << std::endl;
}

void task2_callback() {
    test_output.push_back("Task2 executed");
    std::cout << "Task2 executed at " << millis() << "ms" << std::endl;
}

void task3_callback() {
    test_output.push_back("Task3 executed");
    std::cout << "Task3 executed at " << millis() << "ms" << std::endl;
}

void repeating_callback() {
    static int counter = 0;
    counter++;
    test_output.push_back("Repeating task #" + std::to_string(counter));
    std::cout << "Repeating task #" << counter << " executed at " << millis() << "ms" << std::endl;
}

class SchedulerTest : public ::testing::Test {
protected:
    void SetUp() override {
        clearTestOutput();
        // Reset time by creating new static start point
        millis(); // Initialize timing
    }
    
    void TearDown() override {
        clearTestOutput();
    }
    
    // Helper to run scheduler until condition is met or timeout
    bool runSchedulerUntil(Scheduler& ts, std::function<bool()> condition, unsigned long timeout_ms = 1000) {
        return waitForCondition([&]() {
            ts.execute();
            return condition();
        }, timeout_ms);
    }
};

TEST_F(SchedulerTest, BasicSchedulerCreation) {
    Scheduler ts;
    EXPECT_TRUE(true); // Scheduler creation should not throw
}

TEST_F(SchedulerTest, SchedulerInitialState) {
    Scheduler ts;
    
    // Execute empty scheduler - should not crash
    ts.execute();
    EXPECT_EQ(getTestOutputCount(), 0);
}

TEST_F(SchedulerTest, SingleTaskExecution) {
    Scheduler ts;
    
    // Create a task that runs once after 100ms
    Task task1(100, 1, &task1_callback, &ts, true);
    
    // Run scheduler until task executes
    bool success = runSchedulerUntil(ts, []() { return getTestOutputCount() >= 1; });
    
    EXPECT_TRUE(success) << "Task did not execute within timeout";
    EXPECT_EQ(getTestOutputCount(), 1);
    EXPECT_EQ(getTestOutput(0), "Task1 executed");
}

TEST_F(SchedulerTest, MultipleTaskExecution) {
    Scheduler ts;
    
    // Create multiple tasks with different intervals
    Task task1(50, 1, &task1_callback, &ts, true);   // Run once after 50ms
    Task task2(100, 1, &task2_callback, &ts, true);  // Run once after 100ms
    Task task3(150, 1, &task3_callback, &ts, true);  // Run once after 150ms
    
    // Run scheduler until all tasks execute
    bool success = runSchedulerUntil(ts, []() { return getTestOutputCount() >= 3; });
    
    EXPECT_TRUE(success) << "Not all tasks executed within timeout";
    EXPECT_EQ(getTestOutputCount(), 3);
    EXPECT_EQ(getTestOutput(0), "Task1 executed");
    EXPECT_EQ(getTestOutput(1), "Task2 executed");
    EXPECT_EQ(getTestOutput(2), "Task3 executed");
}

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

TEST_F(SchedulerTest, SchedulerWithNoTasks) {
    Scheduler ts;
    
    // Execute scheduler with no tasks multiple times
    for (int i = 0; i < 100; i++) {
        ts.execute();
        delay(1);
    }
    
    EXPECT_EQ(getTestOutputCount(), 0);
}

TEST_F(SchedulerTest, TaskExecutionOrder) {
    Scheduler ts;
    
    // Create tasks that should execute in specific order
    Task task_late(200, 1, &task3_callback, &ts, true);  // Latest
    Task task_early(50, 1, &task1_callback, &ts, true);  // Earliest  
    Task task_mid(100, 1, &task2_callback, &ts, true);   // Middle
    
    // Run until all execute
    bool success = runSchedulerUntil(ts, []() { return getTestOutputCount() >= 3; });
    
    EXPECT_TRUE(success);
    EXPECT_EQ(getTestOutputCount(), 3);
    
    // Tasks should execute in chronological order regardless of creation order
    EXPECT_EQ(getTestOutput(0), "Task1 executed");  // First (50ms)
    EXPECT_EQ(getTestOutput(1), "Task2 executed");  // Second (100ms)
    EXPECT_EQ(getTestOutput(2), "Task3 executed");  // Third (200ms)
}

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

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}