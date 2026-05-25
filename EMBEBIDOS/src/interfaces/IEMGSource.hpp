#pragma once

#include "models/SensorData.hpp"
#include "models/EMGStatus.hpp"

namespace Interfaces
{
    class IEMGSource
    {
    public:
        virtual ~IEMGSource() = default;

        virtual bool getLatestData(
            Models::EMGData& outData
        ) const = 0;

        virtual Models::EMGStatus getStatus() const = 0;

        virtual bool startCalibration() = 0;

        virtual bool isCalibrated() const = 0;
    };
}