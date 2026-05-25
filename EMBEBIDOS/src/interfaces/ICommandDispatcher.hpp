#pragma once

#include "models/MotionCommand.hpp"

namespace Interfaces
{
    class ICommandDispatcher
    {
    public:
        virtual ~ICommandDispatcher() = default;

        virtual bool dispatch(
            const Models::MotionCommand& command
        ) = 0;
    };
}