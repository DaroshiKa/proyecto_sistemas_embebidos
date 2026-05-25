#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "services/CLIService.hpp"
#include "app/DemoMode.hpp"
#include "communication/UARTConsole.hpp"

namespace Tasks
{
    struct TaskCLIConfig
    {
        uint32_t   stackSize { 6144 };
        UBaseType_t priority { 3 };       // menor que motion/EMG/IMU
        BaseType_t  coreId   { 0 };       // CLI en core 0 (UART)
        const char* name     { "TaskCLI" };
    };

    class TaskCLI
    {
    public:
        TaskCLI(
            Communication::UARTConsole& console,
            Services::CLIService& cliService,
            App::DemoMode& demoMode,
            const TaskCLIConfig& config = TaskCLIConfig{}
        );

        bool start();
        void stop();
        bool isRunning() const { return handle_ != nullptr; }

    private:
        static void taskEntry(void* arg);
        void run();

        Communication::UARTConsole&  console_;
        Services::CLIService&        cli_;
        App::DemoMode&               demo_;
        TaskCLIConfig                config_;
        TaskHandle_t                 handle_ { nullptr };
        volatile bool                running_{ false };
    };
}