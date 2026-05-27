#pragma once

#include "interfaces/IService.hpp"
#include "interfaces/IMotionExecutor.hpp"
#include "interfaces/ISafetyMonitor.hpp"

#include "core/EventBus.hpp"

#include "models/MotionCommand.hpp"
#include "models/MotionConfig.hpp"

namespace Services
{
    class MotionService final :
        public Interfaces::IService
    {
    public:
        MotionService(
            Interfaces::IMotionExecutor& executor,
            Core::EventBus& eventBus,
            const Models::MotionConfig& config
        );

        bool initialize() override;
        void update() override;

        // Inyección opcional: si está presente, los EMERGENCY_STOP
        // se reportan al monitor para que entre en E-STOP_HOLD.
        void attachSafetyMonitor(Interfaces::ISafetyMonitor* monitor);

        bool processMotionCommand(
            const Models::MotionCommand& command,
            uint32_t nowMs
        );

        uint32_t totalExecuted() const { return totalExecuted_; }
        uint32_t totalIgnored()  const { return totalIgnored_; }

    private:
        bool dispatchHandOpen(uint32_t now);
        bool dispatchHandClose(uint32_t now);
        bool dispatchWristLeft(uint32_t now);
        bool dispatchWristRight(uint32_t now);
        bool dispatchElbowPlane(Models::MotionType plane, uint32_t now);
        bool dispatchHome(uint32_t now);
        bool dispatchCustomServo(const Models::MotionCommand& cmd, uint32_t now);
        bool dispatchEmergencyStop(const Models::MotionCommand& cmd);

        void publishExecuted(const Models::MotionCommand& cmd, uint32_t now);

        Interfaces::IMotionExecutor& executor_;
        Core::EventBus&              eventBus_;
        Models::MotionConfig         config_;
        Interfaces::ISafetyMonitor*  safetyMonitor_ { nullptr };

        uint32_t totalExecuted_ { 0 };
        uint32_t totalIgnored_  { 0 };
    };
}