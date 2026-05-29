#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "services/IMUService.hpp"

namespace Tasks
{
    struct TaskIMUConfig
    {
        uint32_t   periodMs   { 10 };       // 100 Hz por defecto
        uint32_t   stackSize  { 4096 };
        UBaseType_t priority  { 5 };
        BaseType_t  coreId    { 1 };        // pinned al core 1
        const char* name      { "TaskIMU" };
    };

    class TaskIMU
    {
    public:
        TaskIMU(
            Services::IMUService& service,
            const TaskIMUConfig& config = TaskIMUConfig{}
        );

        bool start();

        void stop();

        bool isRunning() const { return handle_ != nullptr; }
        TaskHandle_t handle() const { return handle_; }

    private:
        static void taskEntry(void* arg);

        void run();

        Services::IMUService& service_;
        TaskIMUConfig         config_;
        TaskHandle_t          handle_ { nullptr };
        volatile bool         running_ { false };
    };
}