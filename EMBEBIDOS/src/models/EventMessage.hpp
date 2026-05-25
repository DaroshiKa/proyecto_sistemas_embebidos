#pragma once

#include <cstdint>

namespace Models
{
    enum class EventType : uint8_t
    {
        NONE = 0,

        MOTION_COMMAND_RECEIVED,
        MOTION_EXECUTED,

        SENSOR_UPDATED,

        SYSTEM_ERROR,

        EMERGENCY_TRIGGERED,

        CALIBRATION_COMPLETE
    };

    struct EventMessage
    {
        EventType type { EventType::NONE };

        uint32_t timestampMs { 0 };

        void* data { nullptr };
    };
}