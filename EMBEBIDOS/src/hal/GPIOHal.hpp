#pragma once

#include "driver/gpio.h"

namespace HAL
{
    class GPIOHal
    {
    public:
        static bool configureOutput(
            gpio_num_t pin
        );

        static bool configureInput(
            gpio_num_t pin,
            gpio_pull_mode_t pullMode
        );

        static void setLevel(
            gpio_num_t pin,
            bool level
        );

        static bool getLevel(
            gpio_num_t pin
        );
    };
}