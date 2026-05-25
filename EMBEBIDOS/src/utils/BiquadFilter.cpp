#include "utils/BiquadFilter.hpp"

#include <cmath>

namespace Utils
{
    static constexpr float PI_F = 3.14159265358979323846f;

    BiquadFilter::BiquadFilter() = default;

    void BiquadFilter::setCoefficients(
        float b0,
        float b1,
        float b2,
        float a1,
        float a2
    )
    {
        b0_ = b0;
        b1_ = b1;
        b2_ = b2;
        a1_ = a1;
        a2_ = a2;
    }

    void BiquadFilter::reset()
    {
        z1_ = 0.0f;
        z2_ = 0.0f;
    }

    void BiquadFilter::designLowPass(
        float cutoffHz,
        float sampleRateHz,
        float q
    )
    {
        const float w0    = 2.0f * PI_F * cutoffHz / sampleRateHz;
        const float cosw0 = cosf(w0);
        const float sinw0 = sinf(w0);
        const float alpha = sinw0 / (2.0f * q);

        const float a0 = 1.0f + alpha;

        const float b0 = ((1.0f - cosw0) / 2.0f) / a0;
        const float b1 = (1.0f - cosw0) / a0;
        const float b2 = b0;
        const float a1 = (-2.0f * cosw0) / a0;
        const float a2 = (1.0f - alpha) / a0;

        setCoefficients(b0, b1, b2, a1, a2);
    }

    void BiquadFilter::designHighPass(
        float cutoffHz,
        float sampleRateHz,
        float q
    )
    {
        const float w0    = 2.0f * PI_F * cutoffHz / sampleRateHz;
        const float cosw0 = cosf(w0);
        const float sinw0 = sinf(w0);
        const float alpha = sinw0 / (2.0f * q);

        const float a0 = 1.0f + alpha;

        const float b0 = ((1.0f + cosw0) / 2.0f) / a0;
        const float b1 = -(1.0f + cosw0) / a0;
        const float b2 = b0;
        const float a1 = (-2.0f * cosw0) / a0;
        const float a2 = (1.0f - alpha) / a0;

        setCoefficients(b0, b1, b2, a1, a2);
    }

    void BiquadFilter::designNotch(
        float centerHz,
        float sampleRateHz,
        float q
    )
    {
        const float w0    = 2.0f * PI_F * centerHz / sampleRateHz;
        const float cosw0 = cosf(w0);
        const float sinw0 = sinf(w0);
        const float alpha = sinw0 / (2.0f * q);

        const float a0 = 1.0f + alpha;

        const float b0 = 1.0f / a0;
        const float b1 = (-2.0f * cosw0) / a0;
        const float b2 = b0;
        const float a1 = (-2.0f * cosw0) / a0;
        const float a2 = (1.0f - alpha) / a0;

        setCoefficients(b0, b1, b2, a1, a2);
    }

    float BiquadFilter::process(float x)
    {
        // Transposed Direct Form II
        const float y = b0_ * x + z1_;

        z1_ = b1_ * x - a1_ * y + z2_;
        z2_ = b2_ * x - a2_ * y;

        return y;
    }
}