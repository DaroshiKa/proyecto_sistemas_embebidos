#pragma once

#include <atomic>
#include <cstdint>

#include "esp_timer.h"

namespace Core
{
    class TaskHeartbeat
    {
    public:
        TaskHeartbeat() = default;

        void kick()
        {
            const uint32_t now =
                static_cast<uint32_t>(esp_timer_get_time() / 1000ULL);
            lastKickMs_.store(now, std::memory_order_relaxed);
        }

        uint32_t lastKickMs() const
        {
            return lastKickMs_.load(std::memory_order_relaxed);
        }

        uint32_t msSinceLastKick() const
        {
            const uint32_t now =
                static_cast<uint32_t>(esp_timer_get_time() / 1000ULL);
            return now - lastKickMs_.load(std::memory_order_relaxed);
        }

        void reset()
        {
            kick();
        }

    private:
        std::atomic<uint32_t> lastKickMs_ { 0 };
    };
}