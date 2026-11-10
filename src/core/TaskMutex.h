#pragma once
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

// Task-only recursive lock; never used from interrupt handlers.
class TaskMutex
{
public:
    TaskMutex() : handle(xSemaphoreCreateRecursiveMutex()) { configASSERT(handle); }
    ~TaskMutex() { vSemaphoreDelete(handle); }
    TaskMutex(const TaskMutex &) = delete;
    TaskMutex &operator=(const TaskMutex &) = delete;
    class Guard
    {
    public:
        explicit Guard(TaskMutex &mutex) : mutex(mutex) { xSemaphoreTakeRecursive(mutex.handle, portMAX_DELAY); }
        ~Guard() { xSemaphoreGiveRecursive(mutex.handle); }
        Guard(const Guard &) = delete;
        Guard &operator=(const Guard &) = delete;
    private:
        TaskMutex &mutex;
    };
private:
    SemaphoreHandle_t handle;
};
