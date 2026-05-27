#include "tasks/TaskSystemMonitor.hpp"

#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_timer.h"

namespace Tasks
{
    static constexpr const char* TAG = "SysMon";

    TaskSystemMonitor::TaskSystemMonitor(
        Services::SafetyService& safety,
        const TaskSystemMonitorConfig& config
    )
        : safety_(safety),
          config_(config)
    {
    }

    bool TaskSystemMonitor::registerWatchedTask(
        TaskHandle_t handle,
        const char* name
    )
    {
        if (watchedCount_ >= MAX_WATCHED) return false;
        if (handle == nullptr) return false;

        watched_[watchedCount_].handle = handle;
        watched_[watchedCount_].name   = (name != nullptr) ? name : "?";
        ++watchedCount_;
        return true;
    }

    bool TaskSystemMonitor::start()
    {
        if (handle_ != nullptr) return true;

        running_ = true;

        const BaseType_t res = xTaskCreatePinnedToCore(
            &TaskSystemMonitor::taskEntry,
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

        ESP_LOGI(TAG, "Started, period %lu ms",
                 static_cast<unsigned long>(config_.periodMs));
        return true;
    }

    void TaskSystemMonitor::stop()
    {
        running_ = false;
    }

    void TaskSystemMonitor::taskEntry(void* arg)
    {
        auto* self = static_cast<TaskSystemMonitor*>(arg);
        self->run();

        TaskHandle_t toDelete = self->handle_;
        self->handle_ = nullptr;
        vTaskDelete(toDelete);
    }

    void TaskSystemMonitor::run()
    {
        TickType_t lastWake     = xTaskGetTickCount();
        const TickType_t period = pdMS_TO_TICKS(config_.periodMs);

        while (running_)
        {
            const uint32_t nowMs =
                static_cast<uint32_t>(esp_timer_get_time() / 1000ULL);

            const uint32_t freeHeap =
                static_cast<uint32_t>(heap_caps_get_free_size(MALLOC_CAP_DEFAULT));
            const uint32_t minHeap =
                static_cast<uint32_t>(heap_caps_get_minimum_free_size(MALLOC_CAP_DEFAULT));

            ESP_LOGI(TAG,
                     "uptime=%lus  heap_free=%lu  heap_min=%lu",
                     static_cast<unsigned long>(nowMs / 1000),
                     static_cast<unsigned long>(freeHeap),
                     static_cast<unsigned long>(minHeap));

            // Stack high-water mark de cada tarea registrada
            for (size_t i = 0; i < watchedCount_; ++i)
            {
                const auto& w = watched_[i];
                if (w.handle == nullptr) continue;

                const UBaseType_t hwm =
                    uxTaskGetStackHighWaterMark(w.handle);

                ESP_LOGI(TAG, "  task '%s': hwm=%u words", w.name,
                         static_cast<unsigned>(hwm));

                if (hwm < config_.stackLowThresholdWords)
                {
                    // SafetyService consume esto como falta DEGRADED.
                    safety_.triggerEmergencyStop(
                        Models::SafetyFault::STACK_LOW
                    );
                    // Nota: triggerEmergencyStop es un upgrade severo;
                    // si quieres degradar en lugar de E-Stop, podrías
                    // exponer un método raiseStackLow() en SafetyService.
                    // Aquí elegimos la opción conservadora.
                }
            }

            vTaskDelayUntil(&lastWake, period);
        }
    }
}