#pragma once

#include "MotionTypes.hpp"

namespace Models
{
    struct AlarmState
    {
        AlarmLevel level { AlarmLevel::INFO };

        const char* message { "" };

        uint32_t timestampMs { 0 };
    };
}