#pragma once

#include <cstdint>

namespace Utils
{
    class ThresholdDetector
    {
    public:
        ThresholdDetector(
            float thresholdOn  = 0.35f,
            float thresholdOff = 0.20f,
            uint16_t debounceMs = 30
        );

        void setThresholds(float onLevel, float offLevel);
        void setDebounce(uint16_t debounceMs);
        void reset();

        // Devuelve el estado tras debounce
        bool update(float value, uint32_t nowMs);

        bool active() const { return active_; }

    private:
        float    onLevel_   { 0.35f };
        float    offLevel_  { 0.20f };
        uint16_t debounceMs_{ 30 };

        bool     candidate_     { false };
        bool     active_        { false };
        uint32_t candidateSince_{ 0 };
    };
}