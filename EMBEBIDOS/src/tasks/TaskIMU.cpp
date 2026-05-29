#include "tasks/TaskIMU.hpp"

#include "esp_log.h"

namespace Tasks
{
    static constexpr const char* TAG = "TaskIMU";

    TaskIMU::TaskIMU(
        Services::IMUService& service,
        const TaskIMUConfig& config
    )
        : service_(service),
          config_(config)
    {
    }

    bool TaskIMU::start()
    {
        if (handle_ != nullptr)
        {
            return true;
        }

        running_ = true;

        const BaseType_t res =
            xTaskCreatePinnedToCore(
                &TaskIMU::taskEntry,
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
            handle_ = nullptr;
            return false;
        }

        ESP_LOGI(
            TAG,
            "Started on core %d, period %lu ms",
            static_cast<int>(config_.coreId),
            static_cast<unsigned long>(config_.periodMs)
        );

        return true;
    }

    void TaskIMU::stop()
    {
        if (handle_ == nullptr)
        {
            return;
        }

        running_ = false;

        // Cooperativo: el loop saldrá en la siguiente iteración
    }

    void TaskIMU::taskEntry(void* arg)
    {
        auto* self = static_cast<TaskIMU*>(arg);
        self->run();

        // Si run() retorna, la tarea se destruye limpiamente
        TaskHandle_t toDelete = self->handle_;
        self->handle_ = nullptr;
        vTaskDelete(toDelete);
    }

    void TaskIMU::run()
    {
        TickType_t lastWake = xTaskGetTickCount();
        const TickType_t period = pdMS_TO_TICKS(config_.periodMs);

        while (running_)
        {
            service_.update();

            // Loop cadenciado: vTaskDelayUntil garantiza periodicidad estricta
            vTaskDelayUntil(&lastWake, period);
        }
    }
}