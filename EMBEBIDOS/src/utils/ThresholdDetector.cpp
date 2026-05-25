#include "utils/ThresholdDetector.hpp"

namespace Utils
{
    ThresholdDetector::ThresholdDetector(
        float thresholdOn,
        float thresholdOff,
        uint16_t debounceMs
    )
        : onLevel_(thresholdOn),
          offLevel_(thresholdOff),
          debounceMs_(debounceMs)
    {
    }

    void ThresholdDetector::setThresholds(float onLevel, float offLevel)
    {
        if (offLevel >= onLevel)
        {
            // garantizar separación mínima 5%
            offLevel = onLevel * 0.6f;
        }
        onLevel_  = onLevel;
        offLevel_ = offLevel;
    }

    void ThresholdDetector::setDebounce(uint16_t debounceMs)
    {
        debounceMs_ = debounceMs;
    }

    void ThresholdDetector::reset()
    {
        candidate_ = false;
        active_    = false;
        candidateSince_ = 0;
    }

    bool ThresholdDetector::update(float value, uint32_t nowMs)
    {
        const bool instant =
            active_
                ? (value > offLevel_)     // ya activo: sólo apaga si baja de OFF
                : (value > onLevel_);     // inactivo: sólo enciende si sube de ON

        if (instant != candidate_)
        {
            candidate_      = instant;
            candidateSince_ = nowMs;
        }
        else if (candidate_ != active_)
        {
            // Estable y diferente al estado actual: ¿pasó debounce?
            if ((nowMs - candidateSince_) >= debounceMs_)
            {
                active_ = candidate_;
            }
        }

        return active_;
    }
}