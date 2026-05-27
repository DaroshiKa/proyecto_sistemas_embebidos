#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "services/SafetyService.hpp"
#include "services/WatchdogManager.hpp"

namespace Tasks
{
    struct TaskSafetyConfig
    {
        uint32_t    periodMs   { 50 };       // 20 Hz
        uint32_t    stackSize  { 4096 };
        UBaseType_t priority   { 8 };        // > TaskMotion (7)
        BaseType_t  coreId     { 1 };
        const char* name       { "TaskSafety" };
        bool        useWatchdog { true };
    };

    class TaskSafety
    {
    public:
        TaskSafety(
            Services::SafetyService& safety,
            Services::WatchdogManager& watchdog,
            const TaskSafetyConfig& config = TaskSafetyConfig{}
        );

        bool start();
        void stop();
        bool isRunning() const { return handle_ != nullptr; }

    private:
        static void taskEntry(void* arg);
        void run();

        Services::SafetyService&    safety_;
        Services::WatchdogManager&  watchdog_;
        TaskSafetyConfig            config_;
        TaskHandle_t                handle_  { nullptr };
        volatile bool               running_ { false };
    };
}