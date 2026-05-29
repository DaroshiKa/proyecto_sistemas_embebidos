#pragma once

#include <cstdint>

#include "driver/gpio.h"
#include "driver/ledc.h"

namespace Models
{
    enum class ServoMotionProfile : uint8_t
    {
        LINEAR = 0,
        SMOOTHSTEP
    };

    enum class JointId : uint8_t
    {
        HAND     = 0,
        WRIST    = 1,
        ELBOW_X  = 2,
        ELBOW_Y  = 3,
        ELBOW_Z  = 4,
        COUNT
    };

    struct JointConfig
    {
        JointId         id           { JointId::HAND };
        gpio_num_t      pin          { GPIO_NUM_18 };
        ledc_channel_t  channel      { LEDC_CHANNEL_0 };
        float           minAngle     { 0.0f };
        float           maxAngle     { 180.0f };
        float           homeAngle    { 90.0f };
        float           maxSpeedDps  { 180.0f };   // °/s
        bool            inverted     { false };
        bool            enabled      { true };
        const char*     name         { "servo" };
    };

    struct ServoState
    {
        JointId  jointId         { JointId::HAND };
        float    currentAngle    { 0.0f };
        float    targetAngle     { 0.0f };
        float    startAngle      { 0.0f };
        uint32_t startTimeMs     { 0 };
        uint32_t durationMs      { 0 };
        bool     moving          { false };
        bool     fault           { false };
        ServoMotionProfile profile { ServoMotionProfile::SMOOTHSTEP };

        // ⬇⬇⬇ NUEVOS CAMPOS ⬇⬇⬇
        float    currentSpeedDps { 0.0f };   // velocidad instantánea medida
        float    plannedSpeedDps { 0.0f };   // velocidad pedida al startMove
        float    lastAngleSnap   { 0.0f };   // helper interno para derivada
        uint32_t lastTickMs      { 0 };      // helper interno para derivada
    };
}