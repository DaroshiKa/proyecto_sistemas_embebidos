#pragma once

#include <cstdint>

#include "MotionTypes.hpp"

namespace Models
{
    struct MotionCommand
    {
        MotionType type { MotionType::NONE };

        CommandSource source { CommandSource::UNKNOWN };

        CommandPriority priority { CommandPriority::NORMAL };

        uint32_t timestampMs { 0 };

        uint8_t targetServo { 0 };

        float targetAngle { 0.0f };

        float speed { 0.0f };

        bool requiresAck { false };
    };
}