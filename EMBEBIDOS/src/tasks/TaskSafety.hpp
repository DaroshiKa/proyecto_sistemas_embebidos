#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "services/SafetyService.hpp"

namespace Tasks
{
    struct TaskSafetyConfig
    {
        uint32_t   periodMs   { 200 };       // 5 Hz
        uint32_t   stackSize  { 4096 };
        UBaseType_t priority  { 8 };         // mayor que TaskMotion(7)
        BaseType_t  coreId    { 0 };
        const char* name      { "TaskSafety" };

        // Si true, registramos esta task en el ESP-IDF TWDT.
        bool        registerInTWDT { true };
    };

    class TaskSafety
    {
    public:
        TaskSafety(
            Services::SafetyService& service,
            const TaskSafetyConfig& config = TaskSafetyConfig{}
        );

        bool start();
        void stop();
        bool isRunning() const { return handle_ != nullptr; }

    private:
        static void taskEntry(void* arg);
        void run();

        Services::SafetyService& service_;
        TaskSafetyConfig         config_;
        TaskHandle_t             handle_ { nullptr };
        volatile bool            running_ { false };
    };
}