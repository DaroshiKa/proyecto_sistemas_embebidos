#include "tasks/TaskCLI.hpp"

#include "esp_log.h"
#include "esp_timer.h"

namespace Tasks
{
    static constexpr const char* TAG = "TaskCLI";

    TaskCLI::TaskCLI(
        Communication::UARTConsole& console,
        Services::CLIService& cliService,
        App::DemoMode& demoMode,
        const TaskCLIConfig& config
    )
        : console_(console),
          cli_(cliService),
          demo_(demoMode),
          config_(config)
    {
    }

    bool TaskCLI::start()
    {
        if (handle_ != nullptr) return true;

        running_ = true;
        const BaseType_t res = xTaskCreatePinnedToCore(
            &TaskCLI::taskEntry,
            config_.name,
            config_.stackSize,
            this,
            config_.priority,
            &handle_,
            config_.coreId
        );

        if (res != pdPASS)
        {
            ESP_LOGE(TAG, "task create failed");
            running_ = false;
            handle_  = nullptr;
            return false;
        }

        ESP_LOGI(TAG, "Started on core %d", static_cast<int>(config_.coreId));
        return true;
    }

    void TaskCLI::stop()
    {
        running_ = false;
    }

    void TaskCLI::taskEntry(void* arg)
    {
        auto* self = static_cast<TaskCLI*>(arg);
        self->run();

        TaskHandle_t toDelete = self->handle_;
        self->handle_ = nullptr;
        vTaskDelete(toDelete);
    }

    void TaskCLI::run()
    {
        // Pequeña espera para que el banner no quede solapado con logs de boot
        vTaskDelay(pdMS_TO_TICKS(500));

        cli_.printBanner();
        cli_.printPrompt();

        char lineBuf[Communication::UARTConsole::LINE_BUFFER_SIZE];

        while (running_)
        {
            // Lectura no bloqueante con timeout corto: permite tick del demo
            const bool gotLine = console_.readLine(lineBuf, sizeof(lineBuf), 50);

            if (gotLine)
            {
                // Interceptar 'demo start' y 'demo stop' para conectar con DemoMode
                if (lineBuf[0] != '\0')
                {
                    const uint32_t now =
                        static_cast<uint32_t>(esp_timer_get_time() / 1000ULL);

                    // Detección rápida para activar/desactivar el demo loop
                    if (strncmp(lineBuf, "demo start", 10) == 0)
                    {
                        demo_.start(now);
                    }
                    else if (strncmp(lineBuf, "demo stop", 9) == 0)
                    {
                        demo_.stop();
                    }

                    cli_.handleLine(lineBuf);
                }

                cli_.printPrompt();
            }

            // Tick del demo (avanza si está corriendo)
            const uint32_t now =
                static_cast<uint32_t>(esp_timer_get_time() / 1000ULL);
            demo_.tick(now);
        }
    }
}