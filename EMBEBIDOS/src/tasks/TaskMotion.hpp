#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "services/MotionService.hpp"
#include "drivers/ServoManager.hpp"
#include "core/TaskHeartbeat.hpp"

namespace Services { class WatchdogManager; }

namespace Tasks
{
    struct TaskMotionConfig
    {
        uint32_t   periodMs    { 10 };
        uint32_t   stackSize   { 4096 };
        UBaseType_t priority   { 7 };
        BaseType_t  coreId     { 1 };
        const char* name       { "TaskMotion" };
        bool        useWatchdog { true };
    };

    class TaskMotion
    {
    public:
        TaskMotion(
            Services::MotionService& motionService,
            Drivers::ServoManager& servoManager,
            QueueHandle_t commandQueue,
            const TaskMotionConfig& config = TaskMotionConfig{},
            Core::TaskHeartbeat* heartbeat = nullptr
        );

        // Inyección opcional del watchdog para alimentación periódica.
        void attachWatchdog(Services::WatchdogManager* wdt);

        bool start();
        void stop();
        bool isRunning() const { return handle_ != nullptr; }

        TaskHandle_t handle() const { return handle_; }

    private:
        static void taskEntry(void* arg);
        void run();

<<<<<<< HEAD
        Services::MotionService&   motion_;
        Drivers::ServoManager&     servos_;
        QueueHandle_t              queue_;
        TaskMotionConfig           config_;
        Services::WatchdogManager* watchdog_ { nullptr };
        TaskHandle_t               handle_   { nullptr };
        volatile bool              running_  { false };
=======
        Services::MotionService& motion_;
        Drivers::ServoManager&   servos_;
        QueueHandle_t            queue_;
        TaskMotionConfig         config_;
        Core::TaskHeartbeat*     heartbeat_ { nullptr };
        TaskHandle_t             handle_ { nullptr };
        volatile bool            running_ { false };
>>>>>>> 0f18090e8f7bddf39123306abf466af425290d60
    };
}