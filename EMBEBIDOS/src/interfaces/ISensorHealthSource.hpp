#pragma once

#include <cstdint>

namespace Interfaces
{
    class ISensorHealthSource
    {
    public:
        virtual ~ISensorHealthSource() = default;

        // Devuelve el timestamp (ms) de la última muestra válida producida
        // por el sensor. 0 si nunca produjo una.
        virtual uint32_t lastSampleTimestampMs() const = 0;

        // Indicador rápido: ¿el sensor cree estar OK?
        // SafetyService NO se fía sólo de esto; cruza con timeout real.
        virtual bool isSensorOk() const = 0;
    };
}