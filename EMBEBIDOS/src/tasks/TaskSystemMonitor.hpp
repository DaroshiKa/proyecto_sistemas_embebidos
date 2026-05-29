#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "services/SafetyService.hpp"

namespace Tasks
{
    struct TaskSystemMonitorConfig
    {
        uint32_t    periodMs    { 5000 };    // cada 5 s
        uint32_t    stackSize   { 3072 };
        UBaseType_t priority    { 1 };       // baja
        BaseType_t  coreId      { 0 };       // core 0, junto a wifi/idle
        const char* name        { "TaskSysMon" };
        uint32_t    stackLowThresholdWords { 256 };
    };

    class TaskSystemMonitor
    {
    public:
        TaskSystemMonitor(
            Services::SafetyService& safety,
            const TaskSystemMonitorConfig& config = TaskSystemMonitorConfig{}
        );

        // Registrar tasks que queremos monitorear. Hasta 8 handles.
        bool registerWatchedTask(TaskHandle_t handle, const char* name);

        bool start();
        void stop();
        bool isRunning() const { return handle_ != nullptr; }

    private:
        static constexpr size_t MAX_WATCHED = 8;

        struct Watched
        {
            TaskHandle_t handle { nullptr };
            const char*  name   { nullptr };
        };

        static void taskEntry(void* arg);
        void run();

        Services::SafetyService&        safety_;
        TaskSystemMonitorConfig         config_;
        TaskHandle_t                    handle_  { nullptr };
        volatile bool                   running_ { false };

        Watched   watched_[MAX_WATCHED] {};
        size_t    watchedCount_         { 0 };
    };
}