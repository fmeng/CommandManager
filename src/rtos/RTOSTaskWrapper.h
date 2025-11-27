#pragma once
#include "TaskWrapper.h"


class RTOSTaskWrapper : public TaskWrapper {
protected:
    int usStackDepth = -1;
    int uxPriority = -1;
    long startDelayMs = -1;
    long loopDelayMs = -1;
    long loopTimes = -1;
    TaskHandle_t taskHandle = nullptr;

    virtual void create();

public:
    ~RTOSTaskWrapper() override = default;

    RTOSTaskWrapper(String taskName, Task_T setUpFun, Task_T loopFun, int usStackDepth, int uxPriority);

    RTOSTaskWrapper(Task_T setUpFun, Task_T loopFun, int usStackDepth, int uxPriority);

    RTOSTaskWrapper(Task_T loopFun, int usStackDepth, int uxPriority);

    RTOSTaskWrapper(String taskName, Task_T setUpFun, Task_T loopFun);

    RTOSTaskWrapper(Task_T setUpFun, Task_T loopFun);

    RTOSTaskWrapper(String taskName, Task_T loopFun);

    explicit RTOSTaskWrapper(Task_T loopFun);

    static void doInnerLoopEntry(void *pvParameters);

    virtual void doInnerLoop();

    void start(long startDelayMs, long loopDelayMs, long loopTimes, bool autoCreate) override;

    void start() override;

    void restart() override;

    void stop() override;

    void enableLoop() override;

    void disableLoop() override;

    const TaskHandle_t &getTaskHandle() const;
};
