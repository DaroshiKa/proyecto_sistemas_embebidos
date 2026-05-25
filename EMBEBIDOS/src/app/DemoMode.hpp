#pragma once

#include <cstdint>
#include <cstddef>

#include "interfaces/ICommandDispatcher.hpp"
#include "models/MotionCommand.hpp"

namespace App
{
    class DemoMode
    {
    public:
        explicit DemoMode(Interfaces::ICommandDispatcher& dispatcher);

        void start(uint32_t nowMs);
        void stop();

        // Llamar periódicamente (ej. cada 200 ms desde TaskCLI).
        // Avanza los pasos según el reloj.
        void tick(uint32_t nowMs);

        bool isRunning() const { return running_; }

    private:
        struct Step
        {
            Models::MotionType type;
            uint32_t           durationMs;
        };

        void dispatchStep(const Step& s, uint32_t nowMs);

        Interfaces::ICommandDispatcher& dispatcher_;

        // Coreografía: secuencia de movimientos
        static constexpr Step SEQUENCE_[] = {
            { Models::MotionType::HOME,       1500 },
            { Models::MotionType::HAND_OPEN,  1200 },
            { Models::MotionType::WRIST_LEFT, 1200 },
            { Models::MotionType::WRIST_RIGHT,1200 },
            { Models::MotionType::HAND_CLOSE, 1200 },
            { Models::MotionType::ELBOW_XZ,   1500 },
            { Models::MotionType::ELBOW_YZ,   1500 },
            { Models::MotionType::ELBOW_XY,   1500 },
            { Models::MotionType::HAND_OPEN,  1200 },
            { Models::MotionType::HOME,       1500 }
        };

        static constexpr size_t SEQUENCE_LEN_ =
            sizeof(SEQUENCE_) / sizeof(SEQUENCE_[0]);

        bool      running_      { false };
        size_t    stepIndex_    { 0 };
        uint32_t  stepStartMs_  { 0 };
    };
}