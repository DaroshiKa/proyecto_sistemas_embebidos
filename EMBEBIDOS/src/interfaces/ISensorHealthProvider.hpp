#pragma once

#include <cstdint>

namespace Interfaces
{
    enum class SensorKind : uint8_t
    {
        IMU,
        EMG,
        FORCE,
        TEMPERATURE
        // extensible sin tocar SafetyService
    };

    class ISensorHealthProvider
    {
    public:
        virtual ~ISensorHealthProvider() = default;

        virtual SensorKind kind() const = 0;

        // ¿El sensor está conectado/respondiendo?
        virtual bool isPresent() const = 0;

        // ¿Tuvo calibración satisfactoria?
        virtual bool isCalibrated() const = 0;

        // Timestamp ms (epoch sistema) de la última muestra válida.
        // 0 si nunca ha producido datos.
        virtual uint32_t lastValidSampleMs() const = 0;

        // ¿Está en un estado de error explícito?
        virtual bool hasFault() const = 0;
    };
}