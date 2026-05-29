#include "tasks/TaskSafety.hpp"

#include "esp_log.h"
<<<<<<< HEAD
=======
#include "esp_timer.h"
#include "esp_task_wdt.h"
>>>>>>> 0f18090e8f7bddf39123306abf466af425290d60

namespace Tasks
{
    static constexpr const char* TAG = "TaskSafety";

    TaskSafety::TaskSafety(
<<<<<<< HEAD
        Services::SafetyService& safety,
        Services::WatchdogManager& watchdog,
        const TaskSafetyConfig& config
    )
        : safety_(safety),
          watchdog_(watchdog),
=======
        Services::SafetyService& service,
        const TaskSafetyConfig& config
    )
        : service_(service),
>>>>>>> 0f18090e8f7bddf39123306abf466af425290d60
          config_(config)
    {
    }

    bool TaskSafety::start()
    {
        if (handle_ != nullptr) return true;

        running_ = true;
<<<<<<< HEAD

        const BaseType_t res = xTaskCreatePinnedToCore(
=======
        const BaseType_t ok = xTaskCreatePinnedToCore(
>>>>>>> 0f18090e8f7bddf39123306abf466af425290d60
            &TaskSafety::taskEntry,
            config_.name,
            config_.stackSize,
            this,
            config_.priority,
            &handle_,
            config_.coreId
        );

<<<<<<< HEAD
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
=======
        if (ok != pdPASS)
        {
            ESP_LOGE(TAG, "Failed to create TaskSafety");
            running_ = false;
            handle_ = nullptr;
            return false;
        }

        ESP_LOGI(TAG, "TaskSafety started (period=%lums)",
            (unsigned long)config_.periodMs);
>>>>>>> 0f18090e8f7bddf39123306abf466af425290d60
        return true;
    }

    void TaskSafety::stop()
    {
        running_ = false;
<<<<<<< HEAD
=======
        // El task termina cuando vea running_ == false.
        // No usamos vTaskDelete desde fuera por seguridad.
>>>>>>> 0f18090e8f7bddf39123306abf466af425290d60
    }

    void TaskSafety::taskEntry(void* arg)
    {
        auto* self = static_cast<TaskSafety*>(arg);
        self->run();
<<<<<<< HEAD

        TaskHandle_t toDelete = self->handle_;
        self->handle_ = nullptr;
        vTaskDelete(toDelete);
=======
        self->handle_ = nullptr;
        vTaskDelete(nullptr);
>>>>>>> 0f18090e8f7bddf39123306abf466af425290d60
    }

    void TaskSafety::run()
    {
<<<<<<< HEAD
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
=======
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
>>>>>>> 0f18090e8f7bddf39123306abf466af425290d60
            }

            vTaskDelayUntil(&lastWake, period);
        }

<<<<<<< HEAD
        if (config_.useWatchdog && watchdog_.isInitialized())
        {
            watchdog_.unsubscribeCurrentTask();
=======
        if (config_.registerInTWDT)
        {
            esp_task_wdt_delete(nullptr);
>>>>>>> 0f18090e8f7bddf39123306abf466af425290d60
        }
    }
}