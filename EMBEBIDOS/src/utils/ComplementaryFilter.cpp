#include "utils/ComplementaryFilter.hpp"

#include <cmath>

namespace Utils
{
    static constexpr float RAD_TO_DEG = 57.2957795131f;

    ComplementaryFilter::ComplementaryFilter(
        float alpha
    )
        : alpha_(alpha)
    {
    }

    void ComplementaryFilter::reset(
        float pitch,
        float roll,
        float yaw
    )
    {
        pitch_ = pitch;
        roll_  = roll;
        yaw_   = yaw;
    }

    void ComplementaryFilter::setAlpha(
        float alpha
    )
    {
        if (alpha < 0.0f) alpha = 0.0f;
        if (alpha > 1.0f) alpha = 1.0f;

        alpha_ = alpha;
    }

    void ComplementaryFilter::update(
        float ax,
        float ay,
        float az,
        float gx,
        float gy,
        float gz,
        float dtSeconds
    )
    {
        // Estimación de ángulos a partir del acelerómetro
        const float accelMagSq =
            ax * ax + az * az;

        float pitchAcc = 0.0f;
        float rollAcc  = 0.0f;

        if (accelMagSq > 1e-6f)
        {
            pitchAcc =
                atan2f(ay, sqrtf(accelMagSq)) * RAD_TO_DEG;

            rollAcc =
                atan2f(-ax, az) * RAD_TO_DEG;
        }

        // Integración del giroscopio + fusión complementaria
        pitch_ =
            alpha_ * (pitch_ + gy * dtSeconds) +
            (1.0f - alpha_) * pitchAcc;

        roll_ =
            alpha_ * (roll_ + gx * dtSeconds) +
            (1.0f - alpha_) * rollAcc;

        // Yaw: sólo integración (sin magnetómetro tiene deriva,
        // pero es suficiente para detectar rotación a corto plazo).
        yaw_ = yaw_ + gz * dtSeconds;

        // Normalizar yaw a [-180, 180]
        while (yaw_ >  180.0f) yaw_ -= 360.0f;
        while (yaw_ < -180.0f) yaw_ += 360.0f;
    }
}