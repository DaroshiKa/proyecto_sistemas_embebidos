#include "services/MotionService.hpp"

#include "esp_log.h"
#include "esp_timer.h"

namespace Services
{
    static constexpr const char* TAG = "MotionService";

    MotionService::MotionService(
        Interfaces::IMotionExecutor& executor,
        Core::EventBus& eventBus,
        const Models::MotionConfig& config
    )
        : executor_(executor),
          eventBus_(eventBus),
          config_(config)
    {
    }

    bool MotionService::initialize()
    {
        ESP_LOGI(TAG, "MotionService ready");
        return true;
    }

    void MotionService::update()
    {
        // El bucle de control lo lleva TaskMotion → executor_.tick().
        // Este update() existe sólo por contrato IService.
    }

    bool MotionService::processMotionCommand(
        const Models::MotionCommand& command,
        uint32_t nowMs
    )
    {
        bool ok = false;

        switch (command.type)
        {
            case Models::MotionType::EMERGENCY_STOP:
                ok = dispatchEmergencyStop();
                break;

            case Models::MotionType::HAND_OPEN:
                ok = dispatchHandOpen(nowMs);
                break;
            case Models::MotionType::HAND_CLOSE:
                ok = dispatchHandClose(nowMs);
                break;

            case Models::MotionType::WRIST_LEFT:
                ok = dispatchWristLeft(nowMs);
                break;
            case Models::MotionType::WRIST_RIGHT:
                ok = dispatchWristRight(nowMs);
                break;

            case Models::MotionType::ELBOW_XY:
            case Models::MotionType::ELBOW_XZ:
            case Models::MotionType::ELBOW_YZ:
                ok = dispatchElbowPlane(command.type, nowMs);
                break;

            case Models::MotionType::HOME:
            case Models::MotionType::RETURN_HOME:
                ok = dispatchHome(nowMs);
                break;

            case Models::MotionType::CUSTOM_SERVO:
                ok = dispatchCustomServo(command, nowMs);
                break;

            case Models::MotionType::DEMO_START:
            case Models::MotionType::DEMO_STOP:
                // Se manejan en otro servicio (Etapa 7). Aquí los ignoramos.
                ++totalIgnored_;
                return false;

            default:
                ++totalIgnored_;
                return false;
        }

        if (ok)
        {
            ++totalExecuted_;
            publishExecuted(command, nowMs);
        }
        else
        {
            ++totalIgnored_;
        }

        return ok;
    }

    void MotionService::publishExecuted(
        const Models::MotionCommand& cmd,
        uint32_t now
    )
    {
        (void)cmd;
        Models::EventMessage evt {};
        evt.type        = Models::EventType::MOTION_EXECUTED;
        evt.timestampMs = now;
        eventBus_.publish(evt);
    }

    // --- Movimientos discretos ---

    bool MotionService::dispatchHandOpen(uint32_t now)
    {
        Models::ServoCommand c {};
        c.jointId     = Models::JointId::HAND;
        c.targetAngle = config_.handOpenAngle;
        c.speedDps    = config_.defaultSpeedDps;
        c.profile     = Models::ServoMotionProfile::SMOOTHSTEP;
        c.timestampMs = now;
        return executor_.executeServoCommand(c);
    }

    bool MotionService::dispatchHandClose(uint32_t now)
    {
        Models::ServoCommand c {};
        c.jointId     = Models::JointId::HAND;
        c.targetAngle = config_.handCloseAngle;
        c.speedDps    = config_.defaultSpeedDps;
        c.profile     = Models::ServoMotionProfile::SMOOTHSTEP;
        c.timestampMs = now;
        return executor_.executeServoCommand(c);
    }

    bool MotionService::dispatchWristLeft(uint32_t now)
    {
        Models::ServoCommand c {};
        c.jointId     = Models::JointId::WRIST;
        c.targetAngle = config_.wristLeftAngle;
        c.speedDps    = config_.defaultSpeedDps;
        c.profile     = Models::ServoMotionProfile::SMOOTHSTEP;
        c.timestampMs = now;
        return executor_.executeServoCommand(c);
    }

    bool MotionService::dispatchWristRight(uint32_t now)
    {
        Models::ServoCommand c {};
        c.jointId     = Models::JointId::WRIST;
        c.targetAngle = config_.wristRightAngle;
        c.speedDps    = config_.defaultSpeedDps;
        c.profile     = Models::ServoMotionProfile::SMOOTHSTEP;
        c.timestampMs = now;
        return executor_.executeServoCommand(c);
    }

    bool MotionService::dispatchElbowPlane(
        Models::MotionType plane,
        uint32_t now
    )
    {
        // Plano XY = postura neutra, XZ = lateral, YZ = frontal.
        // Asignación simbólica: cada plano fija los 3 joints del codo
        // en una pose concreta. El usuario afina esto por config.

        float xAngle = 90.0f, yAngle = 90.0f, zAngle = 90.0f;

        switch (plane)
        {
            case Models::MotionType::ELBOW_XY:
                xAngle = config_.elbowXyAngle;
                yAngle = config_.elbowXyAngle;
                zAngle = 90.0f;
                break;
            case Models::MotionType::ELBOW_XZ:
                xAngle = config_.elbowXzAngle;
                yAngle = 90.0f;
                zAngle = config_.elbowXzAngle;
                break;
            case Models::MotionType::ELBOW_YZ:
                xAngle = 90.0f;
                yAngle = config_.elbowYzAngle;
                zAngle = config_.elbowYzAngle;
                break;
            default:
                return false;
        }

        Models::CoordinatedMotion motion {};
        motion.synchronized = true;
        motion.count = 3;

        motion.commands[0].jointId     = Models::JointId::ELBOW_X;
        motion.commands[0].targetAngle = xAngle;
        motion.commands[0].speedDps    = config_.defaultSpeedDps;
        motion.commands[0].profile     = Models::ServoMotionProfile::SMOOTHSTEP;
        motion.commands[0].timestampMs = now;

        motion.commands[1].jointId     = Models::JointId::ELBOW_Y;
        motion.commands[1].targetAngle = yAngle;
        motion.commands[1].speedDps    = config_.defaultSpeedDps;
        motion.commands[1].profile     = Models::ServoMotionProfile::SMOOTHSTEP;
        motion.commands[1].timestampMs = now;

        motion.commands[2].jointId     = Models::JointId::ELBOW_Z;
        motion.commands[2].targetAngle = zAngle;
        motion.commands[2].speedDps    = config_.defaultSpeedDps;
        motion.commands[2].profile     = Models::ServoMotionProfile::SMOOTHSTEP;
        motion.commands[2].timestampMs = now;

        return executor_.executeCoordinatedMotion(motion);
    }

    bool MotionService::dispatchHome(uint32_t now)
    {
        (void)now;
        return executor_.goHome();
    }

    bool MotionService::dispatchCustomServo(
        const Models::MotionCommand& cmd,
        uint32_t now
    )
    {
        if (cmd.targetServo >= static_cast<uint8_t>(Models::JointId::COUNT))
        {
            return false;
        }

        Models::ServoCommand c {};
        c.jointId     = static_cast<Models::JointId>(cmd.targetServo);
        c.targetAngle = cmd.targetAngle;
        c.speedDps    = (cmd.speed > 0.0f) ? cmd.speed : config_.defaultSpeedDps;
        c.profile     = Models::ServoMotionProfile::SMOOTHSTEP;
        c.timestampMs = now;
        return executor_.executeServoCommand(c);
    }

    bool MotionService::dispatchEmergencyStop()
    {
        executor_.stopAll();

        Models::EventMessage evt {};
        evt.type        = Models::EventType::EMERGENCY_TRIGGERED;
        evt.timestampMs = static_cast<uint32_t>(
            esp_timer_get_time() / 1000ULL);
        eventBus_.publish(evt);

        return true;
    }
}