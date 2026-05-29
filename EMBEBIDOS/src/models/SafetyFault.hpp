#pragma once

#include <cstdint>

namespace Models
{
    // Bitfield: permite tener varias faltas activas a la vez sin allocaciones.
    // Cada bit representa una causa raíz independiente.
    enum class SafetyFault : uint16_t
    {
        NONE                   = 0x0000,

        IMU_TIMEOUT            = 0x0001,  // sensor no responde
        IMU_BUS_ERROR          = 0x0002,  // I2C NACK persistente
        IMU_NOT_CALIBRATED     = 0x0004,  // se usa pero no se calibró

        EMG_TIMEOUT            = 0x0008,
        EMG_NOT_CALIBRATED     = 0x0010,

        SERVO_LIMIT_VIOLATION  = 0x0020,  // se intentó ángulo fuera de límite
        SERVO_STALL_SUSPECTED  = 0x0040,  // (reservado para futuras estimaciones)

        QUEUE_OVERFLOW         = 0x0080,  // dropped commands > umbral

        COMMAND_RATE_EXCEEDED  = 0x0100,  // spam de comandos de una fuente

        WATCHDOG_TASK_STARVED  = 0x0200,  // tarea crítica no alimentó TWDT
        WATCHDOG_TRIGGERED     = 0x0400,

        HEAP_LOW               = 0x0800,  // free heap por debajo del umbral
        STACK_LOW              = 0x1000,  // task stack high-water bajo

        USER_REQUESTED_ESTOP   = 0x2000,  // E-Stop comandado desde CLI/EMG/IMU
        INIT_FAILURE           = 0x4000   // arranque incompleto
    };

    // Operadores bitwise para usar el enum como bitfield sin perder type-safety.
    inline constexpr SafetyFault operator|(SafetyFault a, SafetyFault b)
    {
        return static_cast<SafetyFault>(
            static_cast<uint16_t>(a) | static_cast<uint16_t>(b)
        );
    }

    inline constexpr SafetyFault operator&(SafetyFault a, SafetyFault b)
    {
        return static_cast<SafetyFault>(
            static_cast<uint16_t>(a) & static_cast<uint16_t>(b)
        );
    }

    inline constexpr SafetyFault operator~(SafetyFault a)
    {
        return static_cast<SafetyFault>(~static_cast<uint16_t>(a));
    }

    inline SafetyFault& operator|=(SafetyFault& a, SafetyFault b)
    {
        a = a | b;
        return a;
    }

    inline SafetyFault& operator&=(SafetyFault& a, SafetyFault b)
    {
        a = a & b;
        return a;
    }

    inline constexpr bool hasFault(SafetyFault mask, SafetyFault flag)
    {
        return static_cast<uint16_t>(mask & flag) != 0;
    }

    inline constexpr bool isAnyFault(SafetyFault mask)
    {
        return static_cast<uint16_t>(mask) != 0;
    }
}