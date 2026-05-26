#include "communication/NextionInterface.hpp"

#include "esp_log.h"

namespace Communication
{
    static constexpr const char* TAG = "NextionIface";

    NextionInterface::NextionInterface(
        QueueHandle_t txQueue,
        QueueHandle_t rxQueue
    )
        : txQueue_(txQueue),
          rxQueue_(rxQueue)
    {
    }

    void NextionInterface::attachCommandDispatcher(
        Interfaces::ICommandDispatcher* dispatcher
    )
    {
        dispatcher_ = dispatcher;
    }

    bool NextionInterface::initialize()
    {
        if (txQueue_ == nullptr || rxQueue_ == nullptr)
        {
            ESP_LOGE(TAG, "Queues not provided");
            return false;
        }

        ESP_LOGI(TAG, "NextionInterface ready");
        return true;
    }

    bool NextionInterface::sendTelemetry(
        const Models::TelemetryFrame& frame
    )
    {
        if (txQueue_ == nullptr) return false;

        // No bloqueamos: si el TX queue está lleno, drop.
        return xQueueSend(txQueue_, &frame, 0) == pdTRUE;
    }

    bool NextionInterface::getNextEvent(
        Models::DisplayEvent& outEvent
    )
    {
        if (rxQueue_ == nullptr) return false;

        return xQueueReceive(rxQueue_, &outEvent, 0) == pdTRUE;
    }

    bool NextionInterface::translateMotionCmd(
        const Protocols::ProtocolFrame& frame,
        Models::DisplayEvent& outEvent
    )
    {
        if (frame.length < 1) return false;

        const uint8_t mtype = frame.payload[0];
        if (mtype >= static_cast<uint8_t>(Models::MotionType::EMERGENCY_STOP) + 1)
        {
            return false;
        }

        outEvent.type           = Models::DisplayEventType::MOTION_COMMAND;
        outEvent.motionType     = static_cast<Models::MotionType>(mtype);
        outEvent.sequenceNumber = frame.seq;
        outEvent.requiresAck    = true;
        return true;
    }

    bool NextionInterface::translateServoCmd(
        const Protocols::ProtocolFrame& frame,
        Models::DisplayEvent& outEvent
    )
    {
        // payload: 1B servoId + 2B angle (centésimas, big-endian)
        if (frame.length < 3) return false;

        outEvent.type    = Models::DisplayEventType::SERVO_COMMAND;
        outEvent.servoId = frame.payload[0];

        const uint16_t raw =
            (static_cast<uint16_t>(frame.payload[1]) << 8) | frame.payload[2];

        outEvent.angle          = static_cast<float>(raw) / 100.0f;
        outEvent.sequenceNumber = frame.seq;
        outEvent.requiresAck    = true;
        return true;
    }

    bool NextionInterface::handleIncomingFrame(
        const Protocols::ProtocolFrame& frame,
        Models::DisplayEvent& outEvent
    )
    {
        outEvent = Models::DisplayEvent {};

        switch (frame.type)
        {
            case Protocols::MessageType::CMD_MOTION:
                if (!translateMotionCmd(frame, outEvent)) return false;
                break;

            case Protocols::MessageType::CMD_SERVO:
                if (!translateServoCmd(frame, outEvent)) return false;
                break;

            case Protocols::MessageType::EVT_BUTTON:
                if (frame.length < 2) return false;
                outEvent.type           = Models::DisplayEventType::BUTTON_PRESSED;
                outEvent.pageId         = frame.payload[0];
                outEvent.componentId    = frame.payload[1];
                outEvent.sequenceNumber = frame.seq;
                outEvent.requiresAck    = false;
                break;

            case Protocols::MessageType::CMD_QUERY_STATUS:
                outEvent.type           = Models::DisplayEventType::QUERY_STATUS;
                outEvent.sequenceNumber = frame.seq;
                outEvent.requiresAck    = false;
                break;

            default:
                return false;
        }

        // Si es un MOTION_COMMAND/SERVO_COMMAND y hay dispatcher, lo emitimos
        // directamente para que viaje por el dispatcher como cualquier otro.
        if (dispatcher_ != nullptr)
        {
            Models::MotionCommand cmd {};
            cmd.source      = Models::CommandSource::NEXTION;
            cmd.priority    = Models::CommandPriority::NORMAL;
            cmd.timestampMs = outEvent.timestampMs;
            cmd.requiresAck = outEvent.requiresAck;

            if (outEvent.type == Models::DisplayEventType::MOTION_COMMAND)
            {
                cmd.type = outEvent.motionType;
                dispatcher_->dispatch(cmd);
            }
            else if (outEvent.type == Models::DisplayEventType::SERVO_COMMAND)
            {
                cmd.type        = Models::MotionType::CUSTOM_SERVO;
                cmd.targetServo = outEvent.servoId;
                cmd.targetAngle = outEvent.angle;
                dispatcher_->dispatch(cmd);
            }
        }

        // También encolamos el evento por si otro consumidor del firmware lo quiere
        xQueueSend(rxQueue_, &outEvent, 0);

        return true;
    }
}