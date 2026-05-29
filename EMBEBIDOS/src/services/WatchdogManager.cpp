#include "services/WatchdogManager.hpp"

#include "esp_task_wdt.h"
#include "esp_log.h"

namespace Services
{
    static constexpr const char* TAG = "Watchdog";

    // Punteros estáticos: el TWDT de IDF llama a esp_task_wdt_user_handler()
    // (símbolo weak) cuando una task no alimenta. Lo proveemos abajo.
    static WatchdogTriggerCallback s_cb       = nullptr;
    static void*                   s_userArg  = nullptr;

    bool WatchdogManager::initialize(
        uint32_t timeoutMs,
        bool panicOnTrigger,
        WatchdogTriggerCallback cb,
        void* userArg
    )
    {
        if (initialized_) return true;

        timeoutMs_      = timeoutMs;
        panicOnTrigger_ = panicOnTrigger;
        cb_             = cb;
        userArg_        = userArg;

        s_cb      = cb;
        s_userArg = userArg;

        esp_task_wdt_config_t cfg = {};
        cfg.timeout_ms     = timeoutMs;
        cfg.idle_core_mask = 0;           // no monitoreamos las idle tasks
        cfg.trigger_panic  = panicOnTrigger;

        // Si el TWDT ya fue inicializado por la configuración por defecto
        // de ESP-IDF (CONFIG_ESP_TASK_WDT_INIT=y), reconfiguramos.
        esp_err_t err = esp_task_wdt_reconfigure(&cfg);
        if (err == ESP_ERR_INVALID_STATE)
        {
            // No estaba inicializado todavía: lo inicializamos.
            err = esp_task_wdt_init(&cfg);
        }

        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "TWDT init failed: %s", esp_err_to_name(err));
            return false;
        }

        initialized_ = true;
        ESP_LOGI(TAG, "TWDT armed: %lu ms, panic=%d",
                 static_cast<unsigned long>(timeoutMs),
                 static_cast<int>(panicOnTrigger));
        return true;
    }

    bool WatchdogManager::subscribeCurrentTask()
    {
        const esp_err_t err = esp_task_wdt_add(nullptr);
        if (err != ESP_OK && err != ESP_ERR_INVALID_ARG)
        {
            ESP_LOGW(TAG, "subscribe failed: %s", esp_err_to_name(err));
            return false;
        }
        return true;
    }

    bool WatchdogManager::unsubscribeCurrentTask()
    {
        const esp_err_t err = esp_task_wdt_delete(nullptr);
        return err == ESP_OK;
    }

    bool WatchdogManager::feed()
    {
        return esp_task_wdt_reset() == ESP_OK;
    }
}

// ----------------------------------------------------------------------------
// User handler. Símbolo weak en ESP-IDF; al definirlo aquí, el TWDT
// nos invoca cuando una task subscrita no alimentó dentro del timeout.
// IMPORTANTE: este callback se ejecuta en contexto de la timer-task del
// TWDT; debe ser corto y NO bloquear. Aquí únicamente reenviamos al
// callback registrado por SafetyService.
// ----------------------------------------------------------------------------
extern "C" void esp_task_wdt_isr_user_handler(void)
{
    if (Services::s_cb != nullptr)
    {
        Services::s_cb(Services::s_userArg);
    }
}