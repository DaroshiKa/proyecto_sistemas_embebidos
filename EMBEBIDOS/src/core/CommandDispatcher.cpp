#include "core/CommandDispatcher.hpp"

#include "esp_log.h"
#include "esp_timer.h"

namespace Core
{
    static constexpr const char* TAG = "Dispatcher";

    CommandDispatcher::CommandDispatcher(
        Interfaces::ISafetyValidator& validator,
        QueueHandle_t outputQueue,
        EventBus* eventBus
    )
        : validator_(validator),
          outputQueue_(outputQueue),
          eventBus_(eventBus)
    {
    }

    bool CommandDispatcher::dispatch(
        const Models::MotionCommand& command
    )
    {
        if (outputQueue_ == nullptr)
        {
            return false;
        }

        // 1) Validación de seguridad
        if (!validator_.validate(command))
        {
            ++totalRejected_;
            ESP_LOGW(
                TAG,
                "Command rejected: type=%u src=%u",
                static_cast<unsigned>(command.type),
                static_cast<unsigned>(command.source)
            );
            return false;
        }

        // 2) Comandos críticos: vaciar queue y encolar al frente
        if (command.priority == Models::CommandPriority::CRITICAL)
        {
            xQueueReset(outputQueue_);

            if (xQueueSendToFront(outputQueue_, &command, 0) != pdTRUE)
            {
                ++totalDropped_;
                return false;
            }
        }
        else
        {
            // 3) Comandos normales: FIFO. No bloqueamos: si está llena, drop.
            if (xQueueSend(outputQueue_, &command, 0) != pdTRUE)
            {
                ++totalDropped_;
                ESP_LOGW(TAG, "Queue full, command dropped");
                return false;
            }
        }

        ++totalDispatched_;

        if (eventBus_ != nullptr)
        {
            Models::EventMessage evt {};
            evt.type        = Models::EventType::MOTION_COMMAND_RECEIVED;
            evt.timestampMs = static_cast<uint32_t>(
                esp_timer_get_time() / 1000ULL);
            eventBus_->publish(evt);
        }

        return true;
    }
}