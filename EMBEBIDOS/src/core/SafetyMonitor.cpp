#include "core/SafetyMonitor.hpp"

#include "esp_log.h"
#include "esp_timer.h"

#include "models/MotionCommand.hpp"
#include "models/EventMessage.hpp"

namespace Core
{
    static constexpr const char* TAG = "SafetyMon";

    SafetyMonitor::SafetyMonitor(
        const SafetyDependencies& deps,
        const Models::SafetyConfig& config
    )
        : deps_(deps),
          config_(config)
    {
    }

    bool SafetyMonitor::initialize()
    {
        mutex_ = xSemaphoreCreateMutex();
        if (mutex_ == nullptr)
        {
            ESP_LOGE(TAG, "Failed to create mutex");
            return false;
        }

        // Estado inicial: INIT → IDLE tras inicialización
        transitionTo(Models::SystemState::IDLE, Models::AlarmCode::NONE);

        ESP_LOGI(TAG, "SafetyMonitor ready");
        return true;
    }

    void SafetyMonitor::transitionTo(
        Models::SystemState newState,
        Models::AlarmCode reason
    )
    {
        if (newState == systemState_) return;

        const Models::SystemState prev = systemState_;

        if (xSemaphoreTake(mutex_, pdMS_TO_TICKS(5)) == pdTRUE)
        {
            systemState_ = newState;
            xSemaphoreGive(mutex_);
        }
        else
        {
            systemState_ = newState;
        }

        ESP_LOGI(
            TAG,
            "State transition: %u -> %u (reason=%u)",
            static_cast<unsigned>(prev),
            static_cast<unsigned>(newState),
            static_cast<unsigned>(reason)
        );

        // Notificar al validator (para que sepa qué aceptar)
        if (deps_.validator != nullptr)
        {
            deps_.validator->onSystemStateChanged(newState);
        }

        // Notificar al EventBus
        if (deps_.eventBus != nullptr)
        {
            Models::EventMessage evt {};
            evt.type = Models::EventType::SYSTEM_STATUS_CHANGED;
            evt.timestampMs =
                static_cast<uint32_t>(esp_timer_get_time() / 1000ULL);
            deps_.eventBus->publish(evt);
        }
    }

    void SafetyMonitor::publishAlarm(
        Models::AlarmLevel level,
        Models::AlarmCode code,
        const char* message
    )
    {
        if (deps_.eventBus == nullptr) return;

        Models::EventMessage evt {};
        evt.timestampMs =
            static_cast<uint32_t>(esp_timer_get_time() / 1000ULL);

        // Reusamos los tipos existentes del bus
        switch (code)
        {
            case Models::AlarmCode::EMERGENCY_TRIGGERED:
                evt.type = Models::EventType::EMERGENCY_TRIGGERED;
                break;
            case Models::AlarmCode::SYSTEM_RECOVERED:
                evt.type = Models::EventType::EMERGENCY_CLEARED;
                break;
            default:
                evt.type = Models::EventType::SYSTEM_ERROR;
                break;
        }

        deps_.eventBus->publish(evt);

        ESP_LOGW(
            TAG,
            "ALARM lvl=%u code=%u msg=%s",
            static_cast<unsigned>(level),
            static_cast<unsigned>(code),
            (message != nullptr) ? message : ""
        );
    }

    void SafetyMonitor::triggerEmergency(Models::AlarmCode reason)
    {
        if (systemState_ == Models::SystemState::EMERGENCY_STOP) return;

        ESP_LOGE(TAG, "EMERGENCY triggered (reason=%u)",
                 static_cast<unsigned>(reason));

        ++totalEmergencies_;

        // Llevar al sistema a EMERGENCY ANTES de disparar el comando, para
        // que el validator no rechace un EMERGENCY_STOP por encontrarse "ya
        // en emergencia" en otro contexto.
        transitionTo(Models::SystemState::EMERGENCY_STOP, reason);

        publishAlarm(
            Models::AlarmLevel::CRITICAL,
            Models::AlarmCode::EMERGENCY_TRIGGERED,
            "Emergency triggered"
        );

        // Encolar comando CRITICAL para que TaskMotion pare los servos
        if (deps_.dispatcher != nullptr)
        {
            Models::MotionCommand stop {};
            stop.type        = Models::MotionType::EMERGENCY_STOP;
            stop.source      = Models::CommandSource::UNKNOWN;
            stop.priority    = Models::CommandPriority::CRITICAL;
            stop.timestampMs =
                static_cast<uint32_t>(esp_timer_get_time() / 1000ULL);

            deps_.dispatcher->dispatch(stop);
        }
        else if (deps_.executor != nullptr)
        {
            // Fallback directo
            deps_.executor->stopAll();
        }
    }

    bool SafetyMonitor::clearEmergency()
    {
        if (systemState_ != Models::SystemState::EMERGENCY_STOP &&
            systemState_ != Models::SystemState::SAFE_MODE)
        {
            // No estaba en emergencia: idempotente OK
            return true;
        }

        // Verificar que el sistema esté saludable antes de salir
        bool imuOk = true;
        bool emgOk = true;

        if (deps_.imu != nullptr)
        {
            const auto s = deps_.imu->getStatus();
            imuOk = (s.state == Models::IMUState::OK ||
                     s.state == Models::IMUState::UNINITIALIZED);
        }

        if (deps_.emg != nullptr)
        {
            const auto s = deps_.emg->getStatus();
            emgOk = (s.state == Models::EMGState::OK ||
                     s.state == Models::EMGState::UNINITIALIZED);
        }

        if (!imuOk && !emgOk)
        {
            ESP_LOGW(TAG, "Cannot clear: sensors still unhealthy");
            return false;
        }

        ++totalRecoveries_;
        transitionTo(Models::SystemState::IDLE, Models::AlarmCode::SYSTEM_RECOVERED);

        publishAlarm(
            Models::AlarmLevel::INFO,
            Models::AlarmCode::SYSTEM_RECOVERED,
            "Emergency cleared"
        );

        return true;
    }

    void SafetyMonitor::enterSafeMode()
    {
        transitionTo(
            Models::SystemState::SAFE_MODE,
            Models::AlarmCode::NONE
        );

        if (deps_.executor != nullptr)
        {
            deps_.executor->stopAll();
        }
    }

    void SafetyMonitor::checkTaskHeartbeats()
    {
        if (deps_.motionHb != nullptr)
        {
            const uint32_t age = deps_.motionHb->msSinceLastKick();
            if (age > config_.motionHeartbeatTimeoutMs)
            {
                ++totalTaskStalls_;
                ESP_LOGE(TAG, "Motion task STALE (age=%lu ms)",
                         static_cast<unsigned long>(age));
                triggerEmergency(Models::AlarmCode::MOTION_TASK_STALE);
                return;
            }
        }

        if (deps_.imuHb != nullptr)
        {
            const uint32_t age = deps_.imuHb->msSinceLastKick();
            if (age > config_.imuHeartbeatTimeoutMs)
            {
                ++totalTaskStalls_;
                ESP_LOGW(TAG, "IMU task STALE (age=%lu ms)",
                         static_cast<unsigned long>(age));
                // Stall del IMU: no es emergencia inmediata, vamos a ERROR.
                if (systemState_ == Models::SystemState::IDLE ||
                    systemState_ == Models::SystemState::ACTIVE)
                {
                    transitionTo(Models::SystemState::ERROR,
                                 Models::AlarmCode::IMU_TASK_STALE);
                    enteredErrorMs_ = static_cast<uint32_t>(
                        esp_timer_get_time() / 1000ULL);
                }
            }
        }

        if (deps_.emgHb != nullptr)
        {
            const uint32_t age = deps_.emgHb->msSinceLastKick();
            if (age > config_.emgHeartbeatTimeoutMs)
            {
                ++totalTaskStalls_;
                ESP_LOGW(TAG, "EMG task STALE (age=%lu ms)",
                         static_cast<unsigned long>(age));
                if (systemState_ == Models::SystemState::IDLE ||
                    systemState_ == Models::SystemState::ACTIVE)
                {
                    transitionTo(Models::SystemState::ERROR,
                                 Models::AlarmCode::EMG_TASK_STALE);
                    enteredErrorMs_ = static_cast<uint32_t>(
                        esp_timer_get_time() / 1000ULL);
                }
            }
        }
    }

    void SafetyMonitor::checkSensorHealth()
    {
        bool imuOk = true;
        bool emgOk = true;

        if (deps_.imu != nullptr)
        {
            const auto s = deps_.imu->getStatus();
            imuOk = (s.state != Models::IMUState::TIMEOUT &&
                     s.state != Models::IMUState::BUS_ERROR &&
                     s.state != Models::IMUState::FAULT);

            if (prevImuOk_ && !imuOk)
            {
                ++totalSensorTimeouts_;
                publishAlarm(
                    Models::AlarmLevel::WARNING,
                    Models::AlarmCode::IMU_TIMEOUT,
                    "IMU lost"
                );
            }
            prevImuOk_ = imuOk;
        }

        if (deps_.emg != nullptr)
        {
            const auto s = deps_.emg->getStatus();
            emgOk = (s.state != Models::EMGState::TIMEOUT &&
                     s.state != Models::EMGState::FAULT);

            if (prevEmgOk_ && !emgOk)
            {
                ++totalSensorTimeouts_;
                publishAlarm(
                    Models::AlarmLevel::WARNING,
                    Models::AlarmCode::EMG_TIMEOUT,
                    "EMG lost"
                );
            }
            prevEmgOk_ = emgOk;
        }

        // Política: ambos sensores fuera + estado activo → SAFE_MODE
        if (!imuOk && !emgOk &&
            (systemState_ == Models::SystemState::IDLE ||
             systemState_ == Models::SystemState::ACTIVE))
        {
            ESP_LOGE(TAG, "Both sensors offline → SAFE_MODE");
            transitionTo(Models::SystemState::SAFE_MODE,
                         Models::AlarmCode::IMU_TIMEOUT);
            if (deps_.executor != nullptr)
            {
                deps_.executor->stopAll();
            }
        }

        // Si veníamos de ERROR y todo se recuperó: auto-recovery
        if (systemState_ == Models::SystemState::ERROR &&
            imuOk && emgOk &&
            config_.autoRecoverFromError)
        {
            const uint32_t now =
                static_cast<uint32_t>(esp_timer_get_time() / 1000ULL);
            if ((now - enteredErrorMs_) > config_.recoveryGracePeriodMs)
            {
                ++totalRecoveries_;
                transitionTo(Models::SystemState::IDLE,
                             Models::AlarmCode::SYSTEM_RECOVERED);
                publishAlarm(
                    Models::AlarmLevel::INFO,
                    Models::AlarmCode::SYSTEM_RECOVERED,
                    "Recovered from ERROR"
                );
            }
        }
    }

    void SafetyMonitor::evaluateAutoRecovery()
    {
        // Placeholder por simetría: hoy la recuperación se maneja dentro de
        // checkSensorHealth. En Etapa 11 podría incorporar criterios más ricos.
    }

    void SafetyMonitor::tick()
    {
        checkTaskHeartbeats();
        checkSensorHealth();
        evaluateAutoRecovery();
    }
}