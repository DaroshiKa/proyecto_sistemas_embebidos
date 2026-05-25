#pragma once

#include <cstdint>

#include "models/SensorData.hpp"

namespace Models
{
    struct EMGStatus
    {
        EMGState  state             { EMGState::UNINITIALIZED };
        bool      calibrated        { false };
        float     baselineLevel     { 0.0f };
        float     peakLevel         { 0.0f };
        uint32_t  consecutiveFails  { 0 };
        uint32_t  totalSamples      { 0 };
        uint32_t  totalErrors       { 0 };
        uint32_t  totalGestures     { 0 };
        uint32_t  lastUpdateMs      { 0 };
    };
}