#include "tasks/TaskMotion.hpp"

#include "esp_log.h"
#include "esp_timer.h"

namespace Tasks
{
    static constexpr const char* TAG = "TaskMotion";

    TaskMotion::TaskMotion(
        Services::MotionService& motionService,
        Drivers::ServoManager& servoManager,
        QueueHandle_t commandQueue,
        const TaskMotionConfig& config,
        Core::TaskHeartbeat* heartbeat
    )
        : motion_(motionService),
          servos_(servoManager),
          queue_(commandQueue),
          config_(config),
          heartbeat_(heartbeat)
    {
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

        ESP_LOGI(
            TAG,
            "Started on core %d, period %lu ms, priority %u",
            static_cast<int>(config_.coreId),
            static_cast<unsigned long>(config_.periodMs),
            static_cast<unsigned>(config_.priority)
        );
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
        TickType_t lastWake = xTaskGetTickCount();
        const TickType_t period = pdMS_TO_TICKS(config_.periodMs);

        while (running_)
        {
            const uint32_t now =
                static_cast<uint32_t>(esp_timer_get_time() / 1000ULL);

            // 1) Drenar TODOS los comandos pendientes (no esperar)
            Models::MotionCommand cmd {};
            while (xQueueReceive(queue_, &cmd, 0) == pdTRUE)
            {
                motion_.processMotionCommand(cmd, now);
            }

            // 2) Tick del bucle de control de servos (interpolación)
            servos_.tick(now);

            // 3) NUEVO en Etapa 9: patear el heartbeat para SafetyMonitor
            if (heartbeat_ != nullptr)
            {
                heartbeat_->kick();
            }

            vTaskDelayUntil(&lastWake, period);
        }
    }
}