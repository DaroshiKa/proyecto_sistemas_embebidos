#include "core/CommandDispatcher.hpp"

namespace Core
{
    CommandDispatcher::CommandDispatcher(
        Interfaces::ISafetyValidator& validator
    )
        : validator_(validator)
    {
    }

    bool CommandDispatcher::dispatch(
        const Models::MotionCommand& command
    )
    {
        return validator_.validate(command);
    }
}