#include "utils/EMGProcessor.hpp"

#include <cmath>

namespace Utils
{
    static constexpr float ADC_MAX_F = 4095.0f;

    EMGProcessor::EMGProcessor() = default;

    void EMGProcessor::configure(const Models::EMGConfig& cfg)
    {
        const float fs = static_cast<float>(cfg.sampleRateHz);

        notch_.designNotch(cfg.notchHz, fs, cfg.notchQ);
        hpf_.designHighPass(cfg.hpfCutoffHz, fs);
        lpf_.designLowPass(cfg.lpfCutoffHz, fs);
        envelopeLpf_.designLowPass(cfg.envelopeCutoffHz, fs);

        movingAvg_.setWindow(cfg.movingAvgWindow);

        reset();
    }

    void EMGProcessor::reset()
    {
        notch_.reset();
        hpf_.reset();
        lpf_.reset();
        envelopeLpf_.reset();
        dcBlocker_.reset();
        movingAvg_.reset();
    }

    void EMGProcessor::setBaseline(float baselineNormalized)
    {
        if (baselineNormalized < 0.0f) baselineNormalized = 0.0f;
        if (baselineNormalized > 1.0f) baselineNormalized = 1.0f;
        baseline_ = baselineNormalized;
    }

    void EMGProcessor::setPeakNormalization(float peak)
    {
        if (peak < 0.01f) peak = 0.01f;  // evita división por 0
        peakNorm_ = peak;
    }

    EMGProcessedSample EMGProcessor::process(uint16_t rawAdc12Bit)
    {
        EMGProcessedSample out {};

        // 1) Normalizar [0..4095] → [0..1]
        const float xNorm =
            static_cast<float>(rawAdc12Bit) / ADC_MAX_F;

        out.raw = xNorm;

        // 2) Centrar restando baseline aprendido (queda en ~ [-0.5..0.5])
        const float centered = xNorm - baseline_;

        // 3) DC blocker para eliminar deriva lenta de electrodos
        const float dcBlocked = dcBlocker_.process(centered);

        // 4) Notch para 50/60 Hz
        const float notched = notch_.process(dcBlocked);

        // 5) Band-pass como HPF + LPF en cascada
        const float highPassed = hpf_.process(notched);
        const float bandPassed = lpf_.process(highPassed);

        out.bandPassed = bandPassed;

        // 6) Rectificación de onda completa
        const float rectified = fabsf(bandPassed);

        // 7) Envolvente (LPF lento)
        const float envelope = envelopeLpf_.process(rectified);

        // 8) Normalización por peak detectado en calibración
        float envelopeNorm = envelope / peakNorm_;
        if (envelopeNorm < 0.0f) envelopeNorm = 0.0f;
        if (envelopeNorm > 1.0f) envelopeNorm = 1.0f;

        out.envelope = envelopeNorm;

        // 9) Media móvil para suavizar
        out.smoothed = movingAvg_.process(envelopeNorm);

        return out;
    }
}