#pragma once

#include <cstdint>

#include "models/ServoState.hpp"

namespace Models
{
    struct ServoCommand
    {
        JointId             jointId      { JointId::HAND };
        float               targetAngle  { 0.0f };
        float               speedDps     { 180.0f };   // °/s
        ServoMotionProfile  profile      { ServoMotionProfile::SMOOTHSTEP };
        uint32_t            timestampMs  { 0 };
        bool                relative     { false };
    };

    // Una secuencia coordinada: varios servos arrancan juntos
    // y terminan juntos (la duración se ajusta al más lento).
    struct CoordinatedMotion
    {
        static constexpr uint8_t MAX_SERVOS = 8;

        ServoCommand commands[MAX_SERVOS] {};
        uint8_t      count                { 0 };
        bool         synchronized         { true };
    };
}