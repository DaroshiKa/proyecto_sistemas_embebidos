#pragma once

#include <cstdint>
#include "MotionTypes.hpp"

namespace Models
{
    struct SafetyStatus
    {
        SystemState systemState         { SystemState::INIT };

        bool        imuHealthy          { false };
        bool        emgHealthy          { false };

        bool        inEmergencyLockout  { false };
        uint32_t    emergencyLockoutRemainingMs { 0 };

        // Contadores acumulados
        uint32_t    totalCommandsValidated  { 0 };
        uint32_t    totalCommandsRejected   { 0 };
        uint32_t    totalLockoutRejections  { 0 };
        uint32_t    totalDeadtimeRejections { 0 };
        uint32_t    totalStateInvalidations { 0 };

        uint32_t    imuTimeouts        { 0 };
        uint32_t    emgTimeouts        { 0 };

        uint32_t    lastStateChangeMs  { 0 };
        AlarmCode   lastAlarm          { AlarmCode::NONE };
    };
}