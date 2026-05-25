#include "core/BasicSafetyValidator.hpp"

namespace Core
{
    bool BasicSafetyValidator::validate(
        const Models::MotionCommand& command
    )
    {
        if (command.type == Models::MotionType::NONE)
        {
            return false;
        }

        if (command.type == Models::MotionType::CUSTOM_SERVO)
        {
            if (command.targetServo >=
                static_cast<uint8_t>(Models::JointId::COUNT))
            {
                return false;
            }

            if (command.targetAngle < 0.0f ||
                command.targetAngle > 180.0f)
            {
                return false;
            }
        }

        return true;
    }
}