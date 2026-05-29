#include "services/SafetyService.hpp"

<<<<<<< HEAD
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_timer.h"

namespace Services
{
    static constexpr const char* TAG = "Safety";

    // ----------------------------------------------------------------------
    // Construcción / inicialización
    // ----------------------------------------------------------------------

    SafetyService::SafetyService(
        Interfaces::IMotionExecutor& motionExecutor,
        Core::EventBus& eventBus,
        const Models::SafetyConfig& config
    )
        : motionExecutor_(motionExecutor),
=======
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
>>>>>>> 0f18090e8f7bddf39123306abf466af425290d60
          eventBus_(eventBus),
          config_(config)
    {
    }

<<<<<<< HEAD
=======
    SafetyService::~SafetyService()
    {
        if (mutex_ != nullptr)
        {
            vSemaphoreDelete(mutex_);
            mutex_ = nullptr;
        }
    }

>>>>>>> 0f18090e8f7bddf39123306abf466af425290d60
    bool SafetyService::initialize()
    {
        mutex_ = xSemaphoreCreateMutex();
        if (mutex_ == nullptr)
        {
            ESP_LOGE(TAG, "Failed to create mutex");
            return false;
        }
<<<<<<< HEAD

        status_.state          = Models::SafetyState::NOMINAL;
        status_.activeFaults   = Models::SafetyFault::NONE;
        status_.latchedFaults  = Models::SafetyFault::NONE;
        status_.lastUpdateMs   = 0;
        status_.minFreeHeapBytes = heap_caps_get_free_size(MALLOC_CAP_DEFAULT);

        ESP_LOGI(TAG, "SafetyService ready");
        return true;
    }

    void SafetyService::update()
    {
        const uint32_t nowMs =
            static_cast<uint32_t>(esp_timer_get_time() / 1000ULL);

        if (mutex_ == nullptr) return;

        if (xSemaphoreTake(mutex_, pdMS_TO_TICKS(5)) != pdTRUE)
        {
            return;
        }

        // 1) Watchdog flag (set en ISR)
        if (wdtFlag_)
        {
            wdtFlag_ = false;
            raiseFault(Models::SafetyFault::WATCHDOG_TRIGGERED, nowMs);
            ESP_LOGE(TAG, "WATCHDOG TRIGGERED → forcing E-STOP");

            if (config_.watchdogTriggerEStop)
            {
                // Forzamos transición inmediata
                transitionTo(Models::SafetyState::EMERGENCY_STOP_HOLD, nowMs);
                motionExecutor_.stopAll();
            }
        }

        // 2) Salud de sensores
        scanSensorHealth(nowMs);

        // 3) Heap
        checkHeap(nowMs);

        // 4) Queue drops (windowed)
        if (nowMs - queueDropsResetMs_ > 5000)
        {
            if (queueDropsCount_ > config_.queueOverflowThreshold)
            {
                raiseFault(Models::SafetyFault::QUEUE_OVERFLOW, nowMs);
            }
            else
            {
                clearFault(Models::SafetyFault::QUEUE_OVERFLOW, nowMs);
            }
            queueDropsCount_   = 0;
            queueDropsResetMs_ = nowMs;
        }

        // 5) Decisión de FSM
        const auto target = decideTargetState();
        if (target != status_.state)
        {
            transitionTo(target, nowMs);
        }

        status_.lastUpdateMs = nowMs;

        xSemaphoreGive(mutex_);
    }

    // ----------------------------------------------------------------------
    // Registro de providers
    // ----------------------------------------------------------------------

    bool SafetyService::registerHealthProvider(
        Interfaces::ISensorHealthProvider* provider
    )
    {
        if (provider == nullptr) return false;
        if (providerCount_ >= MAX_SENSOR_PROVIDERS) return false;

        providers_[providerCount_++] = provider;
        return true;
    }

    // ----------------------------------------------------------------------
    // ISafetyMonitor — lectura de estado
    // ----------------------------------------------------------------------

    Models::SafetyStatus SafetyService::getStatus() const
    {
        Models::SafetyStatus copy {};

        if (mutex_ != nullptr &&
            xSemaphoreTake(mutex_, pdMS_TO_TICKS(5)) == pdTRUE)
        {
            copy = status_;
            xSemaphoreGive(mutex_);
        }
        return copy;
    }

    // ----------------------------------------------------------------------
    // ISafetyMonitor — política de comandos
    // ----------------------------------------------------------------------

