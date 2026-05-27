#include "adapters/IMUHealthAdapter.hpp"

#include "models/IMUStatus.hpp"

namespace Adapters
{
    IMUHealthAdapter::IMUHealthAdapter(
        Interfaces::IIMUSource* source
    )
        : source_(source)
    {
    }

    bool IMUHealthAdapter::isPresent() const
    {
        if (source_ == nullptr) return false;

        const auto s = source_->getStatus();
        return s.state != Models::IMUState::UNINITIALIZED &&
               s.state != Models::IMUState::BUS_ERROR;
    }

    bool IMUHealthAdapter::isCalibrated() const
    {
        if (source_ == nullptr) return false;
        return source_->isCalibrated();
    }

    uint32_t IMUHealthAdapter::lastValidSampleMs() const
    {
        if (source_ == nullptr) return 0;
        return source_->getStatus().lastUpdateMs;
    }

    bool IMUHealthAdapter::hasFault() const
    {
        if (source_ == nullptr)
        {
            // No estar presente no es por sí solo "falta": el sistema
            // puede estar configurado intencionalmente sin IMU.
            return false;
        }

        const auto s = source_->getStatus();
        return s.state == Models::IMUState::TIMEOUT ||
               s.state == Models::IMUState::BUS_ERROR ||
               s.state == Models::IMUState::FAULT;
    }
}