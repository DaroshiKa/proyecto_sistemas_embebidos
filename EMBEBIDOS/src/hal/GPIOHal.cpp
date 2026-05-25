#include "hal/GPIOHal.hpp"

namespace HAL
{
    bool GPIOHal::configureOutput(
        gpio_num_t pin
    )
    {
        gpio_config_t config {};

        config.pin_bit_mask =
            (1ULL << pin);

        config.mode =
            GPIO_MODE_OUTPUT;

        config.pull_down_en =
            GPIO_PULLDOWN_DISABLE;

        config.pull_up_en =
            GPIO_PULLUP_DISABLE;

        config.intr_type =
            GPIO_INTR_DISABLE;

        return gpio_config(&config) == ESP_OK;
    }

    bool GPIOHal::configureInput(
        gpio_num_t pin,
        gpio_pull_mode_t pullMode
    )
    {
        gpio_config_t config {};

        config.pin_bit_mask =
            (1ULL << pin);

        config.mode =
            GPIO_MODE_INPUT;

        config.pull_up_en =
            (pullMode == GPIO_PULLUP_ONLY)
                ? GPIO_PULLUP_ENABLE
                : GPIO_PULLUP_DISABLE;

        config.pull_down_en =
            (pullMode == GPIO_PULLDOWN_ONLY)
                ? GPIO_PULLDOWN_ENABLE
                : GPIO_PULLDOWN_DISABLE;

        config.intr_type =
            GPIO_INTR_DISABLE;

        return gpio_config(&config) == ESP_OK;
    }

    void GPIOHal::setLevel(
        gpio_num_t pin,
        bool level
    )
    {
        gpio_set_level(pin, level);
    }

    bool GPIOHal::getLevel(
        gpio_num_t pin
    )
    {
        return gpio_get_level(pin);
    }
}