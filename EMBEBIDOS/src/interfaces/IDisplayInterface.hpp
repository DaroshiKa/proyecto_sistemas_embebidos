#pragma once

#include "models/DisplayEvent.hpp"
#include "models/TelemetryFrame.hpp"

namespace Interfaces
{
    // Contrato genérico de cualquier display (Nextion, BLE App, Web UI...).
    class IDisplayInterface
    {
    public:
        virtual ~IDisplayInterface() = default;

        virtual bool initialize() = 0;

        virtual bool isConnected() const = 0;

        // Envía un frame de telemetría al display (no bloqueante).
        virtual bool sendTelemetry(
            const Models::TelemetryFrame& frame
        ) = 0;

        // Drena eventos recibidos del display (no bloqueante).
        // Devuelve cuántos eventos quedaron pendientes después de esta llamada.
        virtual bool getNextEvent(
            Models::DisplayEvent& outEvent
        ) = 0;
    };
}