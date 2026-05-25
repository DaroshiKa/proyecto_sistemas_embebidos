#pragma once

#include "interfaces/ICommandDispatcher.hpp"
#include "interfaces/ISafetyValidator.hpp"

namespace Core
{
    class CommandDispatcher final :
        public Interfaces::ICommandDispatcher
    {
    public:
        explicit CommandDispatcher(
            Interfaces::ISafetyValidator& validator
        );

        bool dispatch(
            const Models::MotionCommand& command
        ) override;

    private:
        Interfaces::ISafetyValidator& validator_;
    };
}