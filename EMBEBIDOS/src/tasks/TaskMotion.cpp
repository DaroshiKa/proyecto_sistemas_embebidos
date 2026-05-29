#include "tasks/TaskMotion.hpp"
#include "services/WatchdogManager.hpp"

#include "esp_log.h"
#include "esp_timer.h"

namespace Tasks
{
    static constexpr const char* TAG = "TaskMotion";

    TaskMotion::TaskMotion(
        Services::MotionService& motionService,
        Drivers::ServoManager& servoManager,
        QueueHandle_t commandQueue,
        const TaskMotionConfig& config
    )
        : motion_(motionService),
          servos_(servoManager),
          queue_(commandQueue),
          config_(config)
    {
    }

    void TaskMotion::attachWatchdog(Services::WatchdogManager* wdt)
    {
        watchdog_ = wdt;
    }

    bool TaskMotion::start()
    {
        if (handle_ != nullptr) return true;
        if (queue_ == nullptr)
        {
            ESP_LOGE(TAG, "Queue handle is null");
            return false;
        }

        running_ = true;

        const BaseType_t res = xTaskCreatePinnedToCore(
            &TaskMotion::taskEntry,
            config_.name,
            config_.stackSize,
            this,
            config_.priority,
            &handle_,
            config_.coreId
        );

        if (res != pdPASS)
        {
            ESP_LOGE(TAG, "xTaskCreatePinnedToCore failed");
            running_ = false;
            handle_  = nullptr;
            return false;
        }

        return true;
    }

    void TaskMotion::stop()
    {
        running_ = false;
    }

    void TaskMotion::taskEntry(void* arg)
    {
        auto* self = static_cast<TaskMotion*>(arg);
        self->run();

        TaskHandle_t toDelete = self->handle_;
        self->handle_ = nullptr;
        vTaskDelete(toDelete);
    }

    void TaskMotion::run()
    {
        const bool wdtAvailable =
            config_.useWatchdog &&
            watchdog_ != nullptr &&
            watchdog_->isInitialized();

        if (wdtAvailable)
        {
            watchdog_->subscribeCurrentTask();
        }

        TickType_t lastWake     = xTaskGetTickCount();
        const TickType_t period = pdMS_TO_TICKS(config_.periodMs);

        while (running_)
        {
            const uint32_t now =
                static_cast<uint32_t>(esp_timer_get_time() / 1000ULL);

            Models::MotionCommand cmd {};
            while (xQueueReceive(queue_, &cmd, 0) == pdTRUE)
            {
                motion_.processMotionCommand(cmd, now);
            }

            servos_.tick(now);

            if (wdtAvailable)
            {
                watchdog_->feed();
            }

            vTaskDelayUntil(&lastWake, period);
        }

        if (wdtAvailable)
        {
            watchdog_->unsubscribeCurrentTask();
        }
    }
}