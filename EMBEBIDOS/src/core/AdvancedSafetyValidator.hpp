#pragma once

#include "interfaces/ISafetyValidator.hpp"
#include "interfaces/ISafetyMonitor.hpp"

namespace Core
{
    class AdvancedSafetyValidator final :
        public Interfaces::ISafetyValidator
    {
    public:
        explicit AdvancedSafetyValidator(
            Interfaces::ISafetyMonitor& monitor
        );

        // ISafetyValidator
        bool validate(
            const Models::MotionCommand& command
        ) override;

    private:
        // Capa 1: validación sintáctica (rango, tipo, servo ID)
        bool syntacticCheck(const Models::MotionCommand& cmd) const;

        Interfaces::ISafetyMonitor& monitor_;
    };
}