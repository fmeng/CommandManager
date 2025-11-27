#pragma once
#include <Arduino.h>

typedef std::function<void(void)> Task_T;

class TaskWrapper {
protected:
    String m_taskName;
    Task_T m_setUpFun;
    Task_T m_loopFun;

public:
    virtual ~TaskWrapper() = default;

    TaskWrapper(String taskName, Task_T setUpFun, Task_T loopFun) : m_taskName(std::move(taskName)),
                                                                    m_setUpFun(std::move(setUpFun)),
                                                                    m_loopFun(std::move(loopFun)) {
    }

    virtual void start(long startDelayMs, long loopDelayMs, long loopTimes, bool autoCreate) = 0;

    virtual void start() = 0;

    virtual void restart() = 0;

    virtual void stop() = 0;

    virtual void disableLoop() = 0;

    virtual void enableLoop() = 0;
};
