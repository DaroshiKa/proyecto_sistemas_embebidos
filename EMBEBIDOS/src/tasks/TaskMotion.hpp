#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "services/MotionService.hpp"
#include "drivers/ServoManager.hpp"

namespace Tasks
{
    struct TaskMotionConfig
    {
        uint32_t   periodMs    { 10 };       // 100 Hz
        uint32_t   stackSize   { 4096 };
        UBaseType_t priority   { 7 };        // mayor que EMG (6) e IMU (5)
        BaseType_t  coreId     { 1 };
        const char* name       { "TaskMotion" };
    };

    class TaskMotion
    {
    public:
        TaskMotion(
            Services::MotionService& motionService,
            Drivers::ServoManager& servoManager,
            QueueHandle_t commandQueue,
            const TaskMotionConfig& config = TaskMotionConfig{}
        );

        bool start();
        void stop();
        bool isRunning() const { return handle_ != nullptr; }

    private:
        static void taskEntry(void* arg);
        void run();

        Services::MotionService& motion_;
        Drivers::ServoManager&   servos_;
        QueueHandle_t            queue_;
        TaskMotionConfig         config_;
        TaskHandle_t             handle_ { nullptr };
        volatile bool            running_ { false };
    };
}