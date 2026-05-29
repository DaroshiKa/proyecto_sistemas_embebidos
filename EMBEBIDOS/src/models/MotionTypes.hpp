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

        EMERGENCY_STOP,
        CLEAR_EMERGENCY    // ← NUEVO (Etapa 9)
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
        DEMO,
        SAFETY             // ← NUEVO (Etapa 9): comandos generados por TaskSafety
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

    // NUEVO (Etapa 9): causa específica de una alarma. Permite a CLI/Nextion
    // mostrar mensajes precisos sin acoplarse a strings.
    enum class AlarmCode : uint8_t
    {
        NONE = 0,
        SENSOR_IMU_TIMEOUT,
        SENSOR_EMG_TIMEOUT,
        SENSOR_RECONNECTED,
        INVALID_COMMAND,
        OUT_OF_RANGE,
        EMERGENCY_STOP_TRIGGERED,
        EMERGENCY_CLEARED,
        SAFE_MODE_ENTERED,
        SAFE_MODE_EXITED,
        TASK_WATCHDOG_NEAR_TIMEOUT,
        FAULT_UNRECOVERABLE
    };
}