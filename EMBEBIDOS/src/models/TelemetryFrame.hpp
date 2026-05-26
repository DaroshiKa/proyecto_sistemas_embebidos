#pragma once

#include <cstdint>

#include "models/SensorData.hpp"
#include "models/ServoState.hpp"

namespace Models
{
    enum class TelemetryType : uint8_t
    {
        NONE = 0,
        IMU,
        EMG,
        SERVOS,
        SYSTEM,
        ALARM
    };

    struct TelemetryIMU
    {
        float    pitch;
        float    roll;
        float    yaw;
        uint8_t  plane;     // OrientationPlane
        uint8_t  state;     // IMUState
        uint8_t  calibrated;
    };

    struct TelemetryEMG
    {
        float    envelope;
        float    smoothed;
        uint8_t  gesture;       // EMGGesture
        uint8_t  active;
        uint8_t  state;         // EMGState
        uint8_t  calibrated;
    };

    struct TelemetryServos
    {
        // Un byte por servo con ángulo entero (0..180)
        uint8_t  angles[static_cast<size_t>(JointId::COUNT)];
        uint8_t  moving;        // bitmask: bit i = servo i en movimiento
    };

    struct TelemetrySystem
    {
        uint32_t uptimeMs;
        uint32_t freeHeap;
        uint8_t  systemState;
        uint8_t  emergencyActive;
    };

    struct TelemetryAlarm
    {
        uint8_t  level;         // AlarmLevel
        uint8_t  code;
    };

    struct TelemetryFrame
    {
        TelemetryType type     { TelemetryType::NONE };
        uint32_t timestampMs   { 0 };

        union {
            TelemetryIMU     imu;
            TelemetryEMG     emg;
            TelemetryServos  servos;
            TelemetrySystem  system;
            TelemetryAlarm   alarm;
        } data;

        TelemetryFrame() : type(TelemetryType::NONE), timestampMs(0)
        {
            data.system = {};
        }
    };
}