#include "tasks/TaskSafety.hpp"

#include "esp_log.h"

namespace Tasks
{
    static constexpr const char* TAG = "TaskSafety";

    TaskSafety::TaskSafety(
        Services::SafetyService& safety,
        Services::WatchdogManager& watchdog,
        const TaskSafetyConfig& config
    )
        : safety_(safety),
          watchdog_(watchdog),
          config_(config)
    {
    }

    bool TaskSafety::start()
    {
        if (handle_ != nullptr) return true;

        running_ = true;

        const BaseType_t res = xTaskCreatePinnedToCore(
            &TaskSafety::taskEntry,
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

        ESP_LOGI(
            TAG,
            "Started core=%d period=%lums prio=%u",
            static_cast<int>(config_.coreId),
            static_cast<unsigned long>(config_.periodMs),
            static_cast<unsigned>(config_.priority)
        );
        return true;
    }

    void TaskSafety::stop()
    {
        running_ = false;
    }

    void TaskSafety::taskEntry(void* arg)
    {
        auto* self = static_cast<TaskSafety*>(arg);
        self->run();

        TaskHandle_t toDelete = self->handle_;
        self->handle_ = nullptr;
        vTaskDelete(toDelete);
    }

    void TaskSafety::run()
    {
        if (config_.useWatchdog && watchdog_.isInitialized())
        {
            watchdog_.subscribeCurrentTask();
        }

        TickType_t lastWake      = xTaskGetTickCount();
        const TickType_t period  = pdMS_TO_TICKS(config_.periodMs);

        while (running_)
        {
            safety_.update();

            if (config_.useWatchdog && watchdog_.isInitialized())
            {
                watchdog_.feed();
            }

            vTaskDelayUntil(&lastWake, period);
        }

        if (config_.useWatchdog && watchdog_.isInitialized())
        {
            watchdog_.unsubscribeCurrentTask();
        }
    }
}