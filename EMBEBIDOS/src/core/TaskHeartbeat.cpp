// src/core/TaskHeartbeat.cpp
#include "core/TaskHeartbeat.hpp"
#include "esp_timer.h"

namespace Core
{
    static inline uint32_t nowMs()
    {
        return static_cast<uint32_t>(
            esp_timer_get_time() / 1000ULL
        );
    }

    TaskHeartbeat::TaskHeartbeat(const char* taskName)
        : name_(taskName)
    {
        mutex_ = xSemaphoreCreateMutex();
        // kick inicial para que no alarme antes de que la tarea arranque
        lastKickMs_ = nowMs();
    }

    TaskHeartbeat::~TaskHeartbeat()
    {
        if (mutex_ != nullptr)
        {
            vSemaphoreDelete(mutex_);
        }
    }

    void TaskHeartbeat::kick()
    {
        if (mutex_ == nullptr) { return; }

        if (xSemaphoreTake(mutex_, pdMS_TO_TICKS(10)) == pdTRUE)
        {
            lastKickMs_ = nowMs();
            xSemaphoreGive(mutex_);
        }
    }

    bool TaskHeartbeat::isAlive(uint32_t nowMs, uint32_t timeoutMs) const
    {
        if (mutex_ == nullptr || timeoutMs == 0U) { return true; }

        uint32_t last = 0U;

        if (xSemaphoreTake(
                mutex_,
                pdMS_TO_TICKS(10)
            ) == pdTRUE)
        {
            last = lastKickMs_;
            xSemaphoreGive(mutex_);
        }
        else
        {
            // No se pudo tomar el mutex — asumimos viva para no disparar falsos
            return true;
        }

        // Manejo de wrap-around del contador de 32 bits
        const uint32_t elapsed =
            (nowMs >= last)
                ? (nowMs - last)
                : (0xFFFFFFFFU - last + nowMs + 1U);

        return elapsed < timeoutMs;
    }

    uint32_t TaskHeartbeat::lastKickMs() const
    {
        if (mutex_ == nullptr) { return 0U; }

        uint32_t last = 0U;

        if (xSemaphoreTake(
                mutex_,
                pdMS_TO_TICKS(10)
            ) == pdTRUE)
        {
            last = lastKickMs_;
            xSemaphoreGive(mutex_);
        }

        return last;
    }
}