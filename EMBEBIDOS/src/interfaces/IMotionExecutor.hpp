#pragma once

#include "models/ServoCommand.hpp"
#include "models/ServoState.hpp"

namespace Interfaces
{
    class IMotionExecutor
    {
    public:
        virtual ~IMotionExecutor() = default;

        // Inicia un movimiento sobre un único servo.
        virtual bool executeServoCommand(
            const Models::ServoCommand& cmd
        ) = 0;

        // Inicia un movimiento coordinado (varios servos arrancan juntos).
        virtual bool executeCoordinatedMotion(
            const Models::CoordinatedMotion& motion
        ) = 0;

        // Detiene todo movimiento en curso. Usado por emergency stop.
        virtual void stopAll() = 0;

        // Mueve todos los joints a home, coordinadamente.
        virtual bool goHome() = 0;

        // Lectura del estado de un joint.
        virtual bool getServoState(
            Models::JointId jointId,
            Models::ServoState& outState
        ) const = 0;

        // Tick del bucle de control: avanza interpolaciones, aplica al hardware.
        virtual void tick(uint32_t nowMs) = 0;
    };
}