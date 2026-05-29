#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "services/SafetyService.hpp"
<<<<<<< HEAD
#include "services/WatchdogManager.hpp"
=======
>>>>>>> 0f18090e8f7bddf39123306abf466af425290d60

namespace Tasks
{
    struct TaskSafetyConfig
    {
<<<<<<< HEAD
        uint32_t    periodMs   { 50 };       // 20 Hz
        uint32_t    stackSize  { 4096 };
        UBaseType_t priority   { 8 };        // > TaskMotion (7)
        BaseType_t  coreId     { 1 };
        const char* name       { "TaskSafety" };
        bool        useWatchdog { true };
=======
        uint32_t   periodMs   { 200 };       // 5 Hz
        uint32_t   stackSize  { 4096 };
        UBaseType_t priority  { 8 };         // mayor que TaskMotion(7)
        BaseType_t  coreId    { 0 };
        const char* name      { "TaskSafety" };

        // Si true, registramos esta task en el ESP-IDF TWDT.
        bool        registerInTWDT { true };
>>>>>>> 0f18090e8f7bddf39123306abf466af425290d60
    };

    class TaskSafety
    {
    public:
        TaskSafety(
<<<<<<< HEAD
            Services::SafetyService& safety,
            Services::WatchdogManager& watchdog,
=======
            Services::SafetyService& service,
>>>>>>> 0f18090e8f7bddf39123306abf466af425290d60
            const TaskSafetyConfig& config = TaskSafetyConfig{}
        );

        bool start();
        void stop();
        bool isRunning() const { return handle_ != nullptr; }
<<<<<<< HEAD
        TaskHandle_t handle() const { return handle_; }
=======
>>>>>>> 0f18090e8f7bddf39123306abf466af425290d60

    private:
        static void taskEntry(void* arg);
        void run();

<<<<<<< HEAD
        Services::SafetyService&    safety_;
        Services::WatchdogManager&  watchdog_;
        TaskSafetyConfig            config_;
        TaskHandle_t                handle_  { nullptr };
        volatile bool               running_ { false };
=======
        Services::SafetyService& service_;
        TaskSafetyConfig         config_;
        TaskHandle_t             handle_ { nullptr };
        volatile bool            running_ { false };
>>>>>>> 0f18090e8f7bddf39123306abf466af425290d60
    };
}