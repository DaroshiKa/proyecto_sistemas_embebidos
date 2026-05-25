#pragma once

#include "driver/ledc.h"

namespace HAL
{
    class PWMHal
    {
    public:
        bool initialize();

        bool configureChannel(
            gpio_num_t pin,
            ledc_channel_t channel
        );

        bool setDuty(
            ledc_channel_t channel,
            uint32_t duty
        );
    };
}