#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include "interfaces/ICommandDispatcher.hpp"
#include "interfaces/ISafetyValidator.hpp"

#include "core/EventBus.hpp"

namespace Core
{
    class CommandDispatcher final :
        public Interfaces::ICommandDispatcher
    {
    public:
        CommandDispatcher(
            Interfaces::ISafetyValidator& validator,
            QueueHandle_t outputQueue,
            EventBus* eventBus = nullptr
        );

        bool dispatch(
            const Models::MotionCommand& command
        ) override;

        // Stats para diagnóstico
        uint32_t totalDispatched() const { return totalDispatched_; }
        uint32_t totalRejected()   const { return totalRejected_; }
        uint32_t totalDropped()    const { return totalDropped_; }

    private:
        Interfaces::ISafetyValidator& validator_;
        QueueHandle_t                 outputQueue_;
        EventBus*                     eventBus_;

        uint32_t totalDispatched_ { 0 };
        uint32_t totalRejected_   { 0 };
        uint32_t totalDropped_    { 0 };
    };
}