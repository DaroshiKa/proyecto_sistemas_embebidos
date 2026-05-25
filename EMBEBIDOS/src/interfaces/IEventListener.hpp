#pragma once

#include "models/EventMessage.hpp"

namespace Interfaces
{
    class IEventListener
    {
    public:
        virtual ~IEventListener() = default;

        virtual void onEvent(
            const Models::EventMessage& event
        ) = 0;
    };
}