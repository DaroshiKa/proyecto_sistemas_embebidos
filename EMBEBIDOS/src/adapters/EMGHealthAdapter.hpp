#pragma once

#include "interfaces/ISensorHealthProvider.hpp"
#include "interfaces/IEMGSource.hpp"

namespace Adapters
{
    class EMGHealthAdapter final : public Interfaces::ISensorHealthProvider
    {
    public:
        explicit EMGHealthAdapter(
            Interfaces::IEMGSource* source
        );

        Interfaces::SensorKind kind() const override
        {
            return Interfaces::SensorKind::EMG;
        }

        bool isPresent() const override;
        bool isCalibrated() const override;
        uint32_t lastValidSampleMs() const override;
        bool hasFault() const override;

    private:
        Interfaces::IEMGSource* source_;
    };
}