#pragma once

#include <cstddef>
#include <cstdint>

namespace Utils
{
    class MovingAverage
    {
    public:
        static constexpr size_t MAX_WINDOW = 256;

        explicit MovingAverage(size_t window = 50);

        void   setWindow(size_t window);
        void   reset();
        float  process(float x);
        float  current() const { return (count_ > 0) ? sum_ / static_cast<float>(count_) : 0.0f; }

    private:
        float   buffer_[MAX_WINDOW] {};
        size_t  window_ { 50 };
        size_t  index_  { 0 };
        size_t  count_  { 0 };
        float   sum_    { 0.0f };
    };
}