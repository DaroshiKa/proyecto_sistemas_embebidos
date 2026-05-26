#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include "interfaces/ISafetyValidator.hpp"
#include "interfaces/ISensorHealthSource.hpp"

#include "core/SystemStateMachine.hpp"
#include "core/EventBus.hpp"

#include "models/SafetyConfig.hpp"
#include "models/SafetyStatus.hpp"
#include "models/MotionCommand.hpp"

namespace Services
{
    class SafetyService final :
        public Interfaces::ISafetyValidator
    {
    public:
        SafetyService(
            Core::SystemStateMachine& fsm,
            Core::EventBus& eventBus,
            const Models::SafetyConfig& config = Models::SafetyConfig{}
        );

        ~SafetyService();

        bool initialize();

        // ============ ISafetyValidator ============
        // Es invocado por CommandDispatcher en cada dispatch().
        bool validate(const Models::MotionCommand& command) override;

        // ============ Fuentes de salud de sensores ============
        // Inyección opcional (el sistema puede correr sin sensores).
        void attachImuHealth(Interfaces::ISensorHealthSource* src)
        {
            imuHealth_ = src;
        }
        void attachEmgHealth(Interfaces::ISensorHealthSource* src)
        {
            emgHealth_ = src;
        }

        // ============ Tick periódico (lo invoca TaskSafety) ============
        // Comprueba timeouts de sensores, deadlines de lockout, idle timeout.
        // Publica eventos al EventBus y transiciona la FSM cuando corresponda.
        void tick(uint32_t nowMs);

        // ============ API explícita ============
        // Llamada cuando un MotionCommand de tipo EMERGENCY_STOP llega
        // o cuando un trigger físico se dispara.
        void triggerEmergencyStop(Models::AlarmCode cause);
        bool clearEmergency();

        // Para CLI / Nextion / telemetría
        Models::SafetyStatus snapshot() const;

        // Reconfigurar en runtime (CLI)
        void updateConfig(const Models::SafetyConfig& cfg);
        Models::SafetyConfig config() const;

    private:
        // ---- helpers de validación (todos toman el mutex internamente) ----
        bool checkSystemStateAllowsCommand(
            const Models::MotionCommand& cmd
        ) const;

        bool checkCommandRanges(const Models::MotionCommand& cmd) const;

        bool checkLockout(const Models::MotionCommand& cmd, uint32_t nowMs);
        bool checkDeadtime(const Models::MotionCommand& cmd, uint32_t nowMs);

        void recordAcceptedCommand(
            const Models::MotionCommand& cmd, uint32_t nowMs
        );

        void publishAlarm(
            Models::AlarmCode code, Models::AlarmLevel level, uint32_t nowMs
        );

        Core::SystemStateMachine&    fsm_;
        Core::EventBus&              eventBus_;
        Models::SafetyConfig         config_;
        Models::SafetyStatus         status_;

        Interfaces::ISensorHealthSource* imuHealth_ { nullptr };
        Interfaces::ISensorHealthSource* emgHealth_ { nullptr };

        // Lockouts internos
        uint32_t emergencyLockoutUntilMs_ { 0 };
        bool     emergencyActive_         { false };

        // Deadtime tracking (último comando aceptado por tipo)
        struct LastCmd
        {
            Models::MotionType type { Models::MotionType::NONE };
            uint32_t           ms   { 0 };
        };
        LastCmd lastAccepted_;

        // Último timestamp de actividad para idle timeout
        uint32_t lastActivityMs_ { 0 };

        // Tracking de salud de sensores (para detectar cambio OK->TIMEOUT)
        bool imuWasHealthy_ { true };
        bool emgWasHealthy_ { true };

        mutable SemaphoreHandle_t mutex_ { nullptr };
    };
}