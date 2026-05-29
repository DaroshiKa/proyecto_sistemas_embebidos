#pragma once

#include <cstdint>

namespace Models
{
    enum class MotionType : uint8_t
    {
        NONE = 0,

        HAND_OPEN,
        HAND_CLOSE,

        WRIST_LEFT,
        WRIST_RIGHT,

        ELBOW_XY,
        ELBOW_XZ,
        ELBOW_YZ,

        HOME,
        RETURN_HOME,

        CUSTOM_SERVO,

        DEMO_START,
        DEMO_STOP,

        EMERGENCY_STOP
    };

    enum class CommandSource : uint8_t
    {
        UNKNOWN = 0,
        EMG,
        IMU,
        CLI,
        NEXTION,
        BLE,
        ROS2,
        DEMO
    };

    enum class CommandPriority : uint8_t
    {
        LOW = 0,
        NORMAL,
        HIGH,
        CRITICAL
    };

    enum class SystemState : uint8_t
    {
        INIT = 0,
        IDLE,
        ACTIVE,
        CALIBRATING,
        ERROR,
        SAFE_MODE,
        EMERGENCY_STOP
    };

    enum class AlarmLevel : uint8_t
    {
        INFO = 0,
        WARNING,
        ERROR,
        CRITICAL
    };
}