#pragma once
#include <cstdint>
namespace Models
{
    struct ServoState
    {
        uint8_t servoId { 0 };

        float currentAngle { 0.0f };

        float targetAngle { 0.0f };

        bool moving { false };

        bool fault { false };
    };
}