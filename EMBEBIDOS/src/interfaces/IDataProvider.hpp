#pragma once

#include "models/SensorData.hpp"
#include "models/ServoState.hpp"
#include "models/IMUStatus.hpp"
#include "models/EMGStatus.hpp"

namespace Interfaces
{
    // Cuando el display pregunta "dame el estado actual", alguien tiene
    // que respondérselo. Esta interfaz es el "agregador" que el
    // NextionInterface consulta para armar TelemetryFrames.
    class IDataProvider
    {
    public:
        virtual ~IDataProvider() = default;

        virtual bool getIMUData(Models::IMUData& out) const = 0;
        virtual bool getEMGData(Models::EMGData& out) const = 0;
        virtual bool getServoState(
            Models::JointId id,
            Models::ServoState& out
        ) const = 0;

        virtual Models::IMUStatus getIMUStatus() const = 0;
        virtual Models::EMGStatus getEMGStatus() const = 0;

        virtual uint32_t getSystemUptimeMs() const = 0;
        virtual uint32_t getFreeHeap() const = 0;
    };
}