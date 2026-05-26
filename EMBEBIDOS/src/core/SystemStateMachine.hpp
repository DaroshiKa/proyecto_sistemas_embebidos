#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include "models/MotionTypes.hpp"
#include "models/EventMessage.hpp"
#include "core/EventBus.hpp"

namespace Core
{
    // FSM global del sistema. Es thread-safe por mutex interno.
    // Es la única autoridad sobre Models::SystemState.
    class SystemStateMachine
    {
    public:
        explicit SystemStateMachine(EventBus& eventBus);
        ~SystemStateMachine();

        bool initialize();

        Models::SystemState currentState() const;

        // Transiciones explícitas. Cada una valida la transición; si no es
        // legal, devuelve false sin cambiar el estado. Publica
        // SYSTEM_STATE_CHANGED en el EventBus cuando hay cambio.
        bool bootComplete();
        bool toActive();
        bool toIdle();
        bool toCalibrating();
        bool toSafeMode(Models::AlarmCode cause);
        bool toEmergencyStop(Models::AlarmCode cause);
        bool clearEmergency();
        bool exitSafeMode();
        bool toError(Models::AlarmCode cause);

        // Diagnóstico
        uint32_t lastTransitionMs() const;

    private:
        bool transitionTo(
            Models::SystemState newState,
            Models::AlarmCode cause
        );

        bool isLegalTransition(
            Models::SystemState from,
            Models::SystemState to
        ) const;

        void publishStateChanged(
            Models::SystemState newState,
            Models::AlarmCode cause
        );

        EventBus&            eventBus_;
        Models::SystemState  state_           { Models::SystemState::INIT };
        uint32_t             lastTransitionMs_ { 0 };
        SemaphoreHandle_t    mutex_           { nullptr };
    };
}