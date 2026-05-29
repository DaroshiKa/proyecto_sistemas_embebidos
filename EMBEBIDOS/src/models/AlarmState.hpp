#pragma once

#include <cstdint>

#include "MotionTypes.hpp"

namespace Models
{
    enum class AlarmCode : uint8_t
    {
        NONE = 0,
        EMERGENCY_TRIGGERED,
        IMU_TIMEOUT,
        EMG_TIMEOUT,
        MOTION_TASK_STALE,
        IMU_TASK_STALE,
        EMG_TASK_STALE,
        COMMAND_REJECTED,
        SYSTEM_RECOVERED,
        CALIBRATION_OK,
        CALIBRATION_FAILED
    };

    struct AlarmState
    {
        AlarmLevel  level       { AlarmLevel::INFO };
        AlarmCode   code        { AlarmCode::NONE };
        const char* message     { "" };
        uint32_t    timestampMs { 0 };
    };
}