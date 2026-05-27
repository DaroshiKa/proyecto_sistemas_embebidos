#pragma once

#include <cstdint>
#include "SafetyState.hpp"
#include "SafetyFault.hpp"

namespace Models
{
    struct SafetyStatus
    {
        SafetyState state          { SafetyState::STARTUP };

        SafetyFault activeFaults   { SafetyFault::NONE };
        SafetyFault latchedFaults  { SafetyFault::NONE };
        //  ^^ las latched permanecen hasta que el usuario haga "safety reset",
        //     aunque la causa raíz ya se haya limpiado. Indispensable para
        //     auditoría/forensics.

        // Contadores de transiciones (para diagnóstico)
        uint32_t totalCommandsRejected   { 0 };
        uint32_t totalEmergencyStops     { 0 };
        uint32_t totalTransitionsToDeg   { 0 };
        uint32_t totalRecoveries         { 0 };

        // Métricas de tiempo
        uint32_t lastFaultTimestampMs    { 0 };
        uint32_t lastTransitionMs        { 0 };
        uint32_t lastUpdateMs            { 0 };

        // Sensor health (cached para que CLI no tenga que tomar mutexes
        // de IMUService/EMGService)
        bool     imuHealthy              { false };
        bool     emgHealthy              { false };

        // Rate-limit telemetry
        uint32_t rateLimitedCount        { 0 };

        // Heap monitoring
        uint32_t minFreeHeapBytes        { 0xFFFFFFFFu };
    };
}