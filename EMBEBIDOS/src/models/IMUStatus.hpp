#pragma once

#include <cstdint>

namespace Models
{
    enum class IMUState : uint8_t
    {
        UNINITIALIZED = 0,
        INITIALIZING,
        CALIBRATING,
        OK,
        TIMEOUT,
        BUS_ERROR,
        FAULT
    };

    struct IMUStatus
    {
        IMUState  state             { IMUState::UNINITIALIZED };
        bool      calibrated        { false };
        uint32_t  consecutiveFails  { 0 };
        uint32_t  totalSamples      { 0 };
        uint32_t  totalErrors       { 0 };
        uint32_t  lastUpdateMs      { 0 };
    };
}