#pragma once

#include <cstdint>
#include "models/ServoState.hpp"

namespace Models
{
    struct MotionConfig
    {
        // Configuración de cada joint. Indexado por JointId.
        JointConfig joints[static_cast<size_t>(JointId::COUNT)] {
            { JointId::HAND,    GPIO_NUM_18, LEDC_CHANNEL_0,   0.0f, 180.0f,  90.0f, 180.0f, false, true, "hand"    },
            { JointId::WRIST,   GPIO_NUM_19, LEDC_CHANNEL_1,   0.0f, 180.0f,  90.0f, 180.0f, false, true, "wrist"   },
            { JointId::ELBOW_X, GPIO_NUM_5,  LEDC_CHANNEL_2,   0.0f, 180.0f,  90.0f, 120.0f, false, true, "elbowX"  },
            { JointId::ELBOW_Y, GPIO_NUM_17, LEDC_CHANNEL_3,   0.0f, 180.0f,  90.0f, 120.0f, false, true, "elbowY"  },
            { JointId::ELBOW_Z, GPIO_NUM_16, LEDC_CHANNEL_4,   0.0f, 180.0f,  90.0f, 120.0f, false, true, "elbowZ"  }
        };

        // Velocidad por defecto (cuando un MotionCommand trae speed = 0.0)
        float    defaultSpeedDps        { 90.0f };

        // Ángulos para movimientos discretos predefinidos
        float    handOpenAngle          { 0.0f };
        float    handCloseAngle         { 180.0f };
        float    wristLeftAngle         { 30.0f };
        float    wristRightAngle        { 150.0f };
        float    elbowXyAngle           { 90.0f };
        float    elbowXzAngle           { 60.0f };
        float    elbowYzAngle           { 120.0f };

        // Periodo del control loop
        uint32_t controlLoopPeriodMs    { 10 };   // 100 Hz
    };
}