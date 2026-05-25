#include "hal/ADCHal.hpp"

namespace HAL
{
    bool ADCHal::initialize()
    {
        adc_oneshot_unit_init_cfg_t config {};

        config.unit_id = ADC_UNIT_1;

        return adc_oneshot_new_unit(
            &config,
            &adcHandle_
        ) == ESP_OK;
    }

    bool ADCHal::configureChannel(
        adc_channel_t channel
    )
    {
        adc_oneshot_chan_cfg_t config {};

        config.bitwidth = ADC_BITWIDTH_12;

        config.atten = ADC_ATTEN_DB_12;

        return adc_oneshot_config_channel(
            adcHandle_,
            channel,
            &config
        ) == ESP_OK;
    }

    bool ADCHal::read(
        adc_channel_t channel,
        int& value
    )
    {
        return adc_oneshot_read(
            adcHandle_,
            channel,
            &value
        ) == ESP_OK;
    }
}