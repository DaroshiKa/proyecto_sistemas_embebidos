#pragma once

#include "models/MotionCommand.hpp"

namespace Interfaces
{
    class ISafetyValidator
    {
    public:
        virtual ~ISafetyValidator() = default;

        virtual bool validate(
            const Models::MotionCommand& command
        ) = 0;
    };
}