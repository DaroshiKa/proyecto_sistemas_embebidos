#pragma once

#include <vector>

#include "interfaces/IEventListener.hpp"
#include "models/EventMessage.hpp"

namespace Core
{
    class EventBus
    {
    public:
        void subscribe(
            Interfaces::IEventListener* listener
        );

        void publish(
            const Models::EventMessage& event
        );

    private:
        std::vector<Interfaces::IEventListener*> listeners_;
    };
}