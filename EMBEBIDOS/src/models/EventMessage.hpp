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
        IMU_UPDATED,
        EMG_UPDATED,
        SERVO_STATE_CHANGED,

        DISPLAY_BUTTON_PRESSED,

        SYSTEM_ERROR,
        SYSTEM_STATE_CHANGED,      // ← NUEVO (Etapa 9)
        SAFETY_ALARM,              // ← NUEVO (Etapa 9)
        SENSOR_TIMEOUT,            // ← NUEVO (Etapa 9)
        SENSOR_RECONNECTED,        // ← NUEVO (Etapa 9)
        WATCHDOG_WARNING,          // ← NUEVO (Etapa 9)

        EMERGENCY_TRIGGERED,
        EMERGENCY_CLEARED,         // ← NUEVO (Etapa 9)

        CALIBRATION_COMPLETE
    };

    struct EventMessage
    {
        EventType type { EventType::NONE };

        uint32_t timestampMs { 0 };

        // Campos opcionales para eventos enriquecidos (Etapa 9):
        // - newState: usado por SYSTEM_STATE_CHANGED
        // - alarmCode: usado por SAFETY_ALARM, SENSOR_TIMEOUT, etc.
        // - alarmLevel: severidad de la alarma
        uint8_t  newState   { 0 };
        uint8_t  alarmCode  { 0 };
        uint8_t  alarmLevel { 0 };

        void* data { nullptr };
    };
}