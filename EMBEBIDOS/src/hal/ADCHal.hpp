#pragma once

#include "esp_adc/adc_oneshot.h"

namespace HAL
{
    class ADCHal
    {
    public:
        bool initialize();

        bool configureChannel(
            adc_channel_t channel
        );

        bool read(
            adc_channel_t channel,
            int& value
        );

    private:
        adc_oneshot_unit_handle_t adcHandle_ { nullptr };
    };
}