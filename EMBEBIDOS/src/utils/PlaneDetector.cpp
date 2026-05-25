#include "utils/PlaneDetector.hpp"

#include <cmath>

namespace Utils
{
    PlaneDetector::PlaneDetector(
        float thresholdG,
        float hysteresisG
    )
        : thresholdG_(thresholdG),
          hysteresisG_(hysteresisG)
    {
    }

    void PlaneDetector::reset()
    {
        current_ = Models::OrientationPlane::UNKNOWN;
    }

    Models::OrientationPlane PlaneDetector::detect(
        float ax,
        float ay,
        float az
    )
    {
        const float absX = fabsf(ax);
        const float absY = fabsf(ay);
        const float absZ = fabsf(az);

        // Para mantener el plano actual aplicamos histéresis:
        // se necesita que el nuevo eje dominante supere al actual
        // por al menos `hysteresisG_` para hacer el cambio.

        Models::OrientationPlane candidate =
            Models::OrientationPlane::UNKNOWN;

        if (absZ >= thresholdG_ &&
            absZ >= absX &&
            absZ >= absY)
        {
            candidate = Models::OrientationPlane::XY;
        }
        else if (absY >= thresholdG_ &&
                 absY >= absX &&
                 absY >= absZ)
        {
            candidate = Models::OrientationPlane::XZ;
        }
        else if (absX >= thresholdG_ &&
                 absX >= absY &&
                 absX >= absZ)
        {
            candidate = Models::OrientationPlane::YZ;
        }

        if (candidate == current_)
        {
            return current_;
        }

        // Cambio de plano: aplica histéresis comparando contra el eje del plano actual
        float currentAxisValue = 0.0f;

        switch (current_)
        {
            case Models::OrientationPlane::XY: currentAxisValue = absZ; break;
            case Models::OrientationPlane::XZ: currentAxisValue = absY; break;
            case Models::OrientationPlane::YZ: currentAxisValue = absX; break;
            default:                           currentAxisValue = 0.0f; break;
        }

        float candidateAxisValue = 0.0f;

        switch (candidate)
        {
            case Models::OrientationPlane::XY: candidateAxisValue = absZ; break;
            case Models::OrientationPlane::XZ: candidateAxisValue = absY; break;
            case Models::OrientationPlane::YZ: candidateAxisValue = absX; break;
            default:                           candidateAxisValue = 0.0f; break;
        }

        if (current_ == Models::OrientationPlane::UNKNOWN ||
            candidateAxisValue >= currentAxisValue + hysteresisG_)
        {
            current_ = candidate;
        }

        return current_;
    }
}