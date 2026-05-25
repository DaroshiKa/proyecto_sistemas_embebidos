#pragma once

namespace Models
{
    struct EMGData
    {
        float rawValue { 0.0f };

        float filteredValue { 0.0f };

        bool contractionDetected { false };
    };

    struct IMUData
    {
        float pitch { 0.0f };

        float roll { 0.0f };

        float yaw { 0.0f };

        bool valid { false };
    };
}