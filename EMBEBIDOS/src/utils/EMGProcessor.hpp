#pragma once

#include "utils/BiquadFilter.hpp"
#include "utils/DCRemoveFilter.hpp"
#include "utils/MovingAverage.hpp"

#include "models/EMGConfig.hpp"

namespace Utils
{
    struct EMGProcessedSample
    {
        float raw       { 0.0f };  // entrada normalizada [-1..1]
        float bandPassed{ 0.0f };  // tras notch+HPF+LPF
        float envelope  { 0.0f };  // tras rectificación + LPF
        float smoothed  { 0.0f };  // tras media móvil
    };

    class EMGProcessor
    {
    public:
        EMGProcessor();

        // Configura todos los filtros con base en `cfg`. Llamar tras cambios de config.
        void configure(const Models::EMGConfig& cfg);

        void reset();

        // Procesa una muestra cruda (entera) del ADC. La normaliza internamente.
        EMGProcessedSample process(
            uint16_t rawAdc12Bit
        );

        // Ajusta el baseline aprendido en calibración. Se sustrae antes de los filtros.
        void setBaseline(float baselineNormalized);

        // Ajusta el factor de normalización del envelope (peak detectado en calibración).
        void setPeakNormalization(float peak);

    private:
        BiquadFilter    notch_;
        BiquadFilter    hpf_;
        BiquadFilter    lpf_;
        BiquadFilter    envelopeLpf_;
        DCRemoveFilter  dcBlocker_;
        MovingAverage   movingAvg_;

        float           baseline_ { 0.5f };  // centro nominal del ADC (1.65V → 0.5)
        float           peakNorm_ { 0.5f };  // 0.5 = sin normalización, se actualiza en calibración
    };
}