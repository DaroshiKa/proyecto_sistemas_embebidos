#include "core/SystemStateMachine.hpp"

#include "esp_log.h"
#include "esp_timer.h"

namespace Core
{
    static constexpr const char* TAG = "StateMachine";

    static uint32_t nowMs()
    {
        return static_cast<uint32_t>(esp_timer_get_time() / 1000ULL);
    }

    SystemStateMachine::SystemStateMachine(EventBus& eventBus)
        : eventBus_(eventBus)
    {
    }

    SystemStateMachine::~SystemStateMachine()
    {
        if (mutex_ != nullptr)
        {
            vSemaphoreDelete(mutex_);
            mutex_ = nullptr;
        }
    }

    bool SystemStateMachine::initialize()
    {
        mutex_ = xSemaphoreCreateMutex();
        if (mutex_ == nullptr)
        {
            ESP_LOGE(TAG, "Failed to create mutex");
            return false;
        }
        state_ = Models::SystemState::INIT;
        lastTransitionMs_ = nowMs();
        return true;
    }

    Models::SystemState SystemStateMachine::currentState() const
    {
        if (mutex_ == nullptr) return state_;

        xSemaphoreTake(mutex_, portMAX_DELAY);
        const auto s = state_;
        xSemaphoreGive(mutex_);
        return s;
    }

    uint32_t SystemStateMachine::lastTransitionMs() const
    {
        if (mutex_ == nullptr) return lastTransitionMs_;

        xSemaphoreTake(mutex_, portMAX_DELAY);
        const auto t = lastTransitionMs_;
        xSemaphoreGive(mutex_);
        return t;
    }

    bool SystemStateMachine::bootComplete()
    {
        return transitionTo(Models::SystemState::IDLE, Models::AlarmCode::NONE);
    }

    bool SystemStateMachine::toActive()
    {
        return transitionTo(Models::SystemState::ACTIVE, Models::AlarmCode::NONE);
    }

    bool SystemStateMachine::toIdle()
    {
        return transitionTo(Models::SystemState::IDLE, Models::AlarmCode::NONE);
    }

    bool SystemStateMachine::toCalibrating()
    {
        return transitionTo(
            Models::SystemState::CALIBRATING, Models::AlarmCode::NONE);
    }

    bool SystemStateMachine::toSafeMode(Models::AlarmCode cause)
    {
        return transitionTo(Models::SystemState::SAFE_MODE, cause);
    }

    bool SystemStateMachine::toEmergencyStop(Models::AlarmCode cause)
    {
        return transitionTo(Models::SystemState::EMERGENCY_STOP, cause);
    }

    bool SystemStateMachine::clearEmergency()
    {
        // Solo puede limpiar emergencia si está en EMERGENCY_STOP.
        if (currentState() != Models::SystemState::EMERGENCY_STOP)
        {
            return false;
        }
        return transitionTo(
            Models::SystemState::IDLE, Models::AlarmCode::EMERGENCY_CLEARED);
    }

    bool SystemStateMachine::exitSafeMode()
    {
        if (currentState() != Models::SystemState::SAFE_MODE)
        {
            return false;
        }
        return transitionTo(
            Models::SystemState::IDLE, Models::AlarmCode::SAFE_MODE_EXITED);
    }

    bool SystemStateMachine::toError(Models::AlarmCode cause)
    {
        return transitionTo(Models::SystemState::ERROR, cause);
    }

    bool SystemStateMachine::isLegalTransition(
        Models::SystemState from,
        Models::SystemState to
    ) const
    {
        using S = Models::SystemState;

        // ERROR es absorbente: una vez ahí, sólo reset físico.
        if (from == S::ERROR) return false;

        // Cualquiera -> ERROR siempre es legal.
        if (to == S::ERROR) return true;

        // Cualquiera (excepto INIT->X salvo IDLE) -> EMERGENCY_STOP es legal.
        if (to == S::EMERGENCY_STOP)
        {
            return from != S::INIT;
        }

        // Cualquiera (excepto INIT/EMERGENCY) -> SAFE_MODE es legal.
        if (to == S::SAFE_MODE)
        {
            return from != S::INIT && from != S::EMERGENCY_STOP;
        }

        switch (from)
        {
            case S::INIT:
                return to == S::IDLE;

            case S::IDLE:
                return to == S::ACTIVE
                    || to == S::CALIBRATING;

            case S::ACTIVE:
                return to == S::IDLE
                    || to == S::CALIBRATING;

            case S::CALIBRATING:
                return to == S::IDLE;

            case S::SAFE_MODE:
                return to == S::IDLE;       // vía exitSafeMode()

            case S::EMERGENCY_STOP:
                return to == S::IDLE;       // vía clearEmergency()

            default:
                return false;
        }
    }

    bool SystemStateMachine::transitionTo(
        Models::SystemState newState,
        Models::AlarmCode cause
    )
    {
        if (mutex_ == nullptr) return false;

        xSemaphoreTake(mutex_, portMAX_DELAY);

        // Sin cambio: éxito silencioso.
        if (state_ == newState)
        {
            xSemaphoreGive(mutex_);
            return true;
        }

        if (!isLegalTransition(state_, newState))
        {
            ESP_LOGW(
                TAG, "Illegal transition %u -> %u",
                static_cast<unsigned>(state_),
                static_cast<unsigned>(newState)
            );
            xSemaphoreGive(mutex_);
            return false;
        }

        const auto previous = state_;
        state_ = newState;
        lastTransitionMs_ = nowMs();

        xSemaphoreGive(mutex_);

        ESP_LOGI(
            TAG, "State %u -> %u (cause=%u)",
            static_cast<unsigned>(previous),
            static_cast<unsigned>(newState),
            static_cast<unsigned>(cause)
        );

        publishStateChanged(newState, cause);
        return true;
    }

    void SystemStateMachine::publishStateChanged(
        Models::SystemState newState,
        Models::AlarmCode cause
    )
    {
        Models::EventMessage evt {};
        evt.type        = Models::EventType::SYSTEM_STATE_CHANGED;
        evt.timestampMs = nowMs();
        evt.newState    = static_cast<uint8_t>(newState);
        evt.alarmCode   = static_cast<uint8_t>(cause);
        evt.alarmLevel  = static_cast<uint8_t>(Models::AlarmLevel::INFO);

        eventBus_.publish(evt);
    }
}