#pragma once

#include "models/MotionCommand.hpp"

namespace Interfaces
{
    class ICommandSource
    {
    public:
        virtual ~ICommandSource() = default;

        virtual bool getNextCommand(
            Models::MotionCommand& command
        ) = 0;
    };
}