    bool SafetyService::isCommandAllowed(
        const Models::MotionCommand& cmd
    ) const
    {
        // Lock corto: solo leemos.
        if (mutex_ == nullptr) return false;
        if (xSemaphoreTake(mutex_, pdMS_TO_TICKS(5)) != pdTRUE) return false;

        const auto state = status_.state;
        const auto active = status_.activeFaults;

        xSemaphoreGive(mutex_);

        // Regla 0: comando EMERGENCY_STOP siempre se acepta (es nuestro
        // mecanismo de seguridad, no podemos rechazarlo).
        if (cmd.type == Models::MotionType::EMERGENCY_STOP)
=======
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
>>>>>>> 0f18090e8f7bddf39123306abf466af425290d60
        {
            return true;
        }

<<<<<<< HEAD
        // Regla 0.5: lockout manual del usuario.
        // El operador presionó "lock": rechazar todo movimiento.
        // EMERGENCY_STOP ya pasó en la regla 0, así que sigue funcionando.
        if (userLocked_)
        {
            return false;
        }
        // Regla 1: FATAL = nada se permite
        if (state == Models::SafetyState::FATAL)
        {
            return false;
        }

        // Regla 2: en EMERGENCY_STOP_HOLD solo aceptamos CRITICAL provenientes
        // de fuentes autorizadas para recovery.
        if (state == Models::SafetyState::EMERGENCY_STOP_HOLD)
        {
            if (cmd.priority != Models::CommandPriority::CRITICAL)
            {
                return false;
            }
            return isRecoveryAllowedFrom(cmd.source);
        }

        // Regla 3: en STARTUP / RECOVERING aceptamos solo CRITICAL
        if (state == Models::SafetyState::STARTUP ||
            state == Models::SafetyState::RECOVERING)
        {
            return cmd.priority == Models::CommandPriority::CRITICAL;
        }

        // Regla 4: DEGRADED — política configurable
        if (state == Models::SafetyState::DEGRADED &&
            !config_.allowNormalCommandsInDegraded)
        {
            if (cmd.priority != Models::CommandPriority::CRITICAL &&
                cmd.priority != Models::CommandPriority::HIGH)
            {
                return false;
            }
        }

        // Regla 5: comandos derivados de sensores requieren que el sensor
        // esté sano.
        if (cmd.source == Models::CommandSource::IMU)
        {
            if (Models::hasFault(active, Models::SafetyFault::IMU_TIMEOUT) ||
                Models::hasFault(active, Models::SafetyFault::IMU_BUS_ERROR))
            {
                return false;
            }
        }

        if (cmd.source == Models::CommandSource::EMG)
        {
            if (Models::hasFault(active, Models::SafetyFault::EMG_TIMEOUT))
            {
                return false;
            }
        }

        return true;
    }

    // ----------------------------------------------------------------------
    // ISafetyMonitor — comandos
    // ----------------------------------------------------------------------

    void SafetyService::triggerEmergencyStop(
        Models::SafetyFault cause
    )
    {
        const uint32_t nowMs =
            static_cast<uint32_t>(esp_timer_get_time() / 1000ULL);

        ESP_LOGW(TAG, "E-STOP triggered, cause=0x%04X",
                 static_cast<unsigned>(cause));

        // Parada inmediata, sin tomar mutex todavía
        motionExecutor_.stopAll();

        if (mutex_ == nullptr) return;
        if (xSemaphoreTake(mutex_, pdMS_TO_TICKS(20)) != pdTRUE)
        {
            // Aún sin mutex, ya paramos los actuadores. La FSM se
            // actualizará en el próximo update().
            return;
        }

        status_.activeFaults  |= cause;
        status_.latchedFaults |= cause;
        ++status_.totalEmergencyStops;

        transitionTo(Models::SafetyState::EMERGENCY_STOP_HOLD, nowMs);

        xSemaphoreGive(mutex_);

        publishEvent(Models::EventType::EMERGENCY_TRIGGERED, nowMs);
    }

