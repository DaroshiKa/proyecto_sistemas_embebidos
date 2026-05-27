#include "core/AdvancedSafetyValidator.hpp"

#include "esp_log.h"
#include "models/ServoState.hpp"

namespace Core
{
    static constexpr const char* TAG = "AdvSafety";

    AdvancedSafetyValidator::AdvancedSafetyValidator(
        Interfaces::ISafetyMonitor& monitor
    )
        : monitor_(monitor)
    {
    }

    bool AdvancedSafetyValidator::syntacticCheck(
        const Models::MotionCommand& cmd
    ) const
    {
        if (cmd.type == Models::MotionType::NONE)
        {
            return false;
        }

        if (cmd.type == Models::MotionType::CUSTOM_SERVO)
        {
            if (cmd.targetServo >=
                static_cast<uint8_t>(Models::JointId::COUNT))
            {
                return false;
            }
            if (cmd.targetAngle < 0.0f || cmd.targetAngle > 180.0f)
            {
                return false;
            }
        }

        return true;
    }

    bool AdvancedSafetyValidator::validate(
        const Models::MotionCommand& command
    )
    {
        // ---- Capa 1: sintaxis ----
        if (!syntacticCheck(command))
        {
            ESP_LOGW(TAG, "Syntax check failed (type=%u, servo=%u, angle=%.1f)",
                     static_cast<unsigned>(command.type),
                     static_cast<unsigned>(command.targetServo),
                     command.targetAngle);
            return false;
        }

        // ---- Capas 2 + 3: estado del sistema + salud de sensores ----
        if (!monitor_.isCommandAllowed(command))
        {
            ESP_LOGW(TAG, "Rejected by safety monitor (src=%u type=%u prio=%u)",
                     static_cast<unsigned>(command.source),
                     static_cast<unsigned>(command.type),
                     static_cast<unsigned>(command.priority));
            return false;
        }

        return true;
    }
}