#pragma once

#include <cstdint>

#include "models/EMGConfig.hpp"
#include "models/SensorData.hpp"

namespace Utils
{
    class GestureFSM
    {
    public:
        GestureFSM();

        void configure(const Models::EMGConfig& cfg);

        void reset();

        // Devuelve un gesto nuevo si la transición lo produce (RELAX, SINGLE,
        // DOUBLE, LONG_HOLD). NONE si no hay novedad.
        Models::EMGGesture update(
            bool active,
            uint32_t nowMs
        );

    private:
        enum class State : uint8_t
        {
            IDLE = 0,
            CONTRACTION,           // 1ª contracción detectada
            AFTER_CONTRACTION,     // esperando posible 2ª pulsación
            LONG_HOLD_FIRED        // ya disparamos LONG_HOLD, esperando relajación
        };

        State    state_ { State::IDLE };
        uint32_t stateSince_ { 0 };
        uint32_t lastRelaxFiredAt_ { 0 };

        uint16_t doublePulseWindowMs_ { 400 };
        uint16_t singlePulseMinMs_    { 80 };
        uint16_t longHoldMs_          { 2000 };
        uint16_t relaxMs_             { 500 };
    };
}