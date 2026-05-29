#pragma once

<<<<<<< HEAD
#include <array>
#include <cstdint>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include "interfaces/IService.hpp"
#include "interfaces/ISafetyMonitor.hpp"
#include "interfaces/ISensorHealthProvider.hpp"
#include "interfaces/IMotionExecutor.hpp"

=======
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include "interfaces/ISafetyValidator.hpp"
#include "interfaces/ISensorHealthSource.hpp"

#include "core/SystemStateMachine.hpp"
>>>>>>> 0f18090e8f7bddf39123306abf466af425290d60
#include "core/EventBus.hpp"

#include "models/SafetyConfig.hpp"
#include "models/SafetyStatus.hpp"
#include "models/MotionCommand.hpp"
<<<<<<< HEAD
#include "models/MotionTypes.hpp"
=======
>>>>>>> 0f18090e8f7bddf39123306abf466af425290d60

namespace Services
{
    class SafetyService final :
<<<<<<< HEAD
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
        // Bloqueo manual del operador. Mientras esté activo, NINGÚN comando
        // de movimiento se permite (excepto EMERGENCY_STOP). No cambia la
        // FSM ni levanta faltas: es un override del usuario.
        void setUserLock(bool locked);
        bool isUserLocked() const;

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
        
        volatile bool userLocked_ { false };
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
=======
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
>>>>>>> 0f18090e8f7bddf39123306abf466af425290d60
    };
}