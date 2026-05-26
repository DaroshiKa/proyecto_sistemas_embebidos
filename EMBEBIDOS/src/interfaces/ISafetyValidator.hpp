#pragma once

#include "models/MotionCommand.hpp"
#include "models/MotionTypes.hpp"

namespace Interfaces
{
    class ISafetyValidator
    {
    public:
        virtual ~ISafetyValidator() = default;

        virtual bool validate(
            const Models::MotionCommand& command
        ) = 0;

        // Notificación de cambio de estado (opcional; default no-op).
        // Implementaciones que necesiten contexto lo sobreescriben.
        virtual void onSystemStateChanged(
            Models::SystemState newState
        )
        {
            (void)newState;
        }
    };
}