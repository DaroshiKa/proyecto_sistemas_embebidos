#include "core/SystemDataProvider.hpp"

#include "esp_system.h"
#include "esp_timer.h"

namespace Core
{
    SystemDataProvider::SystemDataProvider(
        Interfaces::IIMUSource* imu,
        Interfaces::IEMGSource* emg,
        Interfaces::IMotionExecutor* executor
    )
        : imu_(imu), emg_(emg), executor_(executor)
    {
    }

    bool SystemDataProvider::getIMUData(Models::IMUData& out) const
    {
        if (imu_ == nullptr) return false;
        return imu_->getLatestData(out);
    }

    bool SystemDataProvider::getEMGData(Models::EMGData& out) const
    {
        if (emg_ == nullptr) return false;
        return emg_->getLatestData(out);
    }

    bool SystemDataProvider::getServoState(
        Models::JointId id,
        Models::ServoState& out
    ) const
    {
        if (executor_ == nullptr) return false;
        return executor_->getServoState(id, out);
    }

    Models::IMUStatus SystemDataProvider::getIMUStatus() const
    {
        if (imu_ == nullptr) return Models::IMUStatus {};
        return imu_->getStatus();
    }

    Models::EMGStatus SystemDataProvider::getEMGStatus() const
    {
        if (emg_ == nullptr) return Models::EMGStatus {};
        return emg_->getStatus();
    }

    uint32_t SystemDataProvider::getSystemUptimeMs() const
    {
        return static_cast<uint32_t>(esp_timer_get_time() / 1000LL);
    }

    uint32_t SystemDataProvider::getFreeHeap() const
    {
        return static_cast<uint32_t>(esp_get_free_heap_size());
    }
}