#pragma once

#include "MotionTypes.hpp"

namespace Models
{
    struct SystemStatus
    {
        SystemState state { SystemState::INIT };

        bool emgConnected { false };

        bool imuConnected { false };

        bool nextionConnected { false };

        bool emergencyStopActive { false };

        uint32_t uptimeMs { 0 };
    };
}