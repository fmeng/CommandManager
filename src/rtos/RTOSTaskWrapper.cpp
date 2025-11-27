#include "RTOSTaskWrapper.h"

RTOSTaskWrapper::RTOSTaskWrapper(String taskName, Task_T setUpFun, Task_T loopFun, const int usStackDepth,
                                 const int uxPriority)
    : TaskWrapper(std::move(taskName), std::move(setUpFun), std::move(loopFun))
      , usStackDepth(usStackDepth), uxPriority(uxPriority) {
}

RTOSTaskWrapper::RTOSTaskWrapper(Task_T setUpFun, Task_T loopFun, const int usStackDepth, const int uxPriority)
    : TaskWrapper(String(F("RTOSTaskWrapper-")) + millis(), std::move(setUpFun), std::move(loopFun))
      , usStackDepth(usStackDepth), uxPriority(uxPriority) {
}

RTOSTaskWrapper::RTOSTaskWrapper(Task_T loopFun, const int usStackDepth, const int uxPriority)
    : TaskWrapper(String(F("RTOSTaskWrapper-")) + millis(), []() {
      }, std::move(loopFun))
      , usStackDepth(usStackDepth), uxPriority(uxPriority) {
}

RTOSTaskWrapper::RTOSTaskWrapper(String taskName, Task_T setUpFun, Task_T loopFun) : TaskWrapper(
    std::move(taskName), std::move(setUpFun), std::move(loopFun)) {
    // 和arduino task优先级保持一致 https://github.com/espressif/arduino-esp32/blob/d027ba16f1e4d3b56d674175549c3252a06f563b/cores/esp32/main.cpp#L113
    uxPriority = 1;
    // usStackDepth = getArduinoLoopTaskStackSize();
    usStackDepth = 4096;
}

RTOSTaskWrapper::RTOSTaskWrapper(Task_T setUpFun, Task_T loopFun) : TaskWrapper(
    String(F("RTOSTaskWrapper-")) + millis(), std::move(setUpFun), std::move(loopFun)) {
    // 和arduino task优先级保持一致 https://github.com/espressif/arduino-esp32/blob/d027ba16f1e4d3b56d674175549c3252a06f563b/cores/esp32/main.cpp#L113
    uxPriority = 1;
    // usStackDepth = getArduinoLoopTaskStackSize();
    usStackDepth = 4096;
}

RTOSTaskWrapper::RTOSTaskWrapper(String taskName, Task_T loopFun)
    : TaskWrapper(std::move(taskName), []() {
    }, std::move(loopFun)) {
    // 和arduino task优先级保持一致 https://github.com/espressif/arduino-esp32/blob/d027ba16f1e4d3b56d674175549c3252a06f563b/cores/esp32/main.cpp#L113
    uxPriority = 1;
    // usStackDepth = getArduinoLoopTaskStackSize();
    usStackDepth = 4096;
}

RTOSTaskWrapper::RTOSTaskWrapper(Task_T loopFun)
    : TaskWrapper(String(F("RTOSTaskWrapper-")) + millis(), []() {
    }, std::move(loopFun)) {
    // 和arduino task优先级保持一致 https://github.com/espressif/arduino-esp32/blob/d027ba16f1e4d3b56d674175549c3252a06f563b/cores/esp32/main.cpp#L113
    uxPriority = 1;
    // usStackDepth = getArduinoLoopTaskStackSize();
    usStackDepth = 4096;
}

void RTOSTaskWrapper::doInnerLoopEntry(void *pvParameters) {
    auto *self = static_cast<RTOSTaskWrapper *>(pvParameters);
    self->doInnerLoop();
}

void RTOSTaskWrapper::doInnerLoop() {
    vTaskDelay(pdMS_TO_TICKS(this->startDelayMs));
    this->m_setUpFun();

    if (this->loopTimes > 0) {
        for (int i = 0; i < this->loopTimes; i++) {
            this->m_loopFun();
            vTaskDelay(pdMS_TO_TICKS(this->loopDelayMs));
        }
    } else {
        for (;;) {
            this->m_loopFun();
            vTaskDelay(pdMS_TO_TICKS(this->loopDelayMs));
        }
    }
    vTaskDelete(nullptr);
}

void RTOSTaskWrapper::create() {
    stop(); // 安全释放资源
    if (xTaskCreate(
            doInnerLoopEntry,
            this->m_taskName.c_str(),
            this->usStackDepth,
            this,
            this->uxPriority,
            &this->taskHandle
        ) != pdPASS) {
#ifdef LOG_DEBUG
        Serial.print(F("RTOSTaskWrapper::create failed"));
        Serial.print(F(", m_taskName: "));
        Serial.print(this->m_taskName);
        Serial.println();
#endif
        return;
    }

    // 立即挂起任务
    vTaskSuspend(taskHandle);

#ifdef LOG_DEBUG
    Serial.print(F("RTOSTaskWrapper::create, success"));
    Serial.print(F(", m_taskName: "));
    Serial.print(this->m_taskName);
    Serial.print(F(", usStackDepth: "));
    Serial.print(this->usStackDepth);
    Serial.print(F(", uxPriority: "));
    Serial.print(this->uxPriority);
    Serial.print(F(", startDelayMs: "));
    Serial.print(this->startDelayMs);
    Serial.print(F(", loopDelayMs: "));
    Serial.print(this->loopDelayMs);
    Serial.print(F(", loopTimes: "));
    Serial.print(this->loopTimes);
    Serial.println();
#endif
}

void RTOSTaskWrapper::start(const long startDelayMs, const long loopDelayMs, const long loopTimes,
                            const bool autoCreate) {
    this->startDelayMs = startDelayMs;
    this->loopDelayMs = loopDelayMs;
    this->loopTimes = loopTimes;

    if (autoCreate && (this->taskHandle == nullptr || eTaskGetState(this->taskHandle) == eDeleted)) {
        create();
    }
    if (eTaskGetState(this->taskHandle) == eSuspended) {
        vTaskResume(this->taskHandle);
    }
}

void RTOSTaskWrapper::start() {
    start(startDelayMs, loopDelayMs, loopTimes, true);
}


void RTOSTaskWrapper::restart() {
    stop();
    start();
}

void RTOSTaskWrapper::disableLoop() {
    if (this->taskHandle != nullptr
        && eTaskGetState(this->taskHandle) != eDeleted && eTaskGetState(this->taskHandle) != eSuspended) {
        vTaskSuspend(this->taskHandle);
    }
}

void RTOSTaskWrapper::enableLoop() {
    if (this->taskHandle == nullptr || eTaskGetState(this->taskHandle) == eDeleted) {
        start();
        return;
    }
    if (eTaskGetState(this->taskHandle) == eSuspended) {
        vTaskResume(this->taskHandle);
    }
}

void RTOSTaskWrapper::stop() {
    if (this->taskHandle != nullptr && eTaskGetState(this->taskHandle) != eDeleted) {
        vTaskDelete(this->taskHandle);
    }
}

const TaskHandle_t &RTOSTaskWrapper::getTaskHandle() const {
    return this->taskHandle;
}
