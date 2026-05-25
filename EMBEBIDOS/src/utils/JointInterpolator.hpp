#pragma once

#include <cstdint>

#include "models/ServoState.hpp"

namespace Utils
{
    class JointInterpolator
    {
    public:
        JointInterpolator();

        // Planifica un movimiento de startAngle a targetAngle a velocidad speedDps,
        // empezando en nowMs. Calcula la duración total.
        void planMove(
            float startAngle,
            float targetAngle,
            float speedDps,
            uint32_t nowMs,
            Models::ServoMotionProfile profile
        );

        // Forzar una duración específica (movimientos sincronizados).
        void planMoveWithDuration(
            float startAngle,
            float targetAngle,
            uint32_t durationMs,
            uint32_t nowMs,
            Models::ServoMotionProfile profile
        );

        // Devuelve el ángulo interpolado en el instante nowMs.
        // Si el movimiento ya terminó, devuelve target y marca done = true.
        float sample(uint32_t nowMs, bool& outDone) const;

        // Aborta el movimiento (curva queda congelada en el punto actual).
        void abort(float currentAngle);

        bool isActive() const { return active_; }

        uint32_t plannedDurationMs() const { return durationMs_; }

        float startAngle()  const { return startAngle_; }
        float targetAngle() const { return targetAngle_; }

    private:
        float    startAngle_  { 0.0f };
        float    targetAngle_ { 0.0f };
        uint32_t startMs_     { 0 };
        uint32_t durationMs_  { 0 };
        bool     active_      { false };
        Models::ServoMotionProfile profile_ { Models::ServoMotionProfile::LINEAR };
    };
}