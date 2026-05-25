#include "hal/ADCHal.hpp"

#include <cstring>

#include "esp_log.h"

namespace HAL
{
    static constexpr const char* TAG = "ADCHal";

    // ----- ONESHOT -----

    bool ADCHal::initialize()
    {
        if (adcHandle_ != nullptr)
        {
            return true;
        }

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
        if (adcHandle_ == nullptr)
        {
            return false;
        }

        adc_oneshot_chan_cfg_t config {};
        config.bitwidth = ADC_BITWIDTH_12;
        config.atten    = ADC_ATTEN_DB_12;

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
        if (adcHandle_ == nullptr)
        {
            return false;
        }

        return adc_oneshot_read(
            adcHandle_,
            channel,
            &value
        ) == ESP_OK;
    }

    // ----- CONTINUOUS -----

    bool ADCHal::initializeContinuous(
        const ADCContinuousConfig& config
    )
    {
        if (continuousHandle_ != nullptr)
        {
            return true;
        }

        adc_continuous_handle_cfg_t handleCfg {};
        handleCfg.max_store_buf_size = config.bufferBytes;
        handleCfg.conv_frame_size    = config.frameBytes;

        if (adc_continuous_new_handle(
                &handleCfg,
                &continuousHandle_
            ) != ESP_OK)
        {
            ESP_LOGE(TAG, "adc_continuous_new_handle failed");
            return false;
        }

        adc_continuous_config_t dmaCfg {};
        dmaCfg.sample_freq_hz = config.sampleRateHz;
        dmaCfg.conv_mode      = ADC_CONV_SINGLE_UNIT_1;
        dmaCfg.format         = ADC_DIGI_OUTPUT_FORMAT_TYPE1;

        adc_digi_pattern_config_t pattern {};
        pattern.atten      = config.attenuation;
        pattern.channel    = config.channel & 0x7;
        pattern.unit       = config.unit;
        pattern.bit_width  = ADC_BITWIDTH_12;

        dmaCfg.pattern_num = 1;
        dmaCfg.adc_pattern = &pattern;

        if (adc_continuous_config(
                continuousHandle_,
                &dmaCfg
            ) != ESP_OK)
        {
            ESP_LOGE(TAG, "adc_continuous_config failed");
            adc_continuous_deinit(continuousHandle_);
            continuousHandle_ = nullptr;
            return false;
        }

        ESP_LOGI(
            TAG,
            "Continuous ADC initialized @ %lu Hz on ch %d",
            static_cast<unsigned long>(config.sampleRateHz),
            static_cast<int>(config.channel)
        );

        return true;
    }

    bool ADCHal::startContinuous()
    {
        if (continuousHandle_ == nullptr)
        {
            return false;
        }

        if (continuousRunning_)
        {
            return true;
        }

        if (adc_continuous_start(continuousHandle_) != ESP_OK)
        {
            return false;
        }

        continuousRunning_ = true;
        return true;
    }

    bool ADCHal::stopContinuous()
    {
        if (continuousHandle_ == nullptr || !continuousRunning_)
        {
            return false;
        }

        if (adc_continuous_stop(continuousHandle_) != ESP_OK)
        {
            return false;
        }

        continuousRunning_ = false;
        return true;
    }

    bool ADCHal::readContinuous(
        uint8_t* outBuffer,
        uint32_t maxBytes,
        uint32_t& outBytesRead,
        uint32_t timeoutMs
    )
    {
        outBytesRead = 0;

        if (continuousHandle_ == nullptr || outBuffer == nullptr || maxBytes == 0)
        {
            return false;
        }

        const esp_err_t err = adc_continuous_read(
            continuousHandle_,
            outBuffer,
            maxBytes,
            &outBytesRead,
            timeoutMs
        );

        // ESP_ERR_TIMEOUT no es fallo: indica que no hubo datos a tiempo.
        if (err == ESP_OK || err == ESP_ERR_TIMEOUT)
        {
            return outBytesRead > 0;
        }

        return false;
    }

    size_t ADCHal::decodeContinuousFrame(
        const uint8_t* buffer,
        uint32_t bytesRead,
        adc_channel_t expectedChannel,
        uint16_t* outSamples,
        size_t maxSamples
    ) const
    {
        if (buffer == nullptr || outSamples == nullptr || maxSamples == 0)
        {
            return 0;
        }

        size_t count = 0;

        for (uint32_t i = 0;
             i + SOC_ADC_DIGI_RESULT_BYTES <= bytesRead &&
             count < maxSamples;
             i += SOC_ADC_DIGI_RESULT_BYTES)
        {
            const auto* frame =
                reinterpret_cast<const adc_digi_output_data_t*>(&buffer[i]);

            // Formato TYPE1: type1.channel, type1.data
            if (frame->type1.channel == (expectedChannel & 0x7))
            {
                outSamples[count++] =
                    static_cast<uint16_t>(frame->type1.data & 0x0FFF);
            }
        }

        return count;
    }
}