#include "core/SystemContext.hpp"

namespace Core
{
    EventGroupHandle_t SystemContext::eventGroup_ =
        nullptr;

    bool SystemContext::initialize()
    {
        eventGroup_ = xEventGroupCreate();

        return eventGroup_ != nullptr;
    }

    EventGroupHandle_t SystemContext::eventGroup()
    {
        return eventGroup_;
    }
}