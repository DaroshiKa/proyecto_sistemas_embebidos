#include "drivers/ServoManager.hpp"

#include "esp_log.h"

namespace Drivers
{
    static constexpr const char* TAG = "ServoManager";

    ServoManager::ServoManager(
        HAL::PWMHal& pwmHal,
        const Models::MotionConfig& config
    )
        : pwmHal_(pwmHal),
          config_(config),
          drivers_ {
              ServoDriver(pwmHal),
              ServoDriver(pwmHal),
              ServoDriver(pwmHal),
              ServoDriver(pwmHal),
              ServoDriver(pwmHal)
          }
    {
        for (size_t i = 0; i < N; ++i)
        {
            states_[i].jointId      = static_cast<Models::JointId>(i);
            states_[i].currentAngle = config_.joints[i].homeAngle;
            states_[i].targetAngle  = config_.joints[i].homeAngle;
        }
    }

    bool ServoManager::initialize()
    {
        stateMutex_ = xSemaphoreCreateMutex();
        if (stateMutex_ == nullptr) return false;

        if (!pwmHal_.initialize())
        {
            ESP_LOGE(TAG, "PWMHal init failed");
            return false;
        }

        for (size_t i = 0; i < N; ++i)
        {
            const auto& cfg = config_.joints[i];
            if (!cfg.enabled) continue;

            if (!drivers_[i].attach(cfg.pin, cfg.channel))
            {
                ESP_LOGE(TAG, "Servo %s attach failed", cfg.name);
                return false;
            }

            // Posición inicial: home, sin interpolación
            applyAngle(cfg.id, cfg.homeAngle);
        }

        ESP_LOGI(TAG, "ServoManager ready (%u joints)", static_cast<unsigned>(N));
        return true;
    }

    const Models::JointConfig& ServoManager::jointConfig(
        Models::JointId id
    ) const
    {
        return config_.joints[static_cast<size_t>(id)];
    }

    float ServoManager::clampAngle(
        const Models::JointConfig& cfg,
        float angle
    ) const
    {
        if (angle < cfg.minAngle) angle = cfg.minAngle;
        if (angle > cfg.maxAngle) angle = cfg.maxAngle;
        return angle;
    }

    bool ServoManager::applyAngle(Models::JointId id, float angle)
    {
        const size_t i = static_cast<size_t>(id);
        if (i >= N) return false;

        const auto& cfg = config_.joints[i];
        if (!cfg.enabled) return false;

        const float clamped = clampAngle(cfg, angle);

        // Inversión mecánica si aplica
        const float physical =
            cfg.inverted ? (cfg.maxAngle - (clamped - cfg.minAngle)) : clamped;

        const bool ok = drivers_[i].setAngle(physical);

        if (ok && xSemaphoreTake(stateMutex_, pdMS_TO_TICKS(2)) == pdTRUE)
        {
            states_[i].currentAngle = clamped;
            xSemaphoreGive(stateMutex_);
        }

        return ok;
    }

    void ServoManager::startMove(
        Models::JointId id,
        float targetAngle,
        float speedDps,
        uint32_t nowMs,
        Models::ServoMotionProfile profile,
        uint32_t forcedDurationMs
    )
    {
        const size_t i = static_cast<size_t>(id);
        if (i >= N) return;

        const auto& cfg = config_.joints[i];
        if (!cfg.enabled) return;

        const float clampedTarget = clampAngle(cfg, targetAngle);
        const float current       = states_[i].currentAngle;

        if (speedDps <= 0.0f || speedDps > cfg.maxSpeedDps)
        {
            speedDps = cfg.maxSpeedDps;
        }

        if (forcedDurationMs > 0)
        {
            interpolators_[i].planMoveWithDuration(
                current, clampedTarget, forcedDurationMs, nowMs, profile);
        }
        else
        {
            interpolators_[i].planMove(
                current, clampedTarget, speedDps, nowMs, profile);
        }

        if (xSemaphoreTake(stateMutex_, pdMS_TO_TICKS(2)) == pdTRUE)
        {
            states_[i].startAngle  = current;
            states_[i].targetAngle = clampedTarget;
            states_[i].startTimeMs = nowMs;
            states_[i].durationMs  = interpolators_[i].plannedDurationMs();
            states_[i].moving      = true;
            states_[i].profile     = profile;
            xSemaphoreGive(stateMutex_);
        }
    }

    bool ServoManager::executeServoCommand(
        const Models::ServoCommand& cmd
    )
    {
        const uint32_t now = cmd.timestampMs;

        float target = cmd.targetAngle;
        if (cmd.relative)
        {
            const size_t i = static_cast<size_t>(cmd.jointId);
            if (i >= N) return false;
            target = states_[i].currentAngle + cmd.targetAngle;
        }

        startMove(cmd.jointId, target, cmd.speedDps, now, cmd.profile);
        return true;
    }

