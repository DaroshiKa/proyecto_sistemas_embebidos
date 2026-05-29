#pragma once

#include "interfaces/IDataProvider.hpp"
#include "interfaces/IIMUSource.hpp"
#include "interfaces/IEMGSource.hpp"
#include "interfaces/IMotionExecutor.hpp"

namespace Core
{
    class SystemDataProvider final :
        public Interfaces::IDataProvider
    {
    public:
        SystemDataProvider(
            Interfaces::IIMUSource* imu,
            Interfaces::IEMGSource* emg,
            Interfaces::IMotionExecutor* executor
        );

        bool getIMUData(Models::IMUData& out) const override;
        bool getEMGData(Models::EMGData& out) const override;
        bool getServoState(
            Models::JointId id,
            Models::ServoState& out
        ) const override;

        Models::IMUStatus getIMUStatus() const override;
        Models::EMGStatus getEMGStatus() const override;

        uint32_t getSystemUptimeMs() const override;
        uint32_t getFreeHeap() const override;

    private:
        Interfaces::IIMUSource*       imu_;
        Interfaces::IEMGSource*       emg_;
        Interfaces::IMotionExecutor*  executor_;
    };
}