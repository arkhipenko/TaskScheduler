// Cooperative multitasking library for Arduino
// Copyright (c) 2015-2019 Anatoli Arkhipenko

/*
    This is a namespace wrapper to avoid collision with frameworks having their
    own class named Scheduler (i.e. nordicnrf52)
    include it instead of <TaskScheduler.h> into your sketch and use Class name wrapper - 'TaskScheduler'
    i.e. 
    TaskScheduler ts;
*/

#pragma once
#include <Arduino.h>

namespace TS{
#include "TaskScheduler.h"
}

using TsScheduler = TS::Scheduler;
using TsTask = TS::Task;
using TsStatusRequest = TS::StatusRequest;
using TsSleepCallback = TS::SleepCallback;
using TsTaskCallback = TS::TaskCallback;
using TsTaskOnDisable = TS::TaskOnDisable;
using TsTaskOnEnable = TS::TaskOnEnable;
