# TaskScheduler

### Cooperative multitasking for Arduino, ESPx, STM32 and other microcontrollers
#### Version 4.0.7: 2026-04-17

[![arduino-library-badge](https://www.ardu-badge.com/badge/TaskScheduler.svg?)](https://www.ardu-badge.com/TaskScheduler)
[![Unit Tests](https://github.com/arkhipenko/TaskScheduler/actions/workflows/test.yml/badge.svg)](https://github.com/arkhipenko/TaskScheduler/actions/workflows/test.yml)

[Latest updates](https://github.com/arkhipenko/TaskScheduler/wiki/Latest-Updates) | [Full documentation](https://arkhipenko.github.io/ts) | [Wiki](https://github.com/arkhipenko/TaskScheduler/wiki) | [Changelog](https://github.com/arkhipenko/TaskScheduler/wiki/Changelog)

#### Delivering robust embedded systems and firmware that perform flawlessly in real-world conditions. [smart solutions for smart devices](https://smart4smart.com/)

[![github](https://github.com/arkhipenko/resources/blob/master/smart4smart_large.gif)](https://smart4smart.com/)

---

### Overview

A lightweight implementation of cooperative multitasking (task scheduling). An easier alternative to preemptive programming and frameworks like FreeRTOS.

**Why cooperative?**

You mostly do not need to worry about pitfalls of concurrent processing (races, deadlocks, livelocks, resource sharing, etc.). The fact of cooperative processing takes care of such issues by design.

_"Everybody who learns concurrency and thinks they understand it, ends up finding mysterious races they thought weren't possible, and discovers that they didn't actually understand it yet after all."_ **Herb Sutter, chair of the ISO C++ standards committee, Microsoft.**

---

### Quick Start

```cpp
#include <TaskScheduler.h>

Scheduler runner;

// Blink LED every 500ms, read sensor every 2 seconds
Task taskBlink(500, TASK_FOREVER, &blinkLED);
Task taskSensor(2000, TASK_FOREVER, &readSensor);

void blinkLED() {
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
}

void readSensor() {
    Serial.println(analogRead(A0));
}

void setup() {
    Serial.begin(115200);
    pinMode(LED_BUILTIN, OUTPUT);

    runner.addTask(taskBlink);
    runner.addTask(taskSensor);
    taskBlink.enable();
    taskSensor.enable();
}

void loop() {
    runner.execute();
}
```

Try it live on [Wokwi](https://wokwi.com/playground/task-scheduler) -- no hardware required.

---

### Installation

**Arduino IDE:** Sketch -> Include Library -> Manage Libraries -> search for "TaskScheduler" -> Install

**PlatformIO:** Add to `platformio.ini`:
```ini
lib_deps = arkhipenko/TaskScheduler
```

**Manual:** Download or clone this repository into your Arduino `libraries/` folder.

---

### Features

**Core scheduling:**
1. Periodic task execution with dynamic period in `milliseconds` (default) or `microseconds`
2. Limited or infinite number of iterations
3. Execution of tasks in predefined sequence
4. Dynamic change of task execution parameters (frequency, iterations, callbacks)
5. Static and dynamic callback method binding
6. Support for `std::functions` (tested on `ESPx` and `STM32`)

**Power and timing:**
7. Power saving via **IDLE** sleep mode when tasks are not scheduled to run
8. Overall task timeout
9. CPU load / idle statistics for time-critical applications
10. Scheduling options: priority for original schedule (with and without catchup) and interval
11. Support for "tickless" execution under FreeRTOS (continuous sleep until next scheduled task invocation)

**Advanced:**
12. Event-driven task invocation via Status Request object
13. Task IDs and Control Points for error handling and watchdog timer
14. Local Task Storage pointer (allowing use of same callback code for multiple tasks)
15. Layered task prioritization
16. Ability to pause/resume and enable/disable scheduling
17. Thread-safe scheduling while running under preemptive scheduler (e.g., FreeRTOS)
18. Optional self-destruction of dynamically created tasks upon disable

**Platform support:**
19. Arduino IDE style (headers only) and PlatformIO style (header + CPP files)
20. Non-Arduino platforms

Scheduling overhead: between `15` and `18` microseconds per scheduling pass (Arduino UNO rev 3 @ `16MHz` clock, single scheduler w/o prioritization)

**Tested platforms:**
* Arduino Uno R3, Nano, Micro
* ATtiny85
* ESP8266, ESP32
* Teensy (tested on Teensy 3.5)
* nRF52832, nRF52 Adafruit Core (tested on nRF52840 with v3.6.2 workaround)
* STM32 (tested on Mini USB STM32F103RCBT6 ARM Cortex-M3 leaflabs Leaf maple mini module F)
* MSP430 and MSP432 boards
* Raspberry Pi (requires `_TASK_NON_ARDUINO` and `_task_millis()` implementation)
* Any Linux (requires `_TASK_NON_ARDUINO` and `_task_millis()` implementation -- that's how unit tests are done)

---
![TaskScheduler process diagram](https://github.com/arkhipenko/TaskScheduler/raw/master/extras/TaskScheduler_html.png)
---

### In the Wild

#### Commercial products

* [3devo](https://www.3devo.com/) -- Commercial polymer processing and 3D printing filament production systems. TaskScheduler runs in the firmware of their entire product line: Filament Maker ONE, Filament Maker TWO, GP20 Shredder Hybrid, and Airid Polymer Dryer. Each product's [license page](https://support.3devo.com/license-information) credits TaskScheduler (BSD3, Anatoli Arkhipenko). Open-source firmware at [github.com/3devo](https://github.com/3devo).
* [Hackabot Nano](https://www.kickstarter.com/projects/hackarobot/hackabot-nano-compact-plug-and-play-arduino-robot) by Funnyvale -- Compact plug-and-play Arduino-compatible robotic kit (Kickstarter)

#### painlessMesh and ESP32 mesh networking

TaskScheduler is a core dependency of [painlessMesh](https://gitlab.com/painlessMesh/painlessMesh) -- the widely adopted ESP8266/ESP32 mesh networking library. painlessMesh uses TaskScheduler internally to manage node discovery, connection maintenance, message routing, and mesh self-healing, all running cooperatively within a single `loop()` call.

This pairing of TaskScheduler + painlessMesh has been deployed across a range of real-world applications:

**Academic and industrial research:**

* **Air quality monitoring networks** -- Indoor/outdoor sensor mesh networks using ESP32 nodes with CO2, PM2.5, temperature, and humidity sensors, deployed across campus-scale areas (100m x 80m) and running continuously for days ([Sustainability, 2022](https://www.mdpi.com/2071-1050/14/24/16630))
* **Cold chain monitoring for perishable goods** -- PrunusPos project evaluating ESP8266 mesh testbeds for monitoring ambient conditions inside fruit and vegetable storage containers, with sensors integrated directly into transport crates ([Information, 2022](https://www.mdpi.com/2078-2489/13/5/210))
* **Mesh network performance evaluation** -- Multiple peer-reviewed papers benchmarking painlessMesh delivery delay, throughput, and scalability with varying node counts and payload sizes ([Yoppy et al., 2019](https://www.researchgate.net/publication/335656647_Performance_Evaluation_of_ESP8266_Mesh_Networks))

**Community and maker projects:**

* **Synchronized LED installations** -- WiFi mesh-synchronized NeoPixel LED bars with coordinated animations across multiple nodes, no data wires required ([Instructables](https://www.instructables.com/WiFi-Mesh-Synchronized-LED-Bars/))
* **MeshyMcLighting** -- Standalone mesh-networked NeoPixel lighting system broadcasting state across nodes without internet connectivity ([MeshyMcLighting](https://sites.lsa.umich.edu/debsahu/2018/05/20/meshymclighting-neopixels-lighting-solution-using-mesh-network/))
* **MQTT-to-Mesh gateways** -- Bridging painlessMesh IoT networks to MQTT brokers for cloud integration ([painlessMeshMqttGateway](https://github.com/latonita/painlessMeshMqttGateway))
* **Woodshop dust collector control** -- Distributed blast gate control across a workshop using mesh-networked ESP nodes
* **Smart agriculture and irrigation** -- Distributed soil moisture, temperature, and water flow monitoring across fields with a gateway node forwarding to cloud dashboards
* **Emergency/disaster communication** -- Pre-deployed ESP32 mesh networks that continue relaying status messages (water levels, power outages) when internet infrastructure is down

#### Community projects

* [Ninja Timer: Giant 7-Segment Display](https://learn.adafruit.com/ninja-timer-giant-7-segment-display/timer-code) -- Featured on Adafruit
* [NeoPixel smartBulb IoT](https://www.zerozone.it/linux-e-open-source/giocare-con-i-neopixel-per-realizzare-un-simpatico-smartbulb-iot/16760) -- DIY smart lamp with ESP8266 and motion sensing
* [Arduino Bluetooth remote control + ultrasonic anti-collision car](https://xie.infoq.cn/article/0f27dbbebcc2b99b35132b262)
* [Houston midi clock](https://github.com/chaffneue/houston) -- Multi-master MIDI controller with shared clock, TaskScheduler at microsecond resolution ([YouTube](https://www.youtube.com/watch?v=QRof550TtXo))
* [Discrete Time Systems Wiki](https://sistemas-en-tiempo-discreto.fandom.com/es/wiki/Tiempo_Real) -- Educational reference

#### My projects

* [Interactive "Do Not Disturb" Minecraft Sword sign](https://www.instructables.com/id/Interactive-Minecraft-Do-Not-Enter-SwordSign-ESP32/) (ESP32)
* [Interactive Predator Costume with Head Tracking Plasma Cannon](https://www.instructables.com/id/Interactive-Predator-Costume-With-Head-Tracking-Pl/) (Teensy, Arduino Nano)
* [IoT APIS v2 -- Automated Plant Irrigation System](https://www.instructables.com/id/IoT-APIS-V2-Autonomous-IoT-enabled-Automated-Plant/) (ESP8266)
* [APIS -- Automated Plant Irrigation System](https://www.instructables.com/id/APIS-Automated-Plant-Irrigation-System/) (Arduino Uno)
* [Party Lights LEDs music visualization](https://www.instructables.com/id/Portable-Party-Lights/) (Leaf Maple Mini)
* [Hexbug Scarab Robotic Spider](https://www.instructables.com/id/Arduino-Nano-based-Hexbug-Scarab-Robotic-Spider/) (Arduino Nano)
* [Wave your hand to control OWI Robotic Arm](https://www.instructables.com/id/Wave-your-hand-to-control-OWI-Robotic-Arm-no-strin/) (Arduino Uno and Nano)
* [Interactive Halloween Pumpkin](https://www.instructables.com/id/Interactive-Halloween-Pumpkin/) (Arduino Uno)

#### Video tutorial

* [TaskScheduler tutorial on YouTube](https://youtu.be/eoJUlH_rWOE?si=eatgXMDMzwLPXrVP)

---

### User Feedback

_"I've used TaskScheduler with great success. Running LED patterns, monitoring button presses, reading data from an accelerometer, auto advancing to the next pattern, reading data from Serial. All at the same time."_ -- [Reddit](https://www.reddit.com/r/FastLED/comments/b3rfzf/wanna_try_some_code_that_is_powerfuldangerous/)

_"You basically queue up a list of task callbacks and a schedule in your `setup()` and then do a call to `tasks.execute()` in `loop()`, which pops off the next task that is due in a queue or sleeps otherwise. It's simple, but much more straightforward than manually using `if millis() - last > delta1... else sleep()`"_ -- [Hacker News](https://news.ycombinator.com/item?id=14848906)

_"I encourage you to use it in the Arduino environment, it allows you to save a lot of time (and code lines) wherever you need to schedule, i.e. run many tasks that should perform at different frequencies and when we want to have the greatest control over the performance of these tasks and we want good diagnostic of errors."_ -- [elektroda.pl](https://www.elektroda.pl/rtvforum/topic3599980.html)

---

### Contributing

**Code:** As of version 4.0.0 TaskScheduler has a comprehensive set of compilation and unit tests. Please submit a PR with your changes and make sure that your code passes all the tests.

**Tests:** There is no such thing as enough testing. If you come up with another test scenario -- please contribute!

---

### License

BSD 3-Clause. See [LICENSE.txt](LICENSE.txt).

---
[![github](https://github.com/arkhipenko/resources/blob/master/smart4smart_hero_banner.gif)](https://smart4smart.com/)