    bool SafetyService::requestRecovery()
    {
        const uint32_t nowMs =
            static_cast<uint32_t>(esp_timer_get_time() / 1000ULL);

        if (mutex_ == nullptr) return false;
        if (xSemaphoreTake(mutex_, pdMS_TO_TICKS(20)) != pdTRUE) return false;

        // Solo desde EMERGENCY_STOP_HOLD podemos pedir recovery.
        if (status_.state != Models::SafetyState::EMERGENCY_STOP_HOLD &&
            status_.state != Models::SafetyState::DEGRADED)
        {
            xSemaphoreGive(mutex_);
            return false;
        }

        // No iniciamos recovery si todavía hay faltas activas hardware
        // (sensor bus error, watchdog reciente). Estas deben limpiarse solas
        // o requieren reset.
        constexpr Models::SafetyFault HARD_FAULTS =
            Models::SafetyFault::WATCHDOG_TRIGGERED |
            Models::SafetyFault::IMU_BUS_ERROR      |
            Models::SafetyFault::INIT_FAILURE;

        if (Models::isAnyFault(status_.activeFaults & HARD_FAULTS))
        {
            xSemaphoreGive(mutex_);
            ESP_LOGW(TAG, "Recovery denied: hard faults still active");
            return false;
        }

        // Limpiamos faltas software (user-requested, rate-limit)
        constexpr Models::SafetyFault SOFT_FAULTS =
            Models::SafetyFault::USER_REQUESTED_ESTOP |
            Models::SafetyFault::COMMAND_RATE_EXCEEDED;

        status_.activeFaults = status_.activeFaults & ~SOFT_FAULTS;

        recoveryEnteredMs_ = nowMs;
        transitionTo(Models::SafetyState::RECOVERING, nowMs);

        xSemaphoreGive(mutex_);

        ESP_LOGI(TAG, "Recovery requested");
        return true;
    }

    void SafetyService::clearLatchedFaults()
    {
        if (mutex_ == nullptr) return;
        if (xSemaphoreTake(mutex_, pdMS_TO_TICKS(20)) != pdTRUE) return;

        status_.latchedFaults = Models::SafetyFault::NONE;
        xSemaphoreGive(mutex_);

        ESP_LOGI(TAG, "Latched faults cleared");
    }

    // ----------------------------------------------------------------------
    // Watchdog
    // ----------------------------------------------------------------------

    void SafetyService::onWatchdogTrigger()
    {
        // ATENCIÓN: este método se ejecuta en contexto de ISR.
        // Sólo seteamos un flag; el procesamiento real ocurre en update().
        wdtFlag_ = true;
    }

    // ----------------------------------------------------------------------
    // Telemetría externa
    // ----------------------------------------------------------------------

    void SafetyService::reportQueueDropped()
    {
        // Llamado posiblemente desde otra task; lectura/escritura atómica.
        ++queueDropsCount_;
    }

    // ----------------------------------------------------------------------
    // Rate limit
    // ----------------------------------------------------------------------

