// mock_arduino.h
#ifndef MOCK_ARDUINO_H
#define MOCK_ARDUINO_H

#include <chrono>
#include <thread>
#include <vector>
#include <string>
#include <iostream>

// Arduino constants
#define HIGH 1
#define LOW  0
#define INPUT 0
#define OUTPUT 1
#define LED_BUILTIN 13

// Mock Arduino timing functions
inline unsigned long millis() {
    static auto start = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();
}

inline unsigned long micros() {
    static auto start = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::microseconds>(now - start).count();
}

inline void delay(unsigned long ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

inline void delayMicroseconds(unsigned long us) {
    std::this_thread::sleep_for(std::chrono::microseconds(us));
}

// Mock digital I/O functions
inline void pinMode(int pin, int mode) {
    // Mock implementation - do nothing
    if ( pin == mode ) return;
    else return;
}

inline void digitalWrite(int pin, int value) {
    // Mock implementation - could log pin states if needed
    if ( pin == value ) return;
    else return;
}

inline int digitalRead(int pin) {
    // Mock implementation - return LOW by default
    if (pin == 0) return LOW;
    else return HIGH;
}

inline void yield() {
    std::this_thread::yield();
}

// Global test output capture
extern std::vector<std::string> test_output;

// Helper function to clear test output
inline void clearTestOutput() {
    test_output.clear();
}

// Helper function to get test output count
inline size_t getTestOutputCount() {
    return test_output.size();
}

// Helper function to get specific test output
inline std::string getTestOutput(size_t index) {
    if (index < test_output.size()) {
        return test_output[index];
    }
    return "";
}

// Helper function to wait for condition with timeout
template<typename Condition>
inline bool waitForCondition(Condition condition, unsigned long timeout_ms) {
    unsigned long start_time = millis();
    while (millis() - start_time < timeout_ms) {
        if (condition()) {
            return true;
        }
        delay(10);
    }
    return false;
}

#endif // MOCK_ARDUINO_H
