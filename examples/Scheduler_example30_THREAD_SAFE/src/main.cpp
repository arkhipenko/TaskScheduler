/*
 * TaskScheduler Example 30: _TASK_THREAD_SAFE
 *
 * This example demonstrates the use of the _TASK_THREAD_SAFE compile option
 * for safe multi-threaded operation with TaskScheduler.
 *
 * Features demonstrated:
 * 1. FreeRTOS queue implementation for thread-safe request handling
 * 2. Task control from hardware timer ISR
 * 3. Task control from FreeRTOS worker thread
 * 4. StatusRequest signaling from ISR
 * 5. Safe parameter updates from different contexts
 *
 * Hardware: ESP32 (or any platform with FreeRTOS support)
 *
 * Expected behavior:
 * - LED blinks at varying rates
 * - Button press triggers immediate blink from ISR
 * - Worker thread adjusts blink rate periodically
 * - All operations are thread-safe via request queue
 */

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

// Must define _TASK_THREAD_SAFE before including TaskScheduler
// (or define it in platformio.ini build_flags)
#include <TaskSchedulerDeclarations.h>

// ============================================
// Configuration
// ============================================
#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif

#define LED_PIN             LED_BUILTIN
#define BUTTON_PIN          0              // Boot button on most ESP32 boards

#define TS_QUEUE_LEN        16             // Task request queue length
#define TS_ENQUEUE_WAIT_MS  10             // Max wait time for enqueue (ms)
#define TS_DEQUEUE_WAIT_MS  0              // No wait for dequeue

// ============================================
// Global Variables
// ============================================

// Task Scheduler queue (for thread-safe operations)
QueueHandle_t tsQueue;
uint8_t tsQueueData[TS_QUEUE_LEN * sizeof(_task_request_t)];
StaticQueue_t tsQueueBuffer;

// Scheduler
Scheduler ts;

// Status request for ISR signaling
StatusRequest buttonPressed;

// Task counters
volatile uint32_t blinkCount = 0;
volatile uint32_t isrTriggers = 0;
volatile uint32_t threadUpdates = 0;

// ============================================
// Task Queue Implementation (Required for _TASK_THREAD_SAFE)
// ============================================

/**
 * Enqueue a task request (called from any thread/ISR)
 * This function is required when _TASK_THREAD_SAFE is enabled
 */
bool _task_enqueue_request(_task_request_t* req) {
  if (xPortInIsrContext()) {
    // Called from ISR context
    BaseType_t xHigherPriorityTaskWokenByPost = pdFALSE;
    BaseType_t rc = xQueueSendFromISR(tsQueue, req, &xHigherPriorityTaskWokenByPost);
    if (xHigherPriorityTaskWokenByPost) {
      portYIELD_FROM_ISR();
    }
    return (rc == pdTRUE);
  }
  else {
    // Called from normal task context
    return (xQueueSend(tsQueue, req, pdMS_TO_TICKS(TS_ENQUEUE_WAIT_MS)) == pdTRUE);
  }
}

/**
 * Dequeue a task request (called by scheduler)
 * This function is required when _TASK_THREAD_SAFE is enabled
 */
bool _task_dequeue_request(_task_request_t* req) {
  if (xPortInIsrContext()) {
    // Called from ISR context (shouldn't normally happen)
    BaseType_t xHigherPriorityTaskWokenByPost = pdFALSE;
    BaseType_t rc = xQueueReceiveFromISR(tsQueue, req, &xHigherPriorityTaskWokenByPost);
    if (xHigherPriorityTaskWokenByPost) {
      portYIELD_FROM_ISR();
    }
    return (rc == pdTRUE);
  }
  else {
    // Called from normal task context
    return (xQueueReceive(tsQueue, req, TS_DEQUEUE_WAIT_MS) == pdTRUE);
  }
}

// ============================================
// Task Callbacks
// ============================================

/**
 * LED blink task callback
 */
void blinkCallback() {
  digitalWrite(LED_PIN, !digitalRead(LED_PIN));
  blinkCount++;

  if (blinkCount % 10 == 0) {
    Serial.printf("[%lu] Blink #%lu (ISR triggers: %lu, Thread updates: %lu)\n",
                  millis(), blinkCount, isrTriggers, threadUpdates);
  }
}

Task tBlink(500, TASK_FOREVER, &blinkCallback, &ts, true);

/**
 * Fast blink task (triggered by button press)
 */
void fastBlinkCallback() {
  digitalWrite(LED_PIN, !digitalRead(LED_PIN));
}

Task tFastBlink(100, 10, &fastBlinkCallback, &ts, false);

/**
 * Statistics task callback
 */
