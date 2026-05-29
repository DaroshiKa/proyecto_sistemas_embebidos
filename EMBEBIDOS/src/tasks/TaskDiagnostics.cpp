#include "tasks/TaskDiagnostics.hpp"

#include "services/IMUService.hpp"
#include "services/EMGService.hpp"

#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_timer.h"

#include <cstring>

namespace Tasks
{
    static constexpr const char* TAG = "Diag";

    TaskDiagnostics::TaskDiagnostics(
        const Dependencies& deps,
        const TaskDiagnosticsConfig& cfg
    )
        : deps_(deps),
          config_(cfg)
    {
    }

    bool TaskDiagnostics::addWatchedTask(TaskHandle_t handle, const char* name)
    {
        if (watchedCount_ >= MAX_WATCHED) return false;
        if (handle == nullptr) return false;

        watched_[watchedCount_].handle = handle;
        watched_[watchedCount_].name   = (name != nullptr) ? name : "?";
        ++watchedCount_;
        return true;
    }

    bool TaskDiagnostics::start()
    {
        if (handle_ != nullptr) return true;

        mutex_ = xSemaphoreCreateMutex();
        if (mutex_ == nullptr)
        {
            ESP_LOGE(TAG, "mutex create failed");
            return false;
        }

        running_ = true;
        const BaseType_t res = xTaskCreatePinnedToCore(
            &TaskDiagnostics::taskEntry,
            config_.name,
            config_.stackSize,
            this,
            config_.priority,
            &handle_,
            config_.coreId
        );

        if (res != pdPASS)
        {
            ESP_LOGE(TAG, "xTaskCreate failed");
            running_ = false;
            handle_  = nullptr;
            return false;
        }

        ESP_LOGI(TAG, "Started, period=%lu ms, %u watched tasks",
                 static_cast<unsigned long>(config_.periodMs),
                 static_cast<unsigned>(watchedCount_));
        return true;
    }

    void TaskDiagnostics::stop()
    {
        running_ = false;
    }

    void TaskDiagnostics::taskEntry(void* arg)
    {
        auto* self = static_cast<TaskDiagnostics*>(arg);
        self->run();

        TaskHandle_t toDelete = self->handle_;
        self->handle_ = nullptr;
        vTaskDelete(toDelete);
    }

    void TaskDiagnostics::collect()
    {
        Models::DiagnosticsSnapshot snap {};

        snap.uptimeMs =
            static_cast<uint32_t>(esp_timer_get_time() / 1000ULL);

        snap.freeHeapBytes =
            static_cast<uint32_t>(heap_caps_get_free_size(MALLOC_CAP_DEFAULT));
        snap.minFreeHeapBytes =
            static_cast<uint32_t>(
                heap_caps_get_minimum_free_size(MALLOC_CAP_DEFAULT));
        snap.largestBlockBytes =
            static_cast<uint32_t>(
                heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT));

        // Tasks
        snap.taskCount = watchedCount_;
        uint32_t minHwm = 0xFFFFFFFFu;

        for (size_t i = 0; i < watchedCount_ && i < Models::DIAG_MAX_TASKS; ++i)
        {
            const auto& w = watched_[i];
            auto& t = snap.tasks[i];

            t.name = w.name;

            if (w.handle == nullptr)
            {
                t.alive = false;
                continue;
            }

            t.alive = true;
            const UBaseType_t hwm = uxTaskGetStackHighWaterMark(w.handle);
            t.stackHighWater = static_cast<uint32_t>(hwm);

            if (t.stackHighWater < minHwm)
            {
                minHwm = t.stackHighWater;
            }
        }
        if (minHwm != 0xFFFFFFFFu)
        {
            snap.minStackHighWater = minHwm;
        }

        // Dispatcher
        if (deps_.dispatcherStats != nullptr)
        {
            snap.dispatched = deps_.dispatcherStats->totalDispatched();
            snap.rejected   = deps_.dispatcherStats->totalRejected();
            snap.dropped    = deps_.dispatcherStats->totalDropped();
        }

        // Safety
        if (deps_.safetyMonitor != nullptr)
        {
            const auto st = deps_.safetyMonitor->getStatus();
            snap.safetyState         = static_cast<uint8_t>(st.state);
            snap.activeFaults        = static_cast<uint16_t>(st.activeFaults);
            snap.latchedFaults       = static_cast<uint16_t>(st.latchedFaults);
            snap.totalEmergencyStops = st.totalEmergencyStops;
            snap.totalRecoveries     = st.totalRecoveries;
            snap.imuHealthy          = st.imuHealthy;
            snap.emgHealthy          = st.emgHealthy;
        }

        // Sensores
        if (deps_.imuSvc != nullptr)
        {
            const auto s = deps_.imuSvc->getStatus();
            snap.imuTotalSamples = s.totalSamples;
        }
        if (deps_.emgSvc != nullptr)
        {
            const auto s = deps_.emgSvc->getStatus();
            snap.emgTotalSamples = s.totalSamples;
        }

        // Publicar snapshot bajo mutex
        if (xSemaphoreTake(mutex_, pdMS_TO_TICKS(10)) == pdTRUE)
        {
            last_ = snap;
            xSemaphoreGive(mutex_);
        }

        if (config_.logToSerial)
        {
            logSnapshot(snap);
        }
    }

    void TaskDiagnostics::logSnapshot(const Models::DiagnosticsSnapshot& s) const
    {
        ESP_LOGI(TAG,
            "uptime=%lus heap=%lu (min=%lu, blk=%lu) safety=0x%02X "
            "estops=%lu rec=%lu disp=%lu rej=%lu drop=%lu imu=%s emg=%s",
            static_cast<unsigned long>(s.uptimeMs / 1000),
            static_cast<unsigned long>(s.freeHeapBytes),
            static_cast<unsigned long>(s.minFreeHeapBytes),
            static_cast<unsigned long>(s.largestBlockBytes),
            static_cast<unsigned>(s.safetyState),
            static_cast<unsigned long>(s.totalEmergencyStops),
            static_cast<unsigned long>(s.totalRecoveries),
            static_cast<unsigned long>(s.dispatched),
            static_cast<unsigned long>(s.rejected),
            static_cast<unsigned long>(s.dropped),
            s.imuHealthy ? "Y" : "n",
            s.emgHealthy ? "Y" : "n"
        );

        // Stacks bajos sólo si hay alguno preocupante
        if (s.minStackHighWater < 512)
        {
            ESP_LOGW(TAG, "Low stack HWM: %lu words",
                     static_cast<unsigned long>(s.minStackHighWater));
        }
    }

    void TaskDiagnostics::run()
    {
        TickType_t lastWake = xTaskGetTickCount();
        const TickType_t period = pdMS_TO_TICKS(config_.periodMs);

        while (running_)
        {
            collect();
            vTaskDelayUntil(&lastWake, period);
        }
    }

    Models::DiagnosticsSnapshot TaskDiagnostics::snapshot() const
    {
        Models::DiagnosticsSnapshot copy {};
        if (mutex_ == nullptr) return copy;

        if (xSemaphoreTake(mutex_, pdMS_TO_TICKS(10)) == pdTRUE)
        {
            copy = last_;
            xSemaphoreGive(mutex_);
        }
        return copy;
    }
}