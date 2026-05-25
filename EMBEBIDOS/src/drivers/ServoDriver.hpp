#pragma once

#include "hal/PWMHal.hpp"

namespace Drivers
{
    class ServoDriver
    {
    public:
        explicit ServoDriver(
            HAL::PWMHal& pwmHal
        );

        bool attach(
            gpio_num_t pin,
            ledc_channel_t channel
        );

        bool setAngle(
            float angle
        );

    private:
        HAL::PWMHal& pwmHal_;

        ledc_channel_t channel_;
    };
}