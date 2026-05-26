#include "services/SafetyService.hpp"

#include "esp_log.h"
#include "esp_timer.h"

#include "models/ServoState.hpp"

namespace Services
{
    static constexpr const char* TAG = "SafetyService";

    static uint32_t nowMsStatic()
    {
        return static_cast<uint32_t>(esp_timer_get_time() / 1000ULL);
    }

    SafetyService::SafetyService(
        Core::SystemStateMachine& fsm,
        Core::EventBus& eventBus,
        const Models::SafetyConfig& config
    )
        : fsm_(fsm),
          eventBus_(eventBus),
          config_(config)
    {
    }

    SafetyService::~SafetyService()
    {
        if (mutex_ != nullptr)
        {
            vSemaphoreDelete(mutex_);
            mutex_ = nullptr;
        }
    }

    bool SafetyService::initialize()
    {
        mutex_ = xSemaphoreCreateMutex();
        if (mutex_ == nullptr)
        {
            ESP_LOGE(TAG, "Failed to create mutex");
            return false;
        }
        status_ = Models::SafetyStatus{};
        status_.systemState = fsm_.currentState();
        ESP_LOGI(TAG, "Initialized");
        return true;
    }

    // ============================================================
    // Validación de comandos (invocado por CommandDispatcher)
    // ============================================================
    bool SafetyService::validate(const Models::MotionCommand& cmd)
    {
        if (mutex_ == nullptr) return false;

        const uint32_t now = nowMsStatic();

        // 1) Filtrado básico: tipo NONE no es válido como input.
        //    EMERGENCY_STOP y CLEAR_EMERGENCY se manejan especialmente.
        if (cmd.type == Models::MotionType::NONE)
        {
            xSemaphoreTake(mutex_, portMAX_DELAY);
            ++status_.totalCommandsRejected;
            xSemaphoreGive(mutex_);
            return false;
        }

        // 2) EMERGENCY_STOP: siempre se acepta, independientemente del estado.
        //    Activa el lockout y notifica.
        if (cmd.type == Models::MotionType::EMERGENCY_STOP)
        {
            triggerEmergencyStop(Models::AlarmCode::EMERGENCY_STOP_TRIGGERED);
            xSemaphoreTake(mutex_, portMAX_DELAY);
            ++status_.totalCommandsValidated;
            xSemaphoreGive(mutex_);
            return true;
        }

        // 3) CLEAR_EMERGENCY: válido solo en EMERGENCY_STOP.
        if (cmd.type == Models::MotionType::CLEAR_EMERGENCY)
        {
            const bool ok = clearEmergency();
            xSemaphoreTake(mutex_, portMAX_DELAY);
            if (ok) ++status_.totalCommandsValidated;
            else    ++status_.totalCommandsRejected;
            xSemaphoreGive(mutex_);
            return ok;
        }

        // 4) Rangos: validar antes que estado para detectar comandos basura.
        if (!checkCommandRanges(cmd))
        {
            xSemaphoreTake(mutex_, portMAX_DELAY);
            ++status_.totalCommandsRejected;
            xSemaphoreGive(mutex_);
            publishAlarm(
                Models::AlarmCode::OUT_OF_RANGE,
                Models::AlarmLevel::WARNING, now);
            return false;
        }

        // 5) Lockout post-emergencia.
        if (config_.checkLockout && !checkLockout(cmd, now))
        {
            xSemaphoreTake(mutex_, portMAX_DELAY);
            ++status_.totalCommandsRejected;
            ++status_.totalLockoutRejections;
            xSemaphoreGive(mutex_);
            return false;
        }

        // 6) Estado del sistema permite este comando?
        if (!checkSystemStateAllowsCommand(cmd))
        {
            xSemaphoreTake(mutex_, portMAX_DELAY);
            ++status_.totalCommandsRejected;
            ++status_.totalStateInvalidations;
            xSemaphoreGive(mutex_);
            return false;
        }

        // 7) Deadtime anti-rebote.
        if (config_.checkDeadtime && !checkDeadtime(cmd, now))
        {
            xSemaphoreTake(mutex_, portMAX_DELAY);
            ++status_.totalCommandsRejected;
            ++status_.totalDeadtimeRejections;
            xSemaphoreGive(mutex_);
            return false;
        }

        // 8) Aceptado.
        recordAcceptedCommand(cmd, now);

        // 9) Si estamos en IDLE y entra un comando de movimiento, vamos a ACTIVE.
        const auto st = fsm_.currentState();
        if (st == Models::SystemState::IDLE &&
            cmd.type != Models::MotionType::DEMO_START &&
            cmd.type != Models::MotionType::DEMO_STOP)
        {
            fsm_.toActive();
        }

        return true;
    }

