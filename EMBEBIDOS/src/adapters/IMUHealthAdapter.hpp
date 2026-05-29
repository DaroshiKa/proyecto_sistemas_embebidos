#pragma once

#include "interfaces/ISensorHealthProvider.hpp"
#include "interfaces/IIMUSource.hpp"

namespace Adapters
{
    class IMUHealthAdapter final : public Interfaces::ISensorHealthProvider
    {
    public:
        // El adapter puede recibir nullptr: el sensor "no existe" en el
        // sistema. Esto permite el escenario "solo quiero probar servos".
        explicit IMUHealthAdapter(
            Interfaces::IIMUSource* source
        );

        Interfaces::SensorKind kind() const override
        {
            return Interfaces::SensorKind::IMU;
        }

        bool isPresent() const override;
        bool isCalibrated() const override;
        uint32_t lastValidSampleMs() const override;
        bool hasFault() const override;

    private:
        Interfaces::IIMUSource* source_;
    };
}