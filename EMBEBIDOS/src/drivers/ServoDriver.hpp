#pragma once

#include "hal/PWMHal.hpp"

namespace Drivers
{
    class ServoDriver
    {
    public:
        explicit ServoDriver(HAL::PWMHal& pwmHal);

        bool attach(gpio_num_t pin, ledc_channel_t channel);

        bool setAngle(float angle);

        float lastAngle() const { return lastAngle_; }

        bool isAttached() const { return attached_; }

    private:
        HAL::PWMHal&    pwmHal_;
        ledc_channel_t  channel_;
        float           lastAngle_ { -1.0f };
        bool            attached_  { false };
    };
}