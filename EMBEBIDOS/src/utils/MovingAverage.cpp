#include "utils/MovingAverage.hpp"

namespace Utils
{
    MovingAverage::MovingAverage(size_t window)
    {
        setWindow(window);
    }

    void MovingAverage::setWindow(size_t window)
    {
        if (window == 0) window = 1;
        if (window > MAX_WINDOW) window = MAX_WINDOW;

        window_ = window;
        reset();
    }

    void MovingAverage::reset()
    {
        for (size_t i = 0; i < MAX_WINDOW; ++i)
        {
            buffer_[i] = 0.0f;
        }
        index_ = 0;
        count_ = 0;
        sum_   = 0.0f;
    }

    float MovingAverage::process(float x)
    {
        sum_ -= buffer_[index_];
        buffer_[index_] = x;
        sum_ += x;

        index_ = (index_ + 1) % window_;

        if (count_ < window_)
        {
            ++count_;
        }

        return sum_ / static_cast<float>(count_);
    }
}