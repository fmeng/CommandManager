#ifdef ENABLE_TASK_SCHEDULER
#include "SchedulerTaskWrapper.h"

SchedulerTaskWrapper::SchedulerTaskWrapper(String taskName, Task_T setUpFun,
                                           Task_T loopFun) : TaskWrapper(
    std::move(taskName), std::move(setUpFun), std::move(loopFun)) {
}

SchedulerTaskWrapper::SchedulerTaskWrapper(Task_T setUpFun, Task_T loopFun) : TaskWrapper(
    String(F("SchedulerTaskWrapper-")) + millis(), std::move(setUpFun), std::move(loopFun)) {
}

SchedulerTaskWrapper::SchedulerTaskWrapper(Task_T loopFun) : TaskWrapper(
    String(F("SchedulerTaskWrapper-")) + millis(), []() {
    }, std::move(loopFun)) {
}

// #ifdef TASK_STD_FUNCTION

Task *SchedulerTaskWrapper::doCreateSetupTask() {
    return new Task(1, 1, [this]() {
        this->m_setUpFun();
        this->setupCompleted = true;
        this->p_loopTask->enable();
    }, &runner, false);
}

Task *SchedulerTaskWrapper::doCreateLoopTask() {
    return new Task(loopDelayMs, loopTimes, this->m_loopFun,
                    &runner, false);
}

// #endif


void SchedulerTaskWrapper::create(const long startDelayMs, const long loopDelayMs, const long loopTimes) {
    this->startDelayMs = startDelayMs;
    this->loopDelayMs = loopDelayMs;
    this->loopTimes = loopTimes;
    this->p_loopTask = doCreateLoopTask();
    this->p_setupTask = doCreateSetupTask();

#ifdef LOG_DEBUG
Serial.print (F("SchedulerTaskWrapper::create, "));
Serial.print (F(", m_taskName: "));
Serial.print (this->m_taskName);
Serial.print (F(", startDelayMs: "));
Serial.print (this->startDelayMs);
Serial.print (F(", loopDelayMs: "));
Serial.print (this->loopDelayMs);
Serial.print (F(", loopTimes: "));
Serial.print (this->loopTimes);
Serial.println();
#endif
}


void SchedulerTaskWrapper::start(const long startDelayMs, const long loopDelayMs, const long loopTimes,
                                 const bool autoCreate) {
    if (autoCreate && (this->p_setupTask == nullptr || this->p_loopTask == nullptr)) {
        create(startDelayMs, loopDelayMs, loopTimes);
    }

    if (this->startDelayMs == startDelayMs && this->loopDelayMs == loopDelayMs && this->loopTimes == loopTimes) {
        if (p_setupTask->isEnabled() || (setupCompleted && p_loopTask->isEnabled())) {
#ifdef LOG_DEBUG
Serial.print (F("SchedulerTaskWrapper::start, no changed"));
Serial.print (F(", m_taskName: "));
Serial.print (this->m_taskName);
Serial.print (F(", setupCompleted: "));
Serial.print (this->setupCompleted);
Serial.print (F(", startDelayMs: "));
Serial.print (this->startDelayMs);
Serial.print (F(", loopDelayMs: "));
Serial.print (this->loopDelayMs);
Serial.print (F(", loopTimes: "));
Serial.print (this->loopTimes);
Serial.println();
#endif
return;
        }
    }

    this->startDelayMs= startDelayMs;
    this->loopDelayMs= loopDelayMs;
    this->loopTimes= loopTimes;
    this->setupCompleted=false;

    if (loopDelayMs> 0) {
        this->p_loopTask->setInterval(loopDelayMs);
    } else {
        this->p_loopTask->setInterval(2);
    }
    if (loopTimes> 0) {
        this->p_loopTask->setIterations(loopTimes);
    } else {
        this->p_loopTask->setIterations(TASK_FOREVER);
    }
    if (startDelayMs> 0) {
        this->p_setupTask->restartDelayed(startDelayMs);
    } else {
        this->p_setupTask->restart();
    }

#ifdef LOG_DEBUG
Serial.print (F("SchedulerTaskWrapper::start, changed"));
Serial.print (F(", m_taskName: "));
Serial.print (this->m_taskName);
Serial.print (F(", setupCompleted: "));
Serial.print (this->setupCompleted);
Serial.print (F(", startDelayMs: "));
Serial.print (this->startDelayMs);
Serial.print (F(", loopDelayMs: "));
Serial.print (this->loopDelayMs);
Serial.print (F(", loopTimes: "));
Serial.print (this->loopTimes);
Serial.println();
#endif
}

void SchedulerTaskWrapper::start() {
    start(startDelayMs, loopDelayMs, loopTimes, true);
}

void SchedulerTaskWrapper::restart() {

#ifdef LOG_DEBUG
Serial.print (F("SchedulerTaskWrapper::restart"));
Serial.print (F(", m_taskName: "));
Serial.print (this->m_taskName);
Serial.println();
#endif
this->p_setupTask->disable();
    this->p_loopTask->disable();
    this->setupCompleted=false;
start(startDelayMs, loopDelayMs, loopTimes, true);
}

void SchedulerTaskWrapper::stop() {

#ifdef LOG_DEBUG
Serial.print (F("SchedulerTaskWrapper::stop"));
Serial.print (F(", m_taskName: "));
Serial.print (this->m_taskName);
Serial.print (F(", setupCompleted: "));
Serial.print (this->setupCompleted);
Serial.print (F(", p_setupTask status: "));
Serial.print (this->p_setupTask->isEnabled());
Serial.print (F(", p_loopTask status: "));
Serial.print (this->p_loopTask->isEnabled());
Serial.println();
#endif
this->p_setupTask->disable();
    this->p_loopTask->disable();
    this->setupCompleted=false;
}

void SchedulerTaskWrapper::disableLoop() {

#ifdef LOG_DEBUG
Serial.print (F("SchedulerTaskWrapper::disableLoop"));
Serial.print (F(", m_taskName: "));
Serial.print (this->m_taskName);
Serial.println();
#endif
if (setupCompleted) {
    this->p_loopTask->disable();
}else if (!setupCompleted &&this->p_setupTask->isEnabled()) {
        this->p_setupTask->disable();
    }
}

void SchedulerTaskWrapper::enableLoop() {

#ifdef LOG_DEBUG
Serial.print (F("SchedulerTaskWrapper::enableLoop"));
Serial.print (F(", m_taskName: "));
Serial.print (this->m_taskName);
Serial.print (F(", setupCompleted: "));
Serial.print (this->setupCompleted);
Serial.print (F(", p_setupTask status: "));
Serial.print (this->p_setupTask->isEnabled());
Serial.print (F(", p_loopTask status: "));
Serial.print (this->p_loopTask->isEnabled());
Serial.println();
#endif
if (setupCompleted) {
    this->p_loopTask->enable();
}else {
        this->p_setupTask->enable();
    }
}

#endif