#pragma once

#include <cstdint>

namespace Models
{
    enum class SafetyState : uint8_t
    {
        STARTUP             = 0,  // boot, antes de que SafetyService esté listo
        NOMINAL,                  // operación normal, sin faltas activas
        DEGRADED,                 // faltas no críticas; algunos comandos rechazados
        EMERGENCY_STOP_HOLD,      // actuadores detenidos, solo recovery
        RECOVERING,               // intentando volver a NOMINAL
        FATAL                     // estado terminal, requiere reset HW
    };

    inline const char* safetyStateToString(SafetyState s)
    {
        switch (s)
        {
            case SafetyState::STARTUP:             return "STARTUP";
            case SafetyState::NOMINAL:             return "NOMINAL";
            case SafetyState::DEGRADED:            return "DEGRADED";
            case SafetyState::EMERGENCY_STOP_HOLD: return "E_STOP_HOLD";
            case SafetyState::RECOVERING:          return "RECOVERING";
            case SafetyState::FATAL:               return "FATAL";
        }
        return "?";
    }
}