void statsCallback() {
  Serial.printf("\n=== Statistics at %lu ms ===\n", millis());
  Serial.printf("Total blinks: %lu\n", blinkCount);
  Serial.printf("ISR triggers: %lu\n", isrTriggers);
  Serial.printf("Thread updates: %lu\n", threadUpdates);
  Serial.printf("Current blink interval: %lu ms\n", tBlink.getInterval());

#ifdef _TASK_TIMECRITICAL
  if (ts.isOverrun()) {
    Serial.println("WARNING: Scheduler overrun detected!");
  }
#endif

  Serial.println("================================\n");
}

Task tStats(5000, TASK_FOREVER, &statsCallback, &ts, true);

// ============================================
// ISR Handlers
// ============================================

/**
 * Button ISR - triggers fast blink using thread-safe requestAction
 */
void IRAM_ATTR buttonISR() {
  // Signal the status request from ISR
  // This is thread-safe via requestAction
  ts.requestAction(&buttonPressed, TASK_SR_REQUEST_SIGNALCOMPLETE, 0, 0, 0, 0, 0);
  isrTriggers++;
}

/**
 * Timer ISR - adjusts blink rate using thread-safe requestAction
 */
void IRAM_ATTR timerISR() {
  // Change the blink interval from ISR (thread-safe)
  static uint32_t nextInterval = 1000;

  ts.requestAction(&tBlink, TASK_REQUEST_SETINTERVAL, nextInterval, 0, 0, 0, 0);

  // Alternate between fast and slow
  nextInterval = (nextInterval == 1000) ? 200 : 1000;
}

// ============================================
// Worker Thread
// ============================================

/**
 * Background worker thread that adjusts task parameters
 * Demonstrates safe task manipulation from another FreeRTOS task
 */
void workerThread(void* parameter) {
  uint32_t counter = 0;

  while (true) {
    vTaskDelay(pdMS_TO_TICKS(3000));  // Run every 3 seconds

    counter++;

    // Safely adjust the blink interval from worker thread
    uint32_t newInterval = 300 + (counter % 5) * 100;  // 300-700ms range

    // Use requestAction for thread-safe operation
    bool success = ts.requestAction(&tBlink, TASK_REQUEST_SETINTERVAL, newInterval, 0, 0, 0, 0);

    if (success) {
      threadUpdates++;
    } else {
      Serial.println("ERROR: Failed to enqueue request (queue full?)");
    }
  }
}

// ============================================
// Status Request Task
// ============================================

/**
 * Task that waits for button press signal
 */
void buttonWatchCallback() {
  Serial.println("Button pressed! Starting fast blink sequence...");

  // Restart the fast blink task
  // Direct call is safe because we're in scheduler context
  tFastBlink.restart();
}

Task tButtonWatch(TASK_IMMEDIATE, TASK_FOREVER, &buttonWatchCallback, &ts, false);

// ============================================
// Setup and Main Loop
// ============================================

void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.println("\n\n===========================================");
  Serial.println("TaskScheduler Example 30: _TASK_THREAD_SAFE");
  Serial.println("===========================================\n");

  // Initialize LED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // Initialize button with pull-up
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Create the task request queue
  tsQueue = xQueueCreateStatic(
    TS_QUEUE_LEN,
    sizeof(_task_request_t),
    tsQueueData,
    &tsQueueBuffer
  );

  if (tsQueue == NULL) {
    Serial.println("ERROR: Failed to create task queue!");
    while (1) delay(1000);
  }

  Serial.printf("Task request queue created (size: %d)\n", TS_QUEUE_LEN);

  // Set up button watch task to wait for button press signal
  tButtonWatch.waitFor(&buttonPressed);

  // Attach button interrupt
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), buttonISR, FALLING);
  Serial.println("Button interrupt attached (press boot button)");

  // Create timer for periodic ISR (every 7 seconds)
  hw_timer_t* timer = timerBegin(0, 80, true);  // Timer 0, prescaler 80 (1MHz), count up
  timerAttachInterrupt(timer, &timerISR, true);
  timerAlarmWrite(timer, 7000000, true);  // 7 seconds in microseconds, auto-reload
  timerAlarmEnable(timer);
  Serial.println("Hardware timer started (triggers every 7 seconds)");

  // Create worker thread
  xTaskCreate(
    workerThread,
    "WorkerThread",
    4096,
    NULL,
    1,  // Priority
    NULL
  );
  Serial.println("Worker thread created");

  Serial.println("\n--- System initialized ---");
  Serial.println("Expected behavior:");
  Serial.println("1. LED blinks at variable rate");
  Serial.println("2. Press button for fast blink burst");
  Serial.println("3. Timer ISR changes rate every 7s");
  Serial.println("4. Worker thread adjusts rate every 3s");
  Serial.println("5. All operations are thread-safe!\n");

  delay(1000);

  // tBlink and tStats were enabled when they were created. Start them now
  // instead of letting them "catch up" on the time spent in setup().
  ts.startNow();
}

void loop() {
  // Run the scheduler
  // It will automatically process queued requests via processRequests()
  ts.execute();

  // Small delay to prevent watchdog issues
  delay(1);
}
