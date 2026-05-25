#pragma once

#include "models/SensorData.hpp"
#include "models/IMUStatus.hpp"

namespace Interfaces
{
    class IIMUSource
    {
    public:
        virtual ~IIMUSource() = default;

        virtual bool getLatestData(
            Models::IMUData& outData
        ) const = 0;

        virtual Models::IMUStatus getStatus() const = 0;

        virtual bool startCalibration() = 0;

        virtual bool isCalibrated() const = 0;
    };
}