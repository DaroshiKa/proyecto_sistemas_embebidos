#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include "core/EventBus.hpp"
#include "core/TaskHeartbeat.hpp"

#include "interfaces/IIMUSource.hpp"
#include "interfaces/IEMGSource.hpp"
#include "interfaces/IMotionExecutor.hpp"
#include "interfaces/ICommandDispatcher.hpp"
#include "interfaces/ISafetyValidator.hpp"

#include "models/SafetyConfig.hpp"
#include "models/MotionTypes.hpp"
#include "models/AlarmState.hpp"

namespace Core
{
    struct SafetyDependencies
    {
        EventBus*                       eventBus    { nullptr };
        Interfaces::IIMUSource*         imu         { nullptr };
        Interfaces::IEMGSource*         emg         { nullptr };
        Interfaces::IMotionExecutor*    executor    { nullptr };
        Interfaces::ICommandDispatcher* dispatcher  { nullptr };
        Interfaces::ISafetyValidator*   validator   { nullptr };

        // Heartbeats de las tasks vigiladas
        TaskHeartbeat*                  motionHb    { nullptr };
        TaskHeartbeat*                  imuHb       { nullptr };
        TaskHeartbeat*                  emgHb       { nullptr };
    };

    class SafetyMonitor
    {
    public:
        SafetyMonitor(
            const SafetyDependencies& deps,
            const Models::SafetyConfig& config
        );

        bool initialize();

        // Ejecutado periódicamente por TaskSafety
        void tick();

        // ---- Operaciones explícitas (CLI/Nextion) ----

        // Limpia un estado de emergencia. Vuelve a IDLE si las condiciones
        // de salud son correctas; si no, va a ERROR.
        bool clearEmergency();

        // Fuerza el modo seguro (desde diagnóstico)
        void enterSafeMode();

        // Estado y diagnóstico
        Models::SystemState currentState() const { return systemState_; }
        bool isEmergencyActive() const
        {
            return systemState_ == Models::SystemState::EMERGENCY_STOP;
        }

        // Stats
        uint32_t totalEmergencies()      const { return totalEmergencies_; }
        uint32_t totalRecoveries()       const { return totalRecoveries_; }
        uint32_t totalTaskStalls()       const { return totalTaskStalls_; }
        uint32_t totalSensorTimeouts()   const { return totalSensorTimeouts_; }

    private:
        // Transiciones
        void transitionTo(
            Models::SystemState newState,
            Models::AlarmCode reason
        );

        // Disparo de emergencia: encola comando CRITICAL
        void triggerEmergency(Models::AlarmCode reason);

        // Chequeos individuales
        void checkTaskHeartbeats();
        void checkSensorHealth();
        void evaluateAutoRecovery();

        // Helpers
        void publishAlarm(
            Models::AlarmLevel level,
            Models::AlarmCode code,
            const char* message
        );

        SafetyDependencies   deps_;
        Models::SafetyConfig config_;

        Models::SystemState  systemState_ { Models::SystemState::INIT };

        // Latches de salud (para detectar transiciones)
        bool prevImuOk_ { true };
        bool prevEmgOk_ { true };

        uint32_t enteredErrorMs_ { 0 };

        // Stats
        uint32_t totalEmergencies_     { 0 };
        uint32_t totalRecoveries_      { 0 };
        uint32_t totalTaskStalls_      { 0 };
        uint32_t totalSensorTimeouts_  { 0 };

        // Mutex para acceso concurrente al estado (CLI lee, monitor escribe)
        SemaphoreHandle_t mutex_ { nullptr };
    };
}