    bool ServoManager::executeCoordinatedMotion(
        const Models::CoordinatedMotion& motion
    )
    {
        if (motion.count == 0) return false;

        // Calcular duración del más lento si la sincronización está activada
        uint32_t maxDuration = 0;
        uint32_t now = 0;

        if (motion.synchronized)
        {
            for (uint8_t k = 0; k < motion.count; ++k)
            {
                const auto& c = motion.commands[k];
                const size_t i = static_cast<size_t>(c.jointId);
                if (i >= N) continue;

                const auto& cfg = config_.joints[i];
                if (!cfg.enabled) continue;

                const float target = clampAngle(cfg, c.targetAngle);
                const float distance =
                    fabsf(target - states_[i].currentAngle);

                float speed = c.speedDps;
                if (speed <= 0.0f || speed > cfg.maxSpeedDps)
                {
                    speed = cfg.maxSpeedDps;
                }

                const uint32_t d =
                    static_cast<uint32_t>((distance / speed) * 1000.0f);

                if (d > maxDuration) maxDuration = d;
                if (c.timestampMs > now) now = c.timestampMs;
            }

            if (maxDuration < 1) maxDuration = 1;
        }

        for (uint8_t k = 0; k < motion.count; ++k)
        {
            const auto& c = motion.commands[k];

            float target = c.targetAngle;
            if (c.relative)
            {
                const size_t i = static_cast<size_t>(c.jointId);
                if (i >= N) continue;
                target = states_[i].currentAngle + c.targetAngle;
            }

            startMove(
                c.jointId,
                target,
                c.speedDps,
                (now > 0) ? now : c.timestampMs,
                c.profile,
                motion.synchronized ? maxDuration : 0
            );
        }

        return true;
    }

    void ServoManager::stopAll()
    {
        for (size_t i = 0; i < N; ++i)
        {
            interpolators_[i].abort(states_[i].currentAngle);
        }

        if (xSemaphoreTake(stateMutex_, pdMS_TO_TICKS(2)) == pdTRUE)
        {
            for (size_t i = 0; i < N; ++i)
            {
                states_[i].moving      = false;
                states_[i].targetAngle = states_[i].currentAngle;
            }
            xSemaphoreGive(stateMutex_);
        }

        ESP_LOGW(TAG, "stopAll() invoked");
    }

    bool ServoManager::goHome()
    {
        Models::CoordinatedMotion motion {};
        motion.synchronized = true;
        motion.count = 0;

        for (size_t i = 0; i < N && motion.count < Models::CoordinatedMotion::MAX_SERVOS; ++i)
        {
            const auto& cfg = config_.joints[i];
            if (!cfg.enabled) continue;

            auto& c = motion.commands[motion.count++];
            c.jointId     = cfg.id;
            c.targetAngle = cfg.homeAngle;
            c.speedDps    = cfg.maxSpeedDps * 0.5f;
            c.profile     = Models::ServoMotionProfile::SMOOTHSTEP;
            c.timestampMs = 0;
            c.relative    = false;
        }

        return executeCoordinatedMotion(motion);
    }

    bool ServoManager::getServoState(
        Models::JointId jointId,
        Models::ServoState& outState
    ) const
    {
        const size_t i = static_cast<size_t>(jointId);
        if (i >= N) return false;

        if (stateMutex_ != nullptr &&
            xSemaphoreTake(stateMutex_, pdMS_TO_TICKS(5)) == pdTRUE)
        {
            outState = states_[i];
            xSemaphoreGive(stateMutex_);
            return true;
        }

        outState = states_[i];
        return true;
    }

    bool ServoManager::isAnyMoving() const
    {
        for (size_t i = 0; i < N; ++i)
        {
            if (states_[i].moving) return true;
        }
        return false;
    }

    void ServoManager::tick(uint32_t nowMs)
    {
        for (size_t i = 0; i < N; ++i)
        {
            if (!interpolators_[i].isActive())
            {
                continue;
            }

            bool done = false;
            const float angle = interpolators_[i].sample(nowMs, done);

            applyAngle(static_cast<Models::JointId>(i), angle);

            if (done)
            {
                interpolators_[i].abort(angle);

                if (xSemaphoreTake(stateMutex_, pdMS_TO_TICKS(2)) == pdTRUE)
                {
                    states_[i].moving = false;
                    xSemaphoreGive(stateMutex_);
                }
            }
        }
    }
}