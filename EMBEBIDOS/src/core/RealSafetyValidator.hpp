#pragma once

#include <cstdint>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include "interfaces/ISafetyValidator.hpp"

#include "models/MotionCommand.hpp"
#include "models/MotionTypes.hpp"
#include "models/SafetyConfig.hpp"
#include "models/ServoState.hpp"

namespace Core
{
    class RealSafetyValidator final :
        public Interfaces::ISafetyValidator
    {
    public:
        explicit RealSafetyValidator(
            const Models::SafetyConfig& config
        );

        bool validate(
            const Models::MotionCommand& command
        ) override;

        void onSystemStateChanged(
            Models::SystemState newState
        ) override;

        Models::SystemState currentState() const { return systemState_; }

        // Stats
        uint32_t totalAccepted() const { return totalAccepted_; }
        uint32_t totalRejectedState()  const { return rejByState_; }
        uint32_t totalRejectedLimits() const { return rejByLimits_; }
        uint32_t totalRejectedDeadtime() const { return rejByDeadtime_; }

    private:
        // ¿Está permitido este tipo de comando en el estado actual?
        bool isAllowedInState(
            Models::MotionType type,
            Models::CommandSource source
        ) const;

        // Comprueba límites mecánicos para CUSTOM_SERVO
        bool checkServoLimits(
            const Models::MotionCommand& cmd
        ) const;

        // Anti-bounce: rechaza comandos demasiado seguidos del mismo origen
        bool checkDeadtime(
            const Models::MotionCommand& cmd
        );

        Models::SafetyConfig config_;
        Models::SystemState  systemState_ { Models::SystemState::INIT };

        // Último timestamp aceptado por fuente
        uint32_t lastAcceptedMsBySource_[8] {};  // indexado por CommandSource

        // Stats
        uint32_t totalAccepted_   { 0 };
        uint32_t rejByState_      { 0 };
        uint32_t rejByLimits_     { 0 };
        uint32_t rejByDeadtime_   { 0 };
    };
}