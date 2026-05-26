#include "core/RealSafetyValidator.hpp"

#include "esp_log.h"

namespace Core
{
    static constexpr const char* TAG = "SafetyValidator";

    RealSafetyValidator::RealSafetyValidator(
        const Models::SafetyConfig& config
    )
        : config_(config)
    {
    }

    void RealSafetyValidator::onSystemStateChanged(
        Models::SystemState newState
    )
    {
        systemState_ = newState;
    }

    bool RealSafetyValidator::isAllowedInState(
        Models::MotionType type,
        Models::CommandSource source
    ) const
    {
        // EMERGENCY_STOP siempre se acepta (idempotente)
        if (type == Models::MotionType::EMERGENCY_STOP)
        {
            return true;
        }

        switch (systemState_)
        {
            case Models::SystemState::INIT:
                // En INIT sólo aceptamos HOME (para mover servos a posición segura)
                return type == Models::MotionType::HOME;

            case Models::SystemState::IDLE:
            case Models::SystemState::ACTIVE:
                return true;

            case Models::SystemState::CALIBRATING:
                // Durante calibración: no aceptamos comandos de movimiento
                // que vengan de sensores (evita realimentación), pero CLI sí
                return (source == Models::CommandSource::CLI ||
                        source == Models::CommandSource::NEXTION);

            case Models::SystemState::ERROR:
                // En ERROR: sólo comandos de CLI/Nextion (operador presente)
                return (source == Models::CommandSource::CLI ||
                        source == Models::CommandSource::NEXTION);

            case Models::SystemState::SAFE_MODE:
                // En SAFE_MODE: sólo HOME y consultas implícitas
                return (type == Models::MotionType::HOME &&
                        (source == Models::CommandSource::CLI ||
                         source == Models::CommandSource::NEXTION));

            case Models::SystemState::EMERGENCY_STOP:
                // Sólo EMERGENCY_STOP pasa; el clear es operación fuera de banda
                return false;
        }

        return false;
    }

    bool RealSafetyValidator::checkServoLimits(
        const Models::MotionCommand& cmd
    ) const
    {
        if (cmd.type != Models::MotionType::CUSTOM_SERVO) return true;

        if (cmd.targetServo >= static_cast<uint8_t>(Models::JointId::COUNT))
        {
            return false;
        }

        if (cmd.targetAngle < 0.0f || cmd.targetAngle > 180.0f)
        {
            return false;
        }

        return true;
    }

    bool RealSafetyValidator::checkDeadtime(
        const Models::MotionCommand& cmd
    )
    {
        // Comandos CRITICAL no tienen deadtime
        if (cmd.priority == Models::CommandPriority::CRITICAL) return true;

        const uint8_t srcIdx = static_cast<uint8_t>(cmd.source);
        if (srcIdx >= 8) return true;

        const uint32_t last = lastAcceptedMsBySource_[srcIdx];
        if (last == 0) return true;  // primera vez

        if (cmd.timestampMs - last < config_.commandDeadtimeMs)
        {
            return false;
        }

        return true;
    }

    bool RealSafetyValidator::validate(
        const Models::MotionCommand& command
    )
    {
        // 1) Tipo válido
        if (command.type == Models::MotionType::NONE)
        {
            ++rejByLimits_;
            return false;
        }

        // 2) Estado del sistema
        if (!isAllowedInState(command.type, command.source))
        {
            ++rejByState_;
            ESP_LOGW(
                TAG,
                "Rejected by state: type=%u src=%u state=%u",
                static_cast<unsigned>(command.type),
                static_cast<unsigned>(command.source),
                static_cast<unsigned>(systemState_)
            );
            return false;
        }

        // 3) Límites mecánicos
        if (!checkServoLimits(command))
        {
            ++rejByLimits_;
            return false;
        }

        // 4) Deadtime
        if (!checkDeadtime(command))
        {
            ++rejByDeadtime_;
            return false;
        }

        // Aceptado: registrar timestamp por fuente
        const uint8_t srcIdx = static_cast<uint8_t>(command.source);
        if (srcIdx < 8)
        {
            lastAcceptedMsBySource_[srcIdx] = command.timestampMs;
        }

        ++totalAccepted_;
        return true;
    }
}