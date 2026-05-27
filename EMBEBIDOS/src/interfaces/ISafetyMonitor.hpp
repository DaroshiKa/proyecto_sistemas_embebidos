#pragma once

#include "models/SafetyStatus.hpp"
#include "models/SafetyFault.hpp"
#include "models/MotionCommand.hpp"

namespace Interfaces
{
    class ISafetyMonitor
    {
    public:
        virtual ~ISafetyMonitor() = default;

        // Lectura no bloqueante (copia snapshot bajo mutex interno)
        virtual Models::SafetyStatus getStatus() const = 0;

        // ¿Se permite ejecutar este comando ahora mismo?
        // Esta es la API que consume el AdvancedSafetyValidator.
        virtual bool isCommandAllowed(
            const Models::MotionCommand& cmd
        ) const = 0;

        // Acciones de control
        virtual void triggerEmergencyStop(
            Models::SafetyFault cause = Models::SafetyFault::USER_REQUESTED_ESTOP
        ) = 0;

        // Solicitar salida del estado EMERGENCY_STOP_HOLD.
        // Retorna false si las condiciones no permiten recovery (faltas
        // todavía activas).
        virtual bool requestRecovery() = 0;

        // Limpieza forzada de faltas latched. Sólo desde CLI con usuario.
        virtual void clearLatchedFaults() = 0;
    };
}