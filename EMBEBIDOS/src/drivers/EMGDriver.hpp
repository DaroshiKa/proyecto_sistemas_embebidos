#pragma once

#include <cstdint>
#include <cstddef>

#include "hal/ADCHal.hpp"
#include "models/EMGConfig.hpp"

namespace Drivers
{
    class EMGDriver
    {
    public:
        explicit EMGDriver(HAL::ADCHal& adcHal);

        bool initialize(const Models::EMGConfig& config);

        bool start();
        bool stop();

        // Lee del DMA y decodifica muestras 12 bit. Devuelve cuántas muestras válidas.
        // `outSamples` debe tener al menos `maxSamples` espacio.
        size_t readSamples(
            uint16_t* outSamples,
            size_t maxSamples,
            uint32_t timeoutMs
        );

        // Calibración bloqueante: aprende baseline (media) y peak (max-min/2)
        // mientras el músculo está en reposo. Llamar con el usuario relajado.
        bool calibrate(
            uint16_t samples,
            float& outBaselineNormalized,
            float& outPeakNormalized
        );

        bool isCalibrated() const { return calibrated_; }

    private:
        HAL::ADCHal& adcHal_;
        Models::EMGConfig config_ {};
        bool initialized_ { false };
        bool calibrated_  { false };
    };
}