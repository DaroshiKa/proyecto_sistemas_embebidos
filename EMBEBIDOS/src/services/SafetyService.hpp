#pragma once

#include <array>
#include <cstdint>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include "interfaces/IService.hpp"
#include "interfaces/ISafetyMonitor.hpp"
#include "interfaces/ISensorHealthProvider.hpp"
#include "interfaces/IMotionExecutor.hpp"

#include "core/EventBus.hpp"

#include "models/SafetyConfig.hpp"
#include "models/SafetyStatus.hpp"
#include "models/MotionCommand.hpp"
#include "models/MotionTypes.hpp"

namespace Services
{
    class SafetyService final :
        public Interfaces::IService,
        public Interfaces::ISafetyMonitor
    {
    public:
        // Máximo de sensor-health providers que el sistema soporta.
        // Para nuestra mano robótica: IMU + EMG. Reservamos slots
        // para sensores futuros (fuerza, temperatura).
        static constexpr size_t MAX_SENSOR_PROVIDERS = 8;

        // CommandSource::COUNT no existe; usamos un máximo discreto que
        // cubre el enum.
        static constexpr size_t MAX_SOURCES = 8;

        SafetyService(
            Interfaces::IMotionExecutor& motionExecutor,
            Core::EventBus& eventBus,
            const Models::SafetyConfig& config
        );

        // IService
        bool initialize() override;
        void update() override;   // llamado por TaskSafety @ 20 Hz

        // ISafetyMonitor
        Models::SafetyStatus getStatus() const override;
        bool isCommandAllowed(
            const Models::MotionCommand& cmd
        ) const override;
        void triggerEmergencyStop(
            Models::SafetyFault cause = Models::SafetyFault::USER_REQUESTED_ESTOP
        ) override;
        bool requestRecovery() override;
        void clearLatchedFaults() override;

        // Registro de sensor-health providers (DI manual).
        // Devuelve false si ya no hay slots.
        bool registerHealthProvider(
            Interfaces::ISensorHealthProvider* provider
        );

        // Hook del watchdog: invocado desde
        // esp_task_wdt_isr_user_handler vía WatchdogManager.
        // SOLO setea un flag (corre en ISR).
        void onWatchdogTrigger();

        // Telemetría / hooks para módulos externos
        void reportQueueDropped();             // CommandDispatcher → SafetyService

    private:
        // FSM transitions
        void transitionTo(
            Models::SafetyState newState,
            uint32_t nowMs
        );

        void raiseFault(
            Models::SafetyFault fault,
            uint32_t nowMs
        );

        void clearFault(
            Models::SafetyFault fault,
            uint32_t nowMs
        );

        // Decisión periódica del estado a partir de las faltas
        Models::SafetyState decideTargetState() const;

        // Helpers
        void scanSensorHealth(uint32_t nowMs);
        void checkHeap(uint32_t nowMs);
        bool isRecoveryAllowedFrom(Models::CommandSource src) const;
        bool checkRateLimit(Models::CommandSource src, uint32_t nowMs);
        void publishEvent(
            Models::EventType type,
            uint32_t nowMs,
            void* data = nullptr
        );

        // ---------- Dependencies ----------
        Interfaces::IMotionExecutor& motionExecutor_;
        Core::EventBus&              eventBus_;
        Models::SafetyConfig         config_;

        // ---------- Providers ----------
        std::array<Interfaces::ISensorHealthProvider*, MAX_SENSOR_PROVIDERS>
                                     providers_   {};
        size_t                       providerCount_ { 0 };

        // ---------- Estado ----------
        mutable SemaphoreHandle_t    mutex_       { nullptr };
        Models::SafetyStatus         status_      {};

        // Watchdog flag (set in ISR, cleared in update())
        volatile bool                wdtFlag_     { false };

        // Rate limit por fuente
        struct RateBucket
        {
            uint32_t windowStartMs { 0 };
            uint16_t count         { 0 };
        };
        mutable std::array<RateBucket, MAX_SOURCES> buckets_ {};

        // Bookkeeping de queue drops
        uint32_t queueDropsCount_ { 0 };
        uint32_t queueDropsResetMs_ { 0 };

        // Tiempo cuando se entró a RECOVERING (para esperar estabilidad)
        uint32_t recoveryEnteredMs_ { 0 };
    };
}