#include "drivers/EMGDriver.hpp"

#include <cstring>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

namespace Drivers
{
    static constexpr const char* TAG = "EMGDriver";
    static constexpr float ADC_MAX_F = 4095.0f;

    EMGDriver::EMGDriver(HAL::ADCHal& adcHal)
        : adcHal_(adcHal)
    {
    }

    bool EMGDriver::initialize(const Models::EMGConfig& config)
    {
        config_ = config;

        HAL::ADCContinuousConfig hwCfg {};
        hwCfg.channel       = config_.adcChannel;
        hwCfg.unit          = config_.adcUnit;
        hwCfg.attenuation   = config_.adcAttenuation;
        hwCfg.sampleRateHz  = config_.sampleRateHz;
        hwCfg.bufferBytes   = config_.dmaBufferBytes;
        hwCfg.frameBytes    = config_.dmaFrameBytes;

        if (!adcHal_.initializeContinuous(hwCfg))
        {
            ESP_LOGE(TAG, "Continuous ADC init failed");
            return false;
        }

        initialized_ = true;
        return true;
    }

    bool EMGDriver::start()
    {
        if (!initialized_) return false;
        return adcHal_.startContinuous();
    }

    bool EMGDriver::stop()
    {
        if (!initialized_) return false;
        return adcHal_.stopContinuous();
    }

    size_t EMGDriver::readSamples(
        uint16_t* outSamples,
        size_t maxSamples,
        uint32_t timeoutMs
    )
    {
        if (!initialized_ || outSamples == nullptr || maxSamples == 0)
        {
            return 0;
        }

        uint8_t dmaBuffer[256];

        uint32_t bytesRead = 0;

        if (!adcHal_.readContinuous(
                dmaBuffer,
                sizeof(dmaBuffer),
                bytesRead,
                timeoutMs))
        {
            return 0;
        }

        return adcHal_.decodeContinuousFrame(
            dmaBuffer,
            bytesRead,
            config_.adcChannel,
            outSamples,
            maxSamples
        );
    }

    bool EMGDriver::calibrate(
        uint16_t samples,
        float& outBaselineNormalized,
        float& outPeakNormalized
    )
    {
        if (!initialized_ || samples == 0)
        {
            return false;
        }

        // Asegurar que el ADC está corriendo durante la calibración
        const bool wasRunning = adcHal_.isContinuousRunning();
        if (!wasRunning)
        {
            if (!adcHal_.startContinuous())
            {
                return false;
            }
        }

        ESP_LOGI(TAG, "EMG calibration: collecting %u samples...", samples);

        double sumNormalized = 0.0;
        float  minVal = 1.0f;
        float  maxVal = 0.0f;

        uint16_t collected = 0;

        const TickType_t startTick = xTaskGetTickCount();
        const TickType_t maxTicks  = pdMS_TO_TICKS(20000);  // hard timeout 20 s

        uint16_t batch[64];

        while (collected < samples &&
               (xTaskGetTickCount() - startTick) < maxTicks)
        {
            const size_t got =
                readSamples(batch, sizeof(batch) / sizeof(batch[0]), 100);

            for (size_t i = 0; i < got && collected < samples; ++i)
            {
                const float n =
                    static_cast<float>(batch[i]) / ADC_MAX_F;

                sumNormalized += n;
                if (n < minVal) minVal = n;
                if (n > maxVal) maxVal = n;

                ++collected;
            }
        }

        if (!wasRunning)
        {
            adcHal_.stopContinuous();
        }

        if (collected < samples / 2)
        {
            ESP_LOGE(
                TAG,
                "Calibration failed: %u/%u samples",
                collected,
                samples
            );
            return false;
        }

        outBaselineNormalized =
            static_cast<float>(sumNormalized / collected);

        // Peak = mitad del rango pico-a-pico observado en reposo.
        // Este valor es nuestra estimación de "ruido en reposo" y se usa
        // como factor de normalización: contracción real producirá envolventes
        // varias veces mayor que esto.
        outPeakNormalized = (maxVal - minVal) * 0.5f;
        if (outPeakNormalized < 0.01f) outPeakNormalized = 0.01f;

        // Escalamos a un peak "esperado en contracción": el ruido en reposo es
        // ~10x menor que una contracción real, así que multiplicamos por 10
        // para normalizar contracciones plenas a ~1.0.
        outPeakNormalized *= 10.0f;

        calibrated_ = true;

        ESP_LOGI(
            TAG,
            "Calibration OK: baseline=%.4f peak=%.4f (rest p-p=%.4f)",
            outBaselineNormalized,
            outPeakNormalized,
            maxVal - minVal
        );

        return true;
    }
}