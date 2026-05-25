#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include "interfaces/IMotionExecutor.hpp"

#include "drivers/ServoDriver.hpp"

#include "utils/JointInterpolator.hpp"

#include "models/MotionConfig.hpp"
#include "models/ServoState.hpp"
#include "models/ServoCommand.hpp"
#include <cmath>
namespace Drivers
{
    class ServoManager final :
        public Interfaces::IMotionExecutor
    {
    public:
        ServoManager(
            HAL::PWMHal& pwmHal,
            const Models::MotionConfig& config
        );

        bool initialize();

        // ---- IMotionExecutor ----
        bool executeServoCommand(
            const Models::ServoCommand& cmd
        ) override;

        bool executeCoordinatedMotion(
            const Models::CoordinatedMotion& motion
        ) override;

        void stopAll() override;

        bool goHome() override;

        bool getServoState(
            Models::JointId jointId,
            Models::ServoState& outState
        ) const override;

        void tick(uint32_t nowMs) override;

        // Utilidades
        bool isAnyMoving() const;

        const Models::JointConfig& jointConfig(Models::JointId id) const;

    private:
        // Helper: aplica límites, inversión y mueve el servo a un ángulo
        bool applyAngle(
            Models::JointId id,
            float angle
        );

        // Verifica límites y devuelve ángulo clampeado
        float clampAngle(
            const Models::JointConfig& cfg,
            float angle
        ) const;

        // Comienza un movimiento sobre un servo concreto
        void startMove(
            Models::JointId id,
            float targetAngle,
            float speedDps,
            uint32_t nowMs,
            Models::ServoMotionProfile profile,
            uint32_t forcedDurationMs = 0
        );

        static constexpr size_t N = static_cast<size_t>(Models::JointId::COUNT);

        HAL::PWMHal&            pwmHal_;
        Models::MotionConfig    config_;

        ServoDriver             drivers_[N];
        Utils::JointInterpolator interpolators_[N];

        // Mutex sólo para lectura concurrente del estado
        mutable SemaphoreHandle_t stateMutex_ { nullptr };
        Models::ServoState        states_[N];
    };
}
