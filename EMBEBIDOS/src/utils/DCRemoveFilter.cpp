#include "utils/DCRemoveFilter.hpp"

namespace Utils
{
    DCRemoveFilter::DCRemoveFilter(float alpha)
        : alpha_(alpha)
    {
    }

    void DCRemoveFilter::setAlpha(float alpha)
    {
        if (alpha < 0.0f) alpha = 0.0f;
        if (alpha > 1.0f) alpha = 1.0f;
        alpha_ = alpha;
    }

    void DCRemoveFilter::reset()
    {
        prevX_ = 0.0f;
        prevY_ = 0.0f;
    }

    float DCRemoveFilter::process(float x)
    {
        const float y = alpha_ * (prevY_ + x - prevX_);
        prevX_ = x;
        prevY_ = y;
        return y;
    }
}