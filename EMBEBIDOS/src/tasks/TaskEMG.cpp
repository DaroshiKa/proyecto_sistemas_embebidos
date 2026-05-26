#include "tasks/TaskEMG.hpp"

#include "esp_log.h"

namespace Tasks
{
    static constexpr const char* TAG = "TaskEMG";

    TaskEMG::TaskEMG(
        Services::EMGService& service,
        const TaskEMGConfig& config,
        Core::TaskHeartbeat* heartbeat                   // NUEVO
    )
        : service_(service),
          config_(config),
          heartbeat_(heartbeat)                          // NUEVO
    {
    }

    bool TaskEMG::start()
    {
        if (handle_ != nullptr) return true;

        running_ = true;

        const BaseType_t res =
            xTaskCreatePinnedToCore(
                &TaskEMG::taskEntry,
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
            "Started on core %d, priority %u",
            static_cast<int>(config_.coreId),
            static_cast<unsigned>(config_.priority)
        );
        return true;
    }

    void TaskEMG::stop()
    {
        if (handle_ == nullptr) return;
        running_ = false;
    }

    void TaskEMG::taskEntry(void* arg)
    {
        auto* self = static_cast<TaskEMG*>(arg);
        self->run();

        TaskHandle_t toDelete = self->handle_;
        self->handle_ = nullptr;
        vTaskDelete(toDelete);
    }

    void TaskEMG::run()
    {
        while (running_)
        {
            service_.update();

            // NUEVO en Etapa 9: patear heartbeat para SafetyMonitor
            if (heartbeat_ != nullptr)
            {
                heartbeat_->kick();
            }

            taskYIELD();
        }
    }
}