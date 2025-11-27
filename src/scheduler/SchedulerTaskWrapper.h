#pragma once
#ifdef ENABLE_TASK_SCHEDULER

#include <TaskSchedulerDeclarations.h>
#include "TaskWrapper.h"

extern Scheduler runner;

class SchedulerTaskWrapper : public TaskWrapper {
protected:
    long startDelayMs = -1;
    long loopDelayMs = -1;
    long loopTimes = -1;
    Task *p_loopTask = nullptr;
    Task *p_setupTask = nullptr;
    volatile bool setupCompleted = false;

    virtual Task *doCreateSetupTask();

    virtual Task *doCreateLoopTask();

    void create(long startDelayMs, long loopDelayMs, long loopTimes);

public:
    ~SchedulerTaskWrapper() override = default;

    SchedulerTaskWrapper(String taskName, Task_T setUpFun, Task_T loopFun);

    SchedulerTaskWrapper(Task_T setUpFun, Task_T loopFun);

    explicit SchedulerTaskWrapper(Task_T loopFun);

    void start(long startDelayMs, long loopDelayMs, long loopTimes, bool autoCreate) override;

    void start() override;

    void restart() override;

    void stop() override;

    void enableLoop() override;

    void disableLoop() override;
};

#endif
