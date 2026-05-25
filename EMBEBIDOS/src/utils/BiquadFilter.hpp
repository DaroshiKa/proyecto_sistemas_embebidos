#pragma once

namespace Utils
{
    class BiquadFilter
    {
    public:
        BiquadFilter();

        // Coeficientes normalizados (a0=1)
        void setCoefficients(
            float b0,
            float b1,
            float b2,
            float a1,
            float a2
        );

        // Diseños analíticos (RBJ Audio EQ Cookbook)
        void designLowPass(
            float cutoffHz,
            float sampleRateHz,
            float q = 0.707f
        );

        void designHighPass(
            float cutoffHz,
            float sampleRateHz,
            float q = 0.707f
        );

        void designNotch(
            float centerHz,
            float sampleRateHz,
            float q = 30.0f
        );

        void reset();

        float process(float x);

    private:
        float b0_ { 1.0f };
        float b1_ { 0.0f };
        float b2_ { 0.0f };
        float a1_ { 0.0f };
        float a2_ { 0.0f };

        // Estados internos (transposed direct form II)
        float z1_ { 0.0f };
        float z2_ { 0.0f };
    };
}