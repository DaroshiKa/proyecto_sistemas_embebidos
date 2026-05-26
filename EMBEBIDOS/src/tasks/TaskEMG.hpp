#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "services/EMGService.hpp"
#include "core/TaskHeartbeat.hpp"

namespace Tasks
{
    struct TaskEMGConfig
    {
        uint32_t   stackSize { 6144 };
        UBaseType_t priority { 6 };
        BaseType_t  coreId   { 1 };
        const char* name     { "TaskEMG" };
    };

    class TaskEMG
    {
    public:
        TaskEMG(
            Services::EMGService& service,
            const TaskEMGConfig& config = TaskEMGConfig{},
            Core::TaskHeartbeat* heartbeat = nullptr     // NUEVO en Etapa 9
        );

        bool start();
        void stop();
        bool isRunning() const { return handle_ != nullptr; }

    private:
        static void taskEntry(void* arg);
        void run();

        Services::EMGService& service_;
        TaskEMGConfig         config_;
        Core::TaskHeartbeat*  heartbeat_ { nullptr };    // NUEVO en Etapa 9
        TaskHandle_t          handle_ { nullptr };
        volatile bool         running_ { false };
    };
}