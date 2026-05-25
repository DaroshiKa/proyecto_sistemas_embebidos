#pragma once

#include "interfaces/IService.hpp"
#include "interfaces/IMotionExecutor.hpp"

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
        void update() override;   // no usado en esta etapa; lo lleva TaskMotion

        // Procesa un comando high-level proveniente del dispatcher.
        bool processMotionCommand(
            const Models::MotionCommand& command,
            uint32_t nowMs
        );

        // Telemetría
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
        bool dispatchEmergencyStop();

        void publishExecuted(const Models::MotionCommand& cmd, uint32_t now);

        Interfaces::IMotionExecutor& executor_;
        Core::EventBus&              eventBus_;
        Models::MotionConfig         config_;

        uint32_t totalExecuted_ { 0 };
        uint32_t totalIgnored_  { 0 };
    };
}