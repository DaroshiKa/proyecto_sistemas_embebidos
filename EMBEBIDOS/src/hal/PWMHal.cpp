#include "hal/PWMHal.hpp"

namespace HAL
{
    bool PWMHal::initialize()
    {
        ledc_timer_config_t timerConfig {};

        timerConfig.speed_mode =
            LEDC_LOW_SPEED_MODE;

        timerConfig.timer_num =
            LEDC_TIMER_0;

        timerConfig.freq_hz = 50;

        timerConfig.duty_resolution =
            LEDC_TIMER_16_BIT;

        timerConfig.clk_cfg =
            LEDC_AUTO_CLK;

        return ledc_timer_config(
            &timerConfig
        ) == ESP_OK;
    }

    bool PWMHal::configureChannel(
        gpio_num_t pin,
        ledc_channel_t channel
    )
    {
        ledc_channel_config_t config {};

        config.gpio_num = pin;

        config.speed_mode =
            LEDC_LOW_SPEED_MODE;

        config.channel = channel;

        config.timer_sel =
            LEDC_TIMER_0;

        config.duty = 0;

        config.hpoint = 0;

        return ledc_channel_config(
            &config
        ) == ESP_OK;
    }

    bool PWMHal::setDuty(
        ledc_channel_t channel,
        uint32_t duty
    )
    {
        if (ledc_set_duty(
                LEDC_LOW_SPEED_MODE,
                channel,
                duty
            ) != ESP_OK)
        {
            return false;
        }

        return ledc_update_duty(
            LEDC_LOW_SPEED_MODE,
            channel
        ) == ESP_OK;
    }
}