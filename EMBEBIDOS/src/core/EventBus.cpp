#include "core/EventBus.hpp"

namespace Core
{
    void EventBus::subscribe(
        Interfaces::IEventListener* listener
    )
    {
        listeners_.push_back(listener);
    }

    void EventBus::publish(
        const Models::EventMessage& event
    )
    {
        for (auto* listener : listeners_)
        {
            if (listener != nullptr)
            {
                listener->onEvent(event);
            }
        }
    }
}