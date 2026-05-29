#pragma once

#include <array>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "interfaces/IDiagnosticsProvider.hpp"
#include "interfaces/ISafetyMonitor.hpp"

#include "core/CommandDispatcher.hpp"

#include "models/DiagnosticsSnapshot.hpp"

// Forward decls — no acoplamos a tipos concretos en el header.
namespace Services { class IMUService; class EMGService; }

namespace Tasks
{
    struct TaskDiagnosticsConfig
    {
        uint32_t    periodMs       { 2000 };    // 0.5 Hz
        uint32_t    stackSize      { 4096 };
        UBaseType_t priority       { 1 };       // baja
        BaseType_t  coreId         { 0 };
        const char* name           { "TaskDiag" };
        bool        logToSerial    { true };    // dump periódico al UART log
    };

    class TaskDiagnostics final :
        public Interfaces::IDiagnosticsProvider
    {
    public:
        struct WatchedTask
        {
            TaskHandle_t handle;
            const char*  name;
        };

        struct Dependencies
        {
            Interfaces::ISafetyMonitor* safetyMonitor   { nullptr };
            Core::CommandDispatcher*    dispatcherStats { nullptr };
            Services::IMUService*       imuSvc          { nullptr };
            Services::EMGService*       emgSvc          { nullptr };
        };

        TaskDiagnostics(
            const Dependencies& deps,
            const TaskDiagnosticsConfig& cfg = TaskDiagnosticsConfig{}
        );

        bool addWatchedTask(TaskHandle_t handle, const char* name);

        bool start();
        void stop();
        bool isRunning() const { return handle_ != nullptr; }
        TaskHandle_t handle() const { return handle_; }

        // IDiagnosticsProvider
        Models::DiagnosticsSnapshot snapshot() const override;

    private:
        static constexpr size_t MAX_WATCHED = Models::DIAG_MAX_TASKS;

        static void taskEntry(void* arg);
        void run();
        void collect();
        void logSnapshot(const Models::DiagnosticsSnapshot& s) const;

        Dependencies                  deps_;
        TaskDiagnosticsConfig         config_;

        std::array<WatchedTask, MAX_WATCHED> watched_ {};
        size_t                               watchedCount_ { 0 };

        mutable SemaphoreHandle_t     mutex_  { nullptr };
        Models::DiagnosticsSnapshot   last_   {};

        TaskHandle_t                  handle_  { nullptr };
        volatile bool                 running_ { false };
    };
}