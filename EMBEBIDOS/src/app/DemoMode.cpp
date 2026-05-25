#include "app/DemoMode.hpp"

#include "esp_log.h"

namespace App
{
    static constexpr const char* TAG = "DemoMode";

    constexpr DemoMode::Step DemoMode::SEQUENCE_[];

    DemoMode::DemoMode(Interfaces::ICommandDispatcher& dispatcher)
        : dispatcher_(dispatcher)
    {
    }

    void DemoMode::start(uint32_t nowMs)
    {
        if (running_) return;

        running_     = true;
        stepIndex_   = 0;
        stepStartMs_ = nowMs;

        ESP_LOGI(TAG, "Demo started (%u steps)",
                 static_cast<unsigned>(SEQUENCE_LEN_));

        dispatchStep(SEQUENCE_[0], nowMs);
    }

    void DemoMode::stop()
    {
        if (!running_) return;

        running_ = false;
        ESP_LOGI(TAG, "Demo stopped at step %u",
                 static_cast<unsigned>(stepIndex_));

        // Al detener el demo, mandamos a home
        Models::MotionCommand homeCmd {};
        homeCmd.type        = Models::MotionType::HOME;
        homeCmd.source      = Models::CommandSource::DEMO;
        homeCmd.priority    = Models::CommandPriority::NORMAL;
        dispatcher_.dispatch(homeCmd);
    }

    void DemoMode::dispatchStep(const Step& s, uint32_t nowMs)
    {
        Models::MotionCommand cmd {};
        cmd.type        = s.type;
        cmd.source      = Models::CommandSource::DEMO;
        cmd.priority    = Models::CommandPriority::NORMAL;
        cmd.timestampMs = nowMs;
        cmd.requiresAck = false;

        dispatcher_.dispatch(cmd);
    }

    void DemoMode::tick(uint32_t nowMs)
    {
        if (!running_) return;

        const Step& current = SEQUENCE_[stepIndex_];

        if ((nowMs - stepStartMs_) < current.durationMs)
        {
            return;
        }

        // Avanzar al siguiente paso (loop circular)
        stepIndex_ = (stepIndex_ + 1) % SEQUENCE_LEN_;
        stepStartMs_ = nowMs;

        dispatchStep(SEQUENCE_[stepIndex_], nowMs);
    }
}