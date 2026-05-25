#pragma once

#include "interfaces/ISafetyValidator.hpp"
#include "models/ServoState.hpp"

namespace Core
{
    class BasicSafetyValidator final :
        public Interfaces::ISafetyValidator
    {
    public:
        BasicSafetyValidator() = default;

        bool validate(
            const Models::MotionCommand& command
        ) override;

        // En etapa 9 esto se sustituye por la lógica completa (límites, estados,
        // emergency lockout, etc.). Por ahora: drop NONE, valida rango de CUSTOM.
    };
}