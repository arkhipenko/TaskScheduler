# Example 30: _TASK_THREAD_SAFE - Multi-threaded TaskScheduler

## Overview

This example demonstrates the use of the `_TASK_THREAD_SAFE` compile option for safe multi-threaded operation with TaskScheduler on FreeRTOS-based platforms (ESP32, ESP8266, STM32, etc.).

## What it Demonstrates

### 1. Thread-Safe Request Queue
- Implementation of required `_task_enqueue_request()` and `_task_dequeue_request()` functions
- FreeRTOS queue for handling task control requests from multiple threads
- Automatic handling of ISR vs. normal task context

### 2. ISR-Safe Operations
- Button interrupt triggering StatusRequest signals
- Hardware timer ISR changing task parameters
- Proper use of `requestAction()` from interrupt context

### 3. Multi-Thread Task Control
- FreeRTOS worker thread adjusting task parameters
- Safe task manipulation from non-scheduler threads
- Queue-based communication between threads

### 4. StatusRequest Integration
- Task waiting on StatusRequest completion
- Signaling from ISR context
- Automatic task activation on event

## Hardware Requirements

- **ESP32** development board (recommended), Arduino-ESP32 core 2.x or 3.x (the hardware timer code selects the API for the installed core)
- Built-in LED (LED_BUILTIN)
- Boot button (GPIO 0 on most boards)

Also compatible with:
- ESP8266
- STM32 with FreeRTOS
- Any Arduino-compatible board with FreeRTOS support

## Expected Behavior

1. **LED Blinking**: LED blinks at varying rates (controlled by worker thread and timer ISR)
2. **Button Press**: Pressing the boot button triggers a fast blink burst
3. **Timer Events**: Every 7 seconds, hardware timer toggles blink rate
4. **Thread Updates**: Worker thread changes blink interval every 3 seconds
5. **Statistics**: Serial output shows counters and current state every 5 seconds

## Key Components

### Queue Implementation

```cpp
// Required global queue
QueueHandle_t tsQueue;
uint8_t tsQueueData[TS_QUEUE_LEN * sizeof(_task_request_t)];
StaticQueue_t tsQueueBuffer;

// Required enqueue function
bool _task_enqueue_request(_task_request_t* req) {
  if (xPortInIsrContext()) {
    // ISR context
    BaseType_t xHigherPriorityTaskWokenByPost = pdFALSE;
    BaseType_t rc = xQueueSendFromISR(tsQueue, req, &xHigherPriorityTaskWokenByPost);
    if (xHigherPriorityTaskWokenByPost) portYIELD_FROM_ISR();
    return (rc == pdTRUE);
  }
  else {
    // Normal task context
    return (xQueueSend(tsQueue, req, pdMS_TO_TICKS(TS_ENQUEUE_WAIT_MS)) == pdTRUE);
  }
}

// Required dequeue function
bool _task_dequeue_request(_task_request_t* req) {
  return (xQueueReceive(tsQueue, req, TS_DEQUEUE_WAIT_MS) == pdTRUE);
}
```

### Safe Task Control from ISR

```cpp
void IRAM_ATTR buttonISR() {
  // Signal StatusRequest from ISR (thread-safe)
  ts.requestAction(&buttonPressed, TASK_SR_REQUEST_SIGNALCOMPLETE, 0, 0, 0, 0, 0);
  isrTriggers++;
}

void IRAM_ATTR timerISR() {
  // Change task interval from ISR (thread-safe)
  ts.requestAction(&tBlink, TASK_REQUEST_SETINTERVAL, newInterval, 0, 0, 0, 0);
}
```

### Safe Task Control from Worker Thread

```cpp
void workerThread(void* parameter) {
  while (true) {
    vTaskDelay(pdMS_TO_TICKS(3000));

    // Safely adjust blink interval from worker thread
    uint32_t newInterval = 300 + (counter % 5) * 100;
    ts.requestAction(&tBlink, TASK_REQUEST_SETINTERVAL, newInterval, 0, 0, 0, 0);
  }
}
```

## Building and Running

### PlatformIO

```bash
# Navigate to example directory
cd lib/TaskScheduler/examples/Scheduler_example30_THREAD_SAFE

# Build and upload
pio run -t upload

# Monitor serial output
pio device monitor
```

### Arduino IDE

1. Open `src/main.cpp` as a `.ino` file
2. Install TaskScheduler library (version 3.8.0 or newer)
3. Select ESP32 board
4. Add to Arduino IDE preprocessor defines:
   ```
   -D_TASK_THREAD_SAFE
   -D_TASK_STATUS_REQUEST
   -D_TASK_TIMEOUT
   -D_TASK_ISR_SUPPORT
   ```
5. Compile and upload

## Serial Output Example

```
===========================================
TaskScheduler Example 30: _TASK_THREAD_SAFE
===========================================

Task request queue created (size: 16)
Button interrupt attached (press boot button)
Hardware timer started (triggers every 7 seconds)
Worker thread created

--- System initialized ---
Expected behavior:
1. LED blinks at variable rate
2. Press button for fast blink burst
3. Timer ISR changes rate every 7s
4. Worker thread adjusts rate every 3s
5. All operations are thread-safe!

[5123] Blink #10 (ISR triggers: 2, Thread updates: 1)
Button pressed! Starting fast blink sequence...

=== Statistics at 10000 ms ===
Total blinks: 45
ISR triggers: 2
Thread updates: 3
Current blink interval: 400 ms
================================
```

## Configuration

Adjust these constants in `main.cpp`:

```cpp
#define TS_QUEUE_LEN        16    // Request queue size (increase if overflow occurs)
#define TS_ENQUEUE_WAIT_MS  10    // Max wait for enqueue (ms)
#define TS_DEQUEUE_WAIT_MS  0     // No wait for dequeue
```

## Troubleshooting

### Queue Overflow
If you see "Failed to enqueue request" messages:
- Increase `TS_QUEUE_LEN`
- Increase `TS_ENQUEUE_WAIT_MS`
- Reduce request frequency

### Watchdog Timeouts
If the ESP32 reboots with watchdog errors:
- Ensure `ts.execute()` runs frequently in loop()
- Check that no task blocks for too long
- Reduce worker thread priority

### Compilation Errors
Ensure all compile flags are set in `platformio.ini`:
- `_TASK_THREAD_SAFE` (required)
- `_TASK_STATUS_REQUEST` (for button example)
- `_TASK_ISR_SUPPORT` (for ESP ISR methods)

## Learning Points

### ✅ DO:
- Always use `requestAction()` from ISRs and worker threads
- Implement both enqueue and dequeue functions
- Size the queue based on request frequency
- Mark ISR functions with `IRAM_ATTR` (ESP32/ESP8266)

### ❌ DON'T:
- Call task methods directly from ISRs or other threads
- Skip queue implementation (required for _TASK_THREAD_SAFE)
- Forget to process requests (automatically done in `execute()`)
- Block in ISRs waiting for queue space

## Related Examples

- **Example 27**: PlatformIO project structure
- **Example 28**: Tickless operation with FreeRTOS
- **Example 04/05**: StatusRequest basics

## Further Reading

- [TaskScheduler Wiki](https://github.com/arkhipenko/TaskScheduler/wiki)
- TaskScheduler_THREAD_SAFE.md in your project root
- FreeRTOS queue documentation
- ESP32 interrupt handling

## License

Same as TaskScheduler library (see library LICENSE file)

## Version

Created for TaskScheduler v3.8.0+
Updated for TaskScheduler v4.0.0 (October 2024)
