# Task Scheduler
### Cooperative multitasking for Arduino microcontrollers
#### Version 3.0.2: 2018-11-14

### OVERVIEW:
A lightweight implementation of cooperative multitasking (task scheduling) supporting:
1. Periodic task execution, with dynamic execution period in `milliseconds` (default) or `microseconds` (if explicitly enabled) – frequency of execution
2. Number of iterations (limited or infinite number of iterations)
3. Execution of tasks in predefined sequence
4. Dynamic change of task execution parameters (frequency, number of iterations, callback methods)
5. Power saving via entering **IDLE** sleep mode when tasks are not scheduled to run
6. Support for event-driven task invocation via Status Request object
7. Support for task IDs and Control Points for error handling and watchdog timer
8. Support for Local Task Storage pointer (allowing use of same callback code for multiple tasks)
9. Support for layered task prioritization
10. Support for `std::functions` (`ESP8266` only)
11. Overall task timeout
12. Static and dynamic callback method binding
13. Support for STM32F1 ARM Cortex-M3 boards

Scheduling overhead: between `15` and `18` microseconds per scheduling pass (Arduino UNO rev 3 @ `16MHz` clock, single scheduler w/o prioritization)

**TaskScheduler** was tested on the following platforms:
* Arduino Uno R3
* Arduino Nano
* Arduino Micro
* ATtiny85
* ESP8266 (Node MCU v2.0)
* ESP32
* Teensy (tested on Teensy 3.5)
* STN32F1 (tested on Mini USB STM32F103RCBT6 ARM Cortex-M3 leaflabs Leaf maple mini module F)
---
![TaskScheduler process diagram](https://github.com/arkhipenko/TaskScheduler/raw/master/extras/TaskScheduler_html.png)
---
### [Changelog:](https://github.com/arkhipenko/TaskScheduler/wiki/Changelog)
Version|Version 1|Version 2
---|---|---
|| |[2.6.1](https://github.com/arkhipenko/TaskScheduler/wiki/Changelog#v261) (*current version*)
||[1.9.2](https://github.com/arkhipenko/TaskScheduler/wiki/Changelog#v192)|[2.6.0](https://github.com/arkhipenko/TaskScheduler/wiki/Changelog#v260)
||[1.9.0](https://github.com/arkhipenko/TaskScheduler/wiki/Changelog#v190)|[2.5.2](https://github.com/arkhipenko/TaskScheduler/wiki/Changelog#v252)
||[1.8.5](https://github.com/arkhipenko/TaskScheduler/wiki/Changelog#v185)|[2.5.1](https://github.com/arkhipenko/TaskScheduler/wiki/Changelog#v251)
||[1.8.4](https://github.com/arkhipenko/TaskScheduler/wiki/Changelog#v184)|[2.5.0](https://github.com/arkhipenko/TaskScheduler/wiki/Changelog#v250)
||[1.8.3](https://github.com/arkhipenko/TaskScheduler/wiki/Changelog#v183)|[2.4.0](https://github.com/arkhipenko/TaskScheduler/wiki/Changelog#v240)
||[1.8.2](https://github.com/arkhipenko/TaskScheduler/wiki/Changelog#v182)|[2.3.0](https://github.com/arkhipenko/TaskScheduler/wiki/Changelog#v230)
||[1.8.1](https://github.com/arkhipenko/TaskScheduler/wiki/Changelog#v181)|[2.2.1](https://github.com/arkhipenko/TaskScheduler/wiki/Changelog#v221)
||[1.8.0](https://github.com/arkhipenko/TaskScheduler/wiki/Changelog#v180)|[2.2.0](https://github.com/arkhipenko/TaskScheduler/wiki/Changelog#v220)
||[1.7.0](https://github.com/arkhipenko/TaskScheduler/wiki/Changelog#v170)|[2.1.0](https://github.com/arkhipenko/TaskScheduler/wiki/Changelog#v210)
||[1.6.0](https://github.com/arkhipenko/TaskScheduler/wiki/Changelog#v160)|[2.0.1](https://github.com/arkhipenko/TaskScheduler/wiki/Changelog#v201)
||[1.5.1](https://github.com/arkhipenko/TaskScheduler/wiki/Changelog#v151)|[2.0.0](https://github.com/arkhipenko/TaskScheduler/wiki/Changelog#v200)
||[1.5.0](https://github.com/arkhipenko/TaskScheduler/wiki/Changelog#v150)|
||[1.4.1](https://github.com/arkhipenko/TaskScheduler/wiki/Changelog#v141)|
||[1.0.0](https://github.com/arkhipenko/TaskScheduler/wiki/Changelog#v100)|

#### For detailed functionality overview please refer to TaskScheduler documentation in the 'extras' folder or in the [Wiki page](https://github.com/arkhipenko/TaskScheduler/wiki).

### Check out what TaskScheduler can do:

* [3 Devo](http://3devo.eu/) - Quality 3D printing filament, now made accessible and affordable
(http://3devo.eu/license-information/)


* [Houston midi](https://github.com/chaffneue/houston) clock project - TaskScheduler with microseconds resolution
    >by chaffneue:
    >>My first arduino project. It's a multi-master midi controller with a shared clock and
     auto count in behaviour.

	 youtube: https://www.youtube.com/watch?v=QRof550TtXo


* [Hackabot Nano](http://hackarobot.com/) by Funnyvale -  Compact Plug and Play Arduino compatible robotic kit
     https://www.kickstarter.com/projects/hackarobot/hackabot-nano-compact-plug-and-play-arduino-robot


* Arduino Nano based Hexbug Scarab Robotic Spider
    (by arkhipenko: http://www.instructables.com/id/Arduino-Nano-based-Hexbug-Scarab-Robotic-Spider/)

* Wave your hand to control OWI Robotic Arm... no strings attached
    (by arkhipenko: http://www.instructables.com/id/Wave-your-hand-to-control-OWI-Robotic-Arm-no-strin/)


* APIS - Automated Plant Irrigation System
    (by arkhipenko: http://www.instructables.com/id/APIS-Automated-Plant-Irrigation-System/)


* IoT APIS v2 - Autonomous IoT-enabled Automated Plant Irrigation System
    (by arkhipenko: http://www.instructables.com/id/IoT-APIS-V2-Autonomous-IoT-enabled-Automated-Plant/)

* Interactive Halloween Pumpkin
    (by arkhipenko: http://www.instructables.com/id/Interactive-Halloween-Pumpkin/)

