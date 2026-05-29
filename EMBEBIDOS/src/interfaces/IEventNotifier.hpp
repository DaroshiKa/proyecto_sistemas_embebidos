#pragma once

#include "models/TelemetryFrame.hpp"

namespace Interfaces
{
    // Implementado por el TelemetryPublisher. Cualquier componente del firmware
    // puede usar esta interfaz para "anunciar" algo al display sin saber qué
    // display hay del otro lado.
    class IEventNotifier
    {
    public:
        virtual ~IEventNotifier() = default;

        virtual void notify(
            const Models::TelemetryFrame& frame
        ) = 0;
    };
}