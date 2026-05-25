#pragma once

#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_continuous.h"

namespace HAL
{
    struct ADCContinuousConfig
    {
        adc_channel_t channel    { ADC_CHANNEL_0 };  // GPIO36 en ADC1
        adc_unit_t    unit       { ADC_UNIT_1 };
        adc_atten_t   attenuation{ ADC_ATTEN_DB_12 };
        uint32_t      sampleRateHz { 1000 };
        uint32_t      bufferBytes  { 1024 };          // tamaño del pool DMA
        uint32_t      frameBytes   { 256 };           // tamaño de cada lectura
    };

    class ADCHal
    {
    public:
        // ----- API ONESHOT (sin cambios) -----
        bool initialize();

        bool configureChannel(
            adc_channel_t channel
        );

        bool read(
            adc_channel_t channel,
            int& value
        );

        // ----- API CONTINUOUS (nueva) -----
        bool initializeContinuous(
            const ADCContinuousConfig& config
        );

        bool startContinuous();

        bool stopContinuous();

        // Lee hasta `maxBytes` bytes en `outBuffer`, devuelve cuántos
        // se leyeron en `outBytesRead`. Bloquea hasta `timeoutMs`.
        bool readContinuous(
            uint8_t* outBuffer,
            uint32_t maxBytes,
            uint32_t& outBytesRead,
            uint32_t timeoutMs
        );

        // Helper: decodifica un buffer crudo del DMA en muestras de 12 bits.
        // Devuelve cuántas muestras válidas se decodificaron.
        // El usuario debe pre-asignar `outSamples` con espacio suficiente.
        size_t decodeContinuousFrame(
            const uint8_t* buffer,
            uint32_t bytesRead,
            adc_channel_t expectedChannel,
            uint16_t* outSamples,
            size_t maxSamples
        ) const;

        bool isContinuousRunning() const { return continuousRunning_; }

    private:
        adc_oneshot_unit_handle_t adcHandle_ { nullptr };

        adc_continuous_handle_t continuousHandle_ { nullptr };
        bool continuousRunning_ { false };
    };
}