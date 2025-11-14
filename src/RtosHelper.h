#pragma once
#include <functional>
#include <utility>

extern "C" {
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
}

using Task = std::function<void(void)>;

class TaskWrapper {
private:
    TaskHandle_t taskHandle = nullptr;
    Task innerTask;
    Task setUp;
    long taskInternMs;

public:
    TaskWrapper(Task setUp, Task task, long taskInternMs) : setUp(std::move(setUp)), innerTask(std::move(task)),
                                                            taskInternMs(taskInternMs) {
        createTask();
    }

    static void innerTaskLoopEntry(void *pvParameters) {
        auto *self = static_cast<TaskWrapper *>(pvParameters);
        self->innerTaskLoop();
    }

    void innerTaskLoop() const {
        setUp();
        for (;;) {
            innerTask();
            // 小睡一下，避免空转太猛
            vTaskDelay(pdMS_TO_TICKS(taskInternMs));
        }

        vTaskDelete(nullptr);
    }

    void createTask() {
        xTaskCreate(
            innerTaskLoopEntry,
            "IRTask",
            4096,
            nullptr,
            2,
            &taskHandle
        );
    }
};