    bool SafetyService::checkRateLimit(
        Models::CommandSource src,
        uint32_t nowMs
    )
    {
        const size_t idx = static_cast<size_t>(src);
        if (idx >= MAX_SOURCES) return true;  // fuente desconocida, no limitamos

        auto& bucket = buckets_[idx];

        if (nowMs - bucket.windowStartMs > config_.rateLimitWindowMs)
        {
            bucket.windowStartMs = nowMs;
            bucket.count         = 0;
        }

        ++bucket.count;
        if (bucket.count > config_.rateLimitMaxCommands)
        {
            ++status_.rateLimitedCount;
=======
        xSemaphoreTake(mutex_, portMAX_DELAY);
        const bool sameType = (lastAccepted_.type == cmd.type);
        const uint32_t dt   = nowMs - lastAccepted_.ms;
        xSemaphoreGive(mutex_);

        if (sameType && dt < config_.commandDeadtimeMs)
        {
>>>>>>> 0f18090e8f7bddf39123306abf466af425290d60
            return false;
        }
        return true;
    }

<<<<<<< HEAD
    bool SafetyService::isRecoveryAllowedFrom(
        Models::CommandSource src
    ) const
    {
        switch (src)
        {
            case Models::CommandSource::CLI:
                return config_.acceptRecoveryFromCli;
            case Models::CommandSource::EMG:
                return config_.acceptRecoveryFromEmg;
            case Models::CommandSource::IMU:
                return config_.acceptRecoveryFromImu;
            case Models::CommandSource::NEXTION:
                return config_.acceptRecoveryFromNextion;
            case Models::CommandSource::BLE:
                return config_.acceptRecoveryFromBle;
            case Models::CommandSource::ROS2:
                return config_.acceptRecoveryFromRos2;
            default:
                return false;
        }
    }

    // ----------------------------------------------------------------------
    // Sensor health scanning
    // ----------------------------------------------------------------------

    void SafetyService::scanSensorHealth(uint32_t nowMs)
    {
        bool imuHealthy = true;
        bool emgHealthy = true;

        for (size_t i = 0; i < providerCount_; ++i)
        {
            auto* p = providers_[i];
            if (p == nullptr) continue;

            const auto kind = p->kind();

            if (!p->isPresent())
            {
                // No estar presente NO levanta falta por sí solo (modo
                // dev sin sensores). Pero invalida la sanidad para
                // comandos derivados de ese sensor.
                if (kind == Interfaces::SensorKind::IMU) imuHealthy = false;
                if (kind == Interfaces::SensorKind::EMG) emgHealthy = false;
                continue;
            }

            // Stale?
            const uint32_t last = p->lastValidSampleMs();
            const uint32_t threshold =
                (kind == Interfaces::SensorKind::IMU)
                ? config_.imuStaleThresholdMs
                : config_.emgStaleThresholdMs;

            // Solo si lleva produciendo datos (last > 0)
            const bool stale = (last > 0) && ((nowMs - last) > threshold);

            // Fault flag del propio sensor
            const bool hwFault = p->hasFault();

            if (kind == Interfaces::SensorKind::IMU)
            {
                if (stale || hwFault)
                {
                    if (hwFault)
                        raiseFault(Models::SafetyFault::IMU_BUS_ERROR, nowMs);
                    if (stale)
                        raiseFault(Models::SafetyFault::IMU_TIMEOUT, nowMs);
                    imuHealthy = false;
                }
                else
                {
                    clearFault(Models::SafetyFault::IMU_TIMEOUT, nowMs);
                    clearFault(Models::SafetyFault::IMU_BUS_ERROR, nowMs);

                    if (!p->isCalibrated())
                    {
                        raiseFault(Models::SafetyFault::IMU_NOT_CALIBRATED, nowMs);
                    }
                    else
                    {
                        clearFault(Models::SafetyFault::IMU_NOT_CALIBRATED, nowMs);
                    }
                }
            }
            else if (kind == Interfaces::SensorKind::EMG)
            {
                if (stale || hwFault)
                {
                    if (stale || hwFault)
                        raiseFault(Models::SafetyFault::EMG_TIMEOUT, nowMs);
                    emgHealthy = false;
                }
                else
                {
                    clearFault(Models::SafetyFault::EMG_TIMEOUT, nowMs);

                    if (!p->isCalibrated())
                    {
                        raiseFault(Models::SafetyFault::EMG_NOT_CALIBRATED, nowMs);
                    }
                    else
                    {
                        clearFault(Models::SafetyFault::EMG_NOT_CALIBRATED, nowMs);
                    }
                }
            }
        }

        status_.imuHealthy = imuHealthy;
        status_.emgHealthy = emgHealthy;
    }

    // ----------------------------------------------------------------------
    // Heap monitoring
    // ----------------------------------------------------------------------

    void SafetyService::checkHeap(uint32_t nowMs)
    {
        const uint32_t freeHeap =
            static_cast<uint32_t>(heap_caps_get_free_size(MALLOC_CAP_DEFAULT));

        if (freeHeap < status_.minFreeHeapBytes)
        {
            status_.minFreeHeapBytes = freeHeap;
        }

        if (freeHeap < config_.minFreeHeapBytes)
        {
            raiseFault(Models::SafetyFault::HEAP_LOW, nowMs);
            publishEvent(Models::EventType::HEAP_LOW, nowMs);
        }
        else
        {
            clearFault(Models::SafetyFault::HEAP_LOW, nowMs);
        }
    }

    // ----------------------------------------------------------------------
    // Decisión de FSM
    // ----------------------------------------------------------------------

    Models::SafetyState SafetyService::decideTargetState() const
    {
        const auto active = status_.activeFaults;

        // Faltas críticas que fuerzan E-STOP
        constexpr Models::SafetyFault CRITICAL_FAULTS =
            Models::SafetyFault::WATCHDOG_TRIGGERED |
            Models::SafetyFault::USER_REQUESTED_ESTOP |
            Models::SafetyFault::INIT_FAILURE;

        if (Models::isAnyFault(active & CRITICAL_FAULTS))
        {
            // E-STOP es estado terminal hasta requestRecovery() explícito.
            // Por eso solo lo elegimos si actualmente no estamos en
            // RECOVERING (porque RECOVERING ya limpió las soft faults).
            if (status_.state != Models::SafetyState::RECOVERING)
            {
                return Models::SafetyState::EMERGENCY_STOP_HOLD;
            }
        }

        // RECOVERING → si lleva config_.recoveryStabilityMs sin faltas,
        // pasamos a NOMINAL.
        if (status_.state == Models::SafetyState::RECOVERING)
        {
            if (!Models::isAnyFault(active))
            {
                const uint32_t nowMs = status_.lastUpdateMs;
                if (nowMs - recoveryEnteredMs_ > config_.recoveryStabilityMs)
                {
                    return Models::SafetyState::NOMINAL;
                }
            }
            return Models::SafetyState::RECOVERING;
        }

        // Si estamos en E-STOP y no hubo requestRecovery, permanecemos.
        if (status_.state == Models::SafetyState::EMERGENCY_STOP_HOLD)
        {
            return Models::SafetyState::EMERGENCY_STOP_HOLD;
        }

        // Faltas menores → DEGRADED
        constexpr Models::SafetyFault DEGRADING_FAULTS =
            Models::SafetyFault::IMU_TIMEOUT          |
            Models::SafetyFault::EMG_TIMEOUT          |
            Models::SafetyFault::IMU_NOT_CALIBRATED   |
            Models::SafetyFault::EMG_NOT_CALIBRATED   |
            Models::SafetyFault::QUEUE_OVERFLOW       |
            Models::SafetyFault::COMMAND_RATE_EXCEEDED|
            Models::SafetyFault::HEAP_LOW             |
            Models::SafetyFault::STACK_LOW            |
            Models::SafetyFault::IMU_BUS_ERROR        |
            Models::SafetyFault::SERVO_LIMIT_VIOLATION;

        if (Models::isAnyFault(active & DEGRADING_FAULTS))
        {
            return Models::SafetyState::DEGRADED;
        }

        return Models::SafetyState::NOMINAL;
    }

    // ----------------------------------------------------------------------
    // FSM transitions
    // ----------------------------------------------------------------------

    void SafetyService::transitionTo(
        Models::SafetyState newState,
        uint32_t nowMs
    )
    {
        if (status_.state == newState) return;

        ESP_LOGI(TAG, "FSM: %s → %s",
                 Models::safetyStateToString(status_.state),
                 Models::safetyStateToString(newState));

        if (newState == Models::SafetyState::DEGRADED)
        {
            ++status_.totalTransitionsToDeg;
        }
        if (status_.state == Models::SafetyState::RECOVERING &&
            newState == Models::SafetyState::NOMINAL)
        {
            ++status_.totalRecoveries;
            publishEvent(Models::EventType::SAFETY_RECOVERED, nowMs);
        }

        status_.state             = newState;
        status_.lastTransitionMs  = nowMs;

        publishEvent(Models::EventType::SAFETY_STATE_CHANGED, nowMs, &status_);
    }

    void SafetyService::raiseFault(
        Models::SafetyFault fault,
        uint32_t nowMs
    )
    {
        if (Models::hasFault(status_.activeFaults, fault))
        {
            return;   // ya estaba activa
        }

        status_.activeFaults  |= fault;
        status_.latchedFaults |= fault;
        status_.lastFaultTimestampMs = nowMs;

        ESP_LOGW(TAG, "Fault raised: 0x%04X",
                 static_cast<unsigned>(fault));

        publishEvent(Models::EventType::SAFETY_FAULT_RAISED, nowMs, &fault);
    }

    void SafetyService::clearFault(
        Models::SafetyFault fault,
        uint32_t nowMs
    )
    {
        if (!Models::hasFault(status_.activeFaults, fault))
        {
            return;
        }

        status_.activeFaults = status_.activeFaults & ~fault;

        ESP_LOGI(TAG, "Fault cleared: 0x%04X",
                 static_cast<unsigned>(fault));

        publishEvent(Models::EventType::SAFETY_FAULT_CLEARED, nowMs, &fault);
    }

    // ----------------------------------------------------------------------
    // Publicación de eventos
    // ----------------------------------------------------------------------

    void SafetyService::publishEvent(
        Models::EventType type,
        uint32_t nowMs,
        void* data
    )
    {
        Models::EventMessage evt {};
        evt.type        = type;
        evt.timestampMs = nowMs;
        evt.data        = data;
        eventBus_.publish(evt);
    }
    // ----------------------------------------------------------------------
    // User-controlled lockout (bloqueo manual)
    // ----------------------------------------------------------------------

    void SafetyService::setUserLock(bool locked)
    {
        userLocked_ = locked;

        if (locked)
        {
            // Si bloqueamos, paramos lo que esté en movimiento.
            motionExecutor_.stopAll();
            ESP_LOGW(TAG, "User LOCK engaged - motion disabled");
        }
        else
        {
            ESP_LOGI(TAG, "User LOCK released - motion enabled");
        }
    }

    bool SafetyService::isUserLocked() const
    {
        return userLocked_;
    }
=======
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
>>>>>>> 0f18090e8f7bddf39123306abf466af425290d60
}