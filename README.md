# Task Scheduler
### Cooperative multitasking for Arduino, ESPx, STM32 and other microcontrollers
#### Version 3.8.5: 2024-06-17 [Latest updates](https://github.com/arkhipenko/TaskScheduler/wiki/Latest-Updates)

[![arduino-library-badge](https://www.ardu-badge.com/badge/TaskScheduler.svg?)](https://www.ardu-badge.com/TaskScheduler)[![xscode](https://img.shields.io/badge/Available%20on-xs%3Acode-blue?style=?style=plastic&logo=appveyor&logo=data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAEAAAABACAMAAACdt4HsAAAAGXRFWHRTb2Z0d2FyZQBBZG9iZSBJbWFnZVJlYWR5ccllPAAAAAZQTFRF////////VXz1bAAAAAJ0Uk5T/wDltzBKAAAAlUlEQVR42uzXSwqAMAwE0Mn9L+3Ggtgkk35QwcnSJo9S+yGwM9DCooCbgn4YrJ4CIPUcQF7/XSBbx2TEz4sAZ2q1RAECBAiYBlCtvwN+KiYAlG7UDGj59MViT9hOwEqAhYCtAsUZvL6I6W8c2wcbd+LIWSCHSTeSAAECngN4xxIDSK9f4B9t377Wd7H5Nt7/Xz8eAgwAvesLRjYYPuUAAAAASUVORK5CYII=)](https://xscode.com/arkhipenko/TaskScheduler)

#### Get expedited support or integration consultation for TaskScheduler [from xs:code](https://xscode.com/arkhipenko/TaskScheduler)

[![xscode](https://github.com/arkhipenko/resources/blob/master/taskscheduler-banner.png)](https://xscode.com/arkhipenko/TaskScheduler)
---

### OVERVIEW:
A lightweight implementation of cooperative multitasking (task scheduling). An easier alternative to preemptive programming and frameworks like FreeRTOS. 

**Why cooperative?**

You mostly do not need to worry about pitfalls of concurrent processing (races, deadlocks, livelocks, resource sharing, etc.).  The fact of cooperative processing takes care of such issues by design. 

_“Everybody who learns concurrency and thinks they understand it, ends up finding mysterious races they thought weren’t possible, and discovers that they didn’t actually understand it yet after all.”_ **Herb Sutter, chair of the ISO C++ standards committee, Microsoft.**

**Main features:**
1. Periodic task execution, with dynamic execution period in `milliseconds` (default) or `microseconds` (if explicitly enabled) – frequency of execution
2. Number of iterations (limited or infinite number of iterations)
3. Execution of tasks in predefined sequence
4. Dynamic change of task execution parameters (frequency, number of iterations, callback methods)
5. Power saving via entering **IDLE** sleep mode when tasks are not scheduled to run
6. Support for event-driven task invocation via Status Request object
7. Support for task IDs and Control Points for error handling and watchdog timer
8. Support for Local Task Storage pointer (allowing use of same callback code for multiple tasks)
9. Support for layered task prioritization
10. Support for `std::functions` (tested on `ESPx` and `STM32` only)
11. Overall task timeout
12. Static and dynamic callback method binding
13. CPU load / idle statistics for time critical applications
14. Scheduling options with priority for original schedule (with and without catchup) and interval
15. Ability to pause/resume and enable/disable scheduling
16. Thread-safe scheduling while running under preemptive scheduler (i. e., FreeRTOS)
17. Optional self-destruction of dynamically created tasks upon disable
18. Support for "tickless" execution under FreeRTOS (continous sleep until next scheduled task invocation)

Scheduling overhead: between `15` and `18` microseconds per scheduling pass (Arduino UNO rev 3 @ `16MHz` clock, single scheduler w/o prioritization)

**TaskScheduler** was tested on the following platforms:
* Arduino Uno R3
* Arduino Nano
* Arduino Micro
* ATtiny85
* ESP8266
* ESP32
* Teensy (tested on Teensy 3.5)
* nRF52 (tested on nRF52832)
* nRF52 Adafruit Core (tested on nRF52840 with v3.6.2 workround)
* STM32 (tested on Mini USB STM32F103RCBT6 ARM Cortex-M3 leaflabs Leaf maple mini module F)
* MSP430 and MSP432 boards
* Raspberry Pi (requires external `Arduino.h` and `millis()` implementation)



​                                                 **Don't just take my word for it - try it for yourself on [Wokwi](https://wokwi.com/playground/task-scheduler)**



---
![TaskScheduler process diagram](https://github.com/arkhipenko/TaskScheduler/raw/master/extras/TaskScheduler_html.png)
---
### Changelog is located [here.](https://github.com/arkhipenko/TaskScheduler/wiki/Changelog)


#### For detailed functionality overview please refer to TaskScheduler documentation in the 'extras' folder or in the [Wiki page](https://github.com/arkhipenko/TaskScheduler/wiki).

### User Feedback:

"I've used https://github.com/arkhipenko/TaskScheduler with great success. Running LED patterns, monitoring button presses, reading data from an accelerometer, auto advancing to the next pattern, reading data from Serial. All at the same time." - [here](https://www.reddit.com/r/FastLED/comments/b3rfzf/wanna_try_some_code_that_is_powerfuldangerous/)

"There are libraries that do this automatically on Arduino too, allowing you to schedule [cooperative] multitasking and sleep the uC between tasks. E.g. https://github.com/arkhipenko/TaskScheduler is really good, I've used it before. You basically queue up a list of task callbacks and a schedule in your `setup()` and then do a call to `tasks.execute()` in `loop()`, which pops off the next task that is due in a queue or sleeps otherwise. It's simple, but much more straightforward than manually using `if millis() - last > delta1... else sleep()` and not as rigid as using the timer ISRs (which really serve a different purpose)." - [here](https://news.ycombinator.com/item?id=14848906)

"I took the controller with me on a business trip and spend the night getting the basic code framework out. It is going to run on top of Arkhipenko’s TaskScheduler. (https://github.com/arkhipenko/TaskScheduler) This should help me isolate any issues between the different control systems while managing the different task’s timing requirements." - [here](https://hackaday.io/project/167479/logs)

"it's really cool and useful, for whenver you want your MCU to do more than 1 task" - [here](https://gitter.im/FastLED/public?at=5947e23dd83c50560c22d5b6)

"I encourage you to use it in the Arduino environment, it allows you to save a lot of time (and code lines) wherever you need to schedule, i.e. run many tasks that should to perform at different frequencies and when we want to have the greatest control over the performance of these tasks and we want good diagnostic of errors." - [here](https://www.elektroda.pl/rtvforum/topic3599980.html)

"arkhipenko/TaskScheduler is still my choice for now, especially when I get my pull request in, so we can have that idle 1 ms sleep feature for free." - [here](http://stm32duinoforum.com/forum/viewtopic_f_18_t_4299.html)

"The difference with milis is basically that you don’t have to be using logics to manage the executions, but the module itself does it. This will allow us to make code more readable and easier to maintain. In addition, we must take into account the extra functions it provides, such as saving energy when not in use, or changing settings dynamically." - [here](https://www.electrosoftcloud.com/en/arduino-taskscheduler-no-more-millis-or-delay/)



### Check out what TaskScheduler can do:

#### Around the world:

* Ninja Timer: Giant 7-Segment Display at Adafruit.com
  https://learn.adafruit.com/ninja-timer-giant-7-segment-display/timer-code
* Playing with NeoPixel to create a nice #smartBulb IoT
  https://www.zerozone.it/linux-e-open-source/giocare-con-i-neopixel-per-realizzare-un-simpatico-smartbulb-iot/16760
* Adding a timer to XK X6 Transmitter
  https://www.elvinplay.com/adding-a-timer-to-xk-x6-transmitter-en/
* Arduino Bluetooth remote control + ultrasonic anti-collision car
  https://xie.infoq.cn/article/0f27dbbebcc2b99b35132b262
* WEMOS D1 Mini로 Ad-hoc WIFI network
  https://m.blog.naver.com/sonyi/221330334326
* [3 Devo](http://3devo.eu/) - Quality 3D printing filament, now made accessible and affordable
(http://3devo.eu/license-information/)


* [Houston midi](https://github.com/chaffneue/houston) clock project - TaskScheduler with microseconds resolution
  
    >by chaffneue:
    >>My first arduino project. It's a multi-master midi controller with a shared clock and
	 auto count in behaviour.
	
	 youtube: https://www.youtube.com/watch?v=QRof550TtXo


* [Hackabot Nano](http://hackarobot.com/) by Funnyvale -  Compact Plug and Play Arduino compatible robotic kit
     https://www.kickstarter.com/projects/hackarobot/hackabot-nano-compact-plug-and-play-arduino-robot
* Discrete Time Systems Wiki - 
     https://sistemas-en-tiempo-discreto.fandom.com/es/wiki/Tiempo_Real

#### My projects:

* Interactive "Do Not Disturb" sign in a shape of Minecraft Sword (ESP32)
    (https://www.instructables.com/id/Interactive-Minecraft-Do-Not-Enter-SwordSign-ESP32/)
* Interactive Predator Costume with Real-Time Head Tracking Plasma Cannon (Teensy, Arduino Nano)
    (https://www.instructables.com/id/Interactive-Predator-Costume-With-Head-Tracking-Pl/)
* IoT APIS v2 - Autonomous IoT-enabled Automated Plant Irrigation System (ESP8266)
    (http://www.instructables.com/id/IoT-APIS-V2-Autonomous-IoT-enabled-Automated-Plant/)
* APIS - Automated Plant Irrigation System (Arduino Uno)
    (http://www.instructables.com/id/APIS-Automated-Plant-Irrigation-System/)

* Party Lights LEDs music visualization (Leaf Maple Mini)
    (https://www.instructables.com/id/Portable-Party-Lights/)
* Arduino Nano based Hexbug Scarab Robotic Spider (Arduino Nano)
    (http://www.instructables.com/id/Arduino-Nano-based-Hexbug-Scarab-Robotic-Spider/)
* Wave your hand to control OWI Robotic Arm... no strings attached (Arduino Uno and Nano)
    (http://www.instructables.com/id/Wave-your-hand-to-control-OWI-Robotic-Arm-no-strin/)


* Interactive Halloween Pumpkin (Arduino Uno)
    (http://www.instructables.com/id/Interactive-Halloween-Pumpkin/)
