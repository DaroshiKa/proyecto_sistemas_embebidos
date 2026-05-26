#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include "interfaces/IDisplayInterface.hpp"
#include "interfaces/ICommandDispatcher.hpp"

#include "protocols/NextionProtocol.hpp"

namespace Communication
{
    class NextionInterface final :
        public Interfaces::IDisplayInterface
    {
    public:
        NextionInterface(
            QueueHandle_t txQueue,   // TelemetryFrame para enviar
            QueueHandle_t rxQueue    // DisplayEvent recibidos
        );

        void attachCommandDispatcher(
            Interfaces::ICommandDispatcher* dispatcher
        );

        // ----- IDisplayInterface -----
        bool initialize() override;
        bool isConnected() const override { return connected_; }

        bool sendTelemetry(
            const Models::TelemetryFrame& frame
        ) override;

        bool getNextEvent(
            Models::DisplayEvent& outEvent
        ) override;

        // Helper público para que TaskNextionInterface lo use al parsear:
        // traduce un ProtocolFrame entrante a DisplayEvent y opcionalmente
        // dispatcha un MotionCommand directamente.
        bool handleIncomingFrame(
            const Protocols::ProtocolFrame& frame,
            Models::DisplayEvent& outEvent
        );

        void setConnected(bool c) { connected_ = c; }

    private:
        bool translateMotionCmd(
            const Protocols::ProtocolFrame& frame,
            Models::DisplayEvent& outEvent
        );

        bool translateServoCmd(
            const Protocols::ProtocolFrame& frame,
            Models::DisplayEvent& outEvent
        );

        QueueHandle_t                   txQueue_;
        QueueHandle_t                   rxQueue_;
        Interfaces::ICommandDispatcher* dispatcher_ { nullptr };
        bool                            connected_  { false };
    };
}