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

        CALIBRATION_COMPLETE,

        // ---------- Etapa 9 ----------
        SAFETY_STATE_CHANGED,        // data → SafetyStatus*
        SAFETY_FAULT_RAISED,         // data → SafetyFault*  (la nueva falta)
        SAFETY_FAULT_CLEARED,        // data → SafetyFault*
        SAFETY_COMMAND_REJECTED,     // data → MotionCommand*
        SAFETY_RECOVERED,            // sin payload
        WATCHDOG_WARNING,            // sin payload
        HEAP_LOW                     // sin payload
    };

    struct EventMessage
    {
        EventType type { EventType::NONE };

        uint32_t timestampMs { 0 };

        void* data { nullptr };
    };
}