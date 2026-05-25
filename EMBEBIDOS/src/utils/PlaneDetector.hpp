#pragma once

#include "models/SensorData.hpp"

namespace Utils
{
    class PlaneDetector
    {
    public:
        explicit PlaneDetector(
            float thresholdG = 0.7f,
            float hysteresisG = 0.1f
        );

        // Devuelve el plano dominante considerando histéresis sobre
        // el plano anterior (evita parpadeo en bordes).
        Models::OrientationPlane detect(
            float ax,
            float ay,
            float az
        );

        Models::OrientationPlane current() const
        {
            return current_;
        }

        void reset();

    private:
        float thresholdG_;
        float hysteresisG_;
        Models::OrientationPlane current_ { Models::OrientationPlane::UNKNOWN };
    };
}