    // ============================================================
    // Tick periódico (TaskSafety lo llama a ~5 Hz)
    // ============================================================
    void SafetyService::tick(uint32_t nowMs)
    {
        if (mutex_ == nullptr) return;

        // ---- IMU watchdog ----
        if (config_.checkImuTimeout && imuHealth_ != nullptr)
        {
            const uint32_t lastImu = imuHealth_->lastSampleTimestampMs();
            const bool healthy =
                (lastImu != 0) &&
                ((nowMs - lastImu) < config_.imuTimeoutMs) &&
                imuHealth_->isSensorOk();

            xSemaphoreTake(mutex_, portMAX_DELAY);
            status_.imuHealthy = healthy;
            xSemaphoreGive(mutex_);

            if (!healthy && imuWasHealthy_)
            {
                xSemaphoreTake(mutex_, portMAX_DELAY);
                ++status_.imuTimeouts;
                xSemaphoreGive(mutex_);

                publishAlarm(
                    Models::AlarmCode::SENSOR_IMU_TIMEOUT,
                    Models::AlarmLevel::ERROR, nowMs);

                // Transición a SAFE_MODE si estamos activos.
                const auto st = fsm_.currentState();
                if (st == Models::SystemState::ACTIVE ||
                    st == Models::SystemState::IDLE)
                {
                    fsm_.toSafeMode(Models::AlarmCode::SENSOR_IMU_TIMEOUT);
                }
            }
            else if (healthy && !imuWasHealthy_)
            {
                publishAlarm(
                    Models::AlarmCode::SENSOR_RECONNECTED,
                    Models::AlarmLevel::INFO, nowMs);

                // Auto-recovery desde SAFE_MODE si ambos sensores ok.
                const auto st = fsm_.currentState();
                if (st == Models::SystemState::SAFE_MODE &&
                    status_.emgHealthy)
                {
                    fsm_.exitSafeMode();
                }
            }
            imuWasHealthy_ = healthy;
        }

        // ---- EMG watchdog ----
        if (config_.checkEmgTimeout && emgHealth_ != nullptr)
        {
            const uint32_t lastEmg = emgHealth_->lastSampleTimestampMs();
            const bool healthy =
                (lastEmg != 0) &&
                ((nowMs - lastEmg) < config_.emgTimeoutMs) &&
                emgHealth_->isSensorOk();

            xSemaphoreTake(mutex_, portMAX_DELAY);
            status_.emgHealthy = healthy;
            xSemaphoreGive(mutex_);

            if (!healthy && emgWasHealthy_)
            {
                xSemaphoreTake(mutex_, portMAX_DELAY);
                ++status_.emgTimeouts;
                xSemaphoreGive(mutex_);

                publishAlarm(
                    Models::AlarmCode::SENSOR_EMG_TIMEOUT,
                    Models::AlarmLevel::ERROR, nowMs);

                const auto st = fsm_.currentState();
                if (st == Models::SystemState::ACTIVE ||
                    st == Models::SystemState::IDLE)
                {
                    fsm_.toSafeMode(Models::AlarmCode::SENSOR_EMG_TIMEOUT);
                }
            }
            else if (healthy && !emgWasHealthy_)
            {
                publishAlarm(
                    Models::AlarmCode::SENSOR_RECONNECTED,
                    Models::AlarmLevel::INFO, nowMs);

                const auto st = fsm_.currentState();
                if (st == Models::SystemState::SAFE_MODE &&
                    status_.imuHealthy)
                {
                    fsm_.exitSafeMode();
                }
            }
            emgWasHealthy_ = healthy;
        }

        // ---- Lockout decrementer ----
        xSemaphoreTake(mutex_, portMAX_DELAY);
        if (emergencyActive_)
        {
            if (nowMs >= emergencyLockoutUntilMs_)
            {
                emergencyActive_ = false;
                status_.inEmergencyLockout = false;
                status_.emergencyLockoutRemainingMs = 0;
            }
            else
            {
                status_.emergencyLockoutRemainingMs =
                    emergencyLockoutUntilMs_ - nowMs;
            }
        }
        xSemaphoreGive(mutex_);

        // ---- Idle timeout: ACTIVE -> IDLE si no hay actividad ----
        if (config_.checkIdleTimeout)
        {
            const auto st = fsm_.currentState();
            if (st == Models::SystemState::ACTIVE &&
                lastActivityMs_ != 0 &&
                (nowMs - lastActivityMs_) > config_.motionIdleTimeoutMs)
            {
                fsm_.toIdle();
            }
        }

        // ---- Sincroniza snapshot de estado de FSM ----
        xSemaphoreTake(mutex_, portMAX_DELAY);
        status_.systemState = fsm_.currentState();
        xSemaphoreGive(mutex_);
    }

    // ============================================================
    // API explícita
    // ============================================================
    void SafetyService::triggerEmergencyStop(Models::AlarmCode cause)
    {
        const uint32_t now = nowMsStatic();

        xSemaphoreTake(mutex_, portMAX_DELAY);
        emergencyActive_           = true;
        emergencyLockoutUntilMs_   = now + config_.emergencyLockoutMs;
        status_.inEmergencyLockout = true;
        status_.emergencyLockoutRemainingMs = config_.emergencyLockoutMs;
        status_.lastAlarm          = cause;
        xSemaphoreGive(mutex_);

        fsm_.toEmergencyStop(cause);

        Models::EventMessage evt {};
        evt.type        = Models::EventType::EMERGENCY_TRIGGERED;
        evt.timestampMs = now;
        evt.alarmCode   = static_cast<uint8_t>(cause);
        evt.alarmLevel  = static_cast<uint8_t>(Models::AlarmLevel::CRITICAL);
        eventBus_.publish(evt);
    }

    bool SafetyService::clearEmergency()
    {
        const uint32_t now = nowMsStatic();

        // El lockout debe haber expirado.
        xSemaphoreTake(mutex_, portMAX_DELAY);
        const bool stillLocked =
            emergencyActive_ && (now < emergencyLockoutUntilMs_);
        xSemaphoreGive(mutex_);

        if (stillLocked)
        {
            ESP_LOGW(TAG, "clearEmergency rejected: lockout still active");
            return false;
        }

        if (!fsm_.clearEmergency())
        {
            ESP_LOGW(TAG, "clearEmergency rejected by FSM");
            return false;
        }

        xSemaphoreTake(mutex_, portMAX_DELAY);
        emergencyActive_           = false;
        status_.inEmergencyLockout = false;
        status_.emergencyLockoutRemainingMs = 0;
        status_.lastAlarm          = Models::AlarmCode::EMERGENCY_CLEARED;
        xSemaphoreGive(mutex_);

        Models::EventMessage evt {};
        evt.type        = Models::EventType::EMERGENCY_CLEARED;
        evt.timestampMs = now;
        evt.alarmCode   = static_cast<uint8_t>(Models::AlarmCode::EMERGENCY_CLEARED);
        evt.alarmLevel  = static_cast<uint8_t>(Models::AlarmLevel::INFO);
        eventBus_.publish(evt);
        return true;
    }

    Models::SafetyStatus SafetyService::snapshot() const
    {
        if (mutex_ == nullptr) return status_;
        xSemaphoreTake(mutex_, portMAX_DELAY);
        Models::SafetyStatus copy = status_;
        xSemaphoreGive(mutex_);
        return copy;
    }

    void SafetyService::updateConfig(const Models::SafetyConfig& cfg)
    {
        xSemaphoreTake(mutex_, portMAX_DELAY);
        config_ = cfg;
        xSemaphoreGive(mutex_);
    }

    Models::SafetyConfig SafetyService::config() const
    {
        if (mutex_ == nullptr) return config_;
        xSemaphoreTake(mutex_, portMAX_DELAY);
        Models::SafetyConfig copy = config_;
        xSemaphoreGive(mutex_);
        return copy;
    }

    // ============================================================
    // Helpers privados
    // ============================================================
    bool SafetyService::checkSystemStateAllowsCommand(
        const Models::MotionCommand& cmd
    ) const
    {
        using S  = Models::SystemState;
        using MT = Models::MotionType;

        const auto st = fsm_.currentState();

        switch (st)
        {
            case S::INIT:
            case S::ERROR:
                return false;

            case S::CALIBRATING:
                // Sólo EMERGENCY_STOP en calibración (ya filtrado arriba).
                return false;

            case S::EMERGENCY_STOP:
                // Sólo CLEAR_EMERGENCY (también ya filtrado arriba).
                return false;

            case S::SAFE_MODE:
                // En safe mode sólo permitimos RETURN_HOME y emergencias.
                return cmd.type == MT::RETURN_HOME
                    || cmd.type == MT::HOME;

            case S::IDLE:
            case S::ACTIVE:
                return true;
        }
        return false;
    }

    bool SafetyService::checkCommandRanges(
        const Models::MotionCommand& cmd
    ) const
    {
        using MT = Models::MotionType;

        if (cmd.type == MT::CUSTOM_SERVO)
        {
            if (cmd.targetServo >=
                static_cast<uint8_t>(Models::JointId::COUNT))
            {
                return false;
            }
            if (cmd.targetAngle < 0.0f || cmd.targetAngle > 180.0f)
            {
                return false;
            }
        }

        if (cmd.speed < 0.0f || cmd.speed > 720.0f)
        {
            return false;
        }

        return true;
    }

    bool SafetyService::checkLockout(
        const Models::MotionCommand& cmd, uint32_t nowMs
    )
    {
        xSemaphoreTake(mutex_, portMAX_DELAY);
        const bool locked =
            emergencyActive_ && (nowMs < emergencyLockoutUntilMs_);
        xSemaphoreGive(mutex_);

        if (!locked) return true;

        // Durante el lockout solo CLEAR_EMERGENCY pasa (ya filtrado).
        // Todo lo demás se rechaza.
        (void)cmd;
        return false;
    }

    bool SafetyService::checkDeadtime(
        const Models::MotionCommand& cmd, uint32_t nowMs
    )
    {
        // Comandos críticos no tienen deadtime (CRITICAL bypasa).
        if (cmd.priority == Models::CommandPriority::CRITICAL)
        {
            return true;
        }

        xSemaphoreTake(mutex_, portMAX_DELAY);
        const bool sameType = (lastAccepted_.type == cmd.type);
        const uint32_t dt   = nowMs - lastAccepted_.ms;
        xSemaphoreGive(mutex_);

        if (sameType && dt < config_.commandDeadtimeMs)
        {
            return false;
        }
        return true;
    }

    void SafetyService::recordAcceptedCommand(
        const Models::MotionCommand& cmd, uint32_t nowMs
    )
    {
        xSemaphoreTake(mutex_, portMAX_DELAY);
        ++status_.totalCommandsValidated;
        lastAccepted_.type = cmd.type;
        lastAccepted_.ms   = nowMs;
        lastActivityMs_    = nowMs;
        xSemaphoreGive(mutex_);
    }

    void SafetyService::publishAlarm(
        Models::AlarmCode code,
        Models::AlarmLevel level,
        uint32_t nowMs
    )
    {
        xSemaphoreTake(mutex_, portMAX_DELAY);
        status_.lastAlarm = code;
        xSemaphoreGive(mutex_);

        Models::EventMessage evt {};
        evt.type        = Models::EventType::SAFETY_ALARM;
        evt.timestampMs = nowMs;
        evt.alarmCode   = static_cast<uint8_t>(code);
        evt.alarmLevel  = static_cast<uint8_t>(level);
        eventBus_.publish(evt);
    }
}