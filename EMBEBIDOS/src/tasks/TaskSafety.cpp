#include "tasks/TaskSafety.hpp"

#include "esp_log.h"
#include "esp_timer.h"
#include "esp_task_wdt.h"

namespace Tasks
{
    static constexpr const char* TAG = "TaskSafety";

    TaskSafety::TaskSafety(
        Services::SafetyService& service,
        const TaskSafetyConfig& config
    )
        : service_(service),
          config_(config)
    {
    }

    bool TaskSafety::start()
    {
        if (handle_ != nullptr) return true;

        running_ = true;
        const BaseType_t ok = xTaskCreatePinnedToCore(
            &TaskSafety::taskEntry,
            config_.name,
            config_.stackSize,
            this,
            config_.priority,
            &handle_,
            config_.coreId
        );

        if (ok != pdPASS)
        {
            ESP_LOGE(TAG, "Failed to create TaskSafety");
            running_ = false;
            handle_ = nullptr;
            return false;
        }

        ESP_LOGI(TAG, "TaskSafety started (period=%lums)",
            (unsigned long)config_.periodMs);
        return true;
    }

    void TaskSafety::stop()
    {
        running_ = false;
        // El task termina cuando vea running_ == false.
        // No usamos vTaskDelete desde fuera por seguridad.
    }

    void TaskSafety::taskEntry(void* arg)
    {
        auto* self = static_cast<TaskSafety*>(arg);
        self->run();
        self->handle_ = nullptr;
        vTaskDelete(nullptr);
    }

    void TaskSafety::run()
    {
        // Registro en TWDT
        if (config_.registerInTWDT)
        {
            const esp_err_t e = esp_task_wdt_add(nullptr);
            if (e != ESP_OK && e != ESP_ERR_INVALID_ARG)
            {
                ESP_LOGW(TAG, "esp_task_wdt_add returned %d", (int)e);
            }
        }

        TickType_t lastWake = xTaskGetTickCount();
        const TickType_t period = pdMS_TO_TICKS(config_.periodMs);

        while (running_)
        {
            const uint32_t nowMs =
                static_cast<uint32_t>(esp_timer_get_time() / 1000ULL);

            service_.tick(nowMs);

            if (config_.registerInTWDT)
            {
                esp_task_wdt_reset();
            }

            vTaskDelayUntil(&lastWake, period);
        }

        if (config_.registerInTWDT)
        {
            esp_task_wdt_delete(nullptr);
        }
    }
}