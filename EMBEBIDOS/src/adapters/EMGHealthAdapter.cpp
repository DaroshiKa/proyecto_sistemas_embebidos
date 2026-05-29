#include "adapters/EMGHealthAdapter.hpp"

#include "models/EMGStatus.hpp"

namespace Adapters
{
    EMGHealthAdapter::EMGHealthAdapter(
        Interfaces::IEMGSource* source
    )
        : source_(source)
    {
    }

    bool EMGHealthAdapter::isPresent() const
    {
        if (source_ == nullptr) return false;

        const auto s = source_->getStatus();
        return s.state != Models::EMGState::UNINITIALIZED;
    }

    bool EMGHealthAdapter::isCalibrated() const
    {
        if (source_ == nullptr) return false;
        return source_->isCalibrated();
    }

    uint32_t EMGHealthAdapter::lastValidSampleMs() const
    {
        if (source_ == nullptr) return 0;
        return source_->getStatus().lastUpdateMs;
    }

    bool EMGHealthAdapter::hasFault() const
    {
        if (source_ == nullptr) return false;

        const auto s = source_->getStatus();
        return s.state == Models::EMGState::TIMEOUT ||
               s.state == Models::EMGState::FAULT;
    }
}