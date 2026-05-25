#include "utils/JointInterpolator.hpp"

#include <cmath>

namespace Utils
{
    JointInterpolator::JointInterpolator() = default;

    void JointInterpolator::planMove(
        float startAngle,
        float targetAngle,
        float speedDps,
        uint32_t nowMs,
        Models::ServoMotionProfile profile
    )
    {
        if (speedDps < 1.0f) speedDps = 1.0f;

        const float distance = fabsf(targetAngle - startAngle);

        // duración ≈ distancia / velocidad
        const uint32_t duration =
            static_cast<uint32_t>((distance / speedDps) * 1000.0f);

        planMoveWithDuration(
            startAngle, targetAngle,
            (duration < 1) ? 1 : duration,
            nowMs, profile
        );
    }

    void JointInterpolator::planMoveWithDuration(
        float startAngle,
        float targetAngle,
        uint32_t durationMs,
        uint32_t nowMs,
        Models::ServoMotionProfile profile
    )
    {
        startAngle_  = startAngle;
        targetAngle_ = targetAngle;
        startMs_     = nowMs;
        durationMs_  = (durationMs < 1) ? 1 : durationMs;
        profile_     = profile;
        active_      = true;
    }

    float JointInterpolator::sample(uint32_t nowMs, bool& outDone) const
    {
        outDone = false;

        if (!active_)
        {
            outDone = true;
            return targetAngle_;
        }

        const uint32_t elapsed = nowMs - startMs_;

        if (elapsed >= durationMs_)
        {
            outDone = true;
            return targetAngle_;
        }

        float t =
            static_cast<float>(elapsed) /
            static_cast<float>(durationMs_);

        if (t < 0.0f) t = 0.0f;
        if (t > 1.0f) t = 1.0f;

        // Aplicar perfil
        float k = t;
        if (profile_ == Models::ServoMotionProfile::SMOOTHSTEP)
        {
            // Hermite: 3t² - 2t³
            k = t * t * (3.0f - 2.0f * t);
        }

        return startAngle_ + (targetAngle_ - startAngle_) * k;
    }

    void JointInterpolator::abort(float currentAngle)
    {
        active_ = false;
        targetAngle_ = currentAngle;
        startAngle_  = currentAngle;
        durationMs_  = 0;
    }
}