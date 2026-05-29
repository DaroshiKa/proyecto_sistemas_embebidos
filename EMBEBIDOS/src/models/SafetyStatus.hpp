#pragma once

#include <cstdint>
<<<<<<< HEAD
#include "SafetyState.hpp"
#include "SafetyFault.hpp"
=======
#include "MotionTypes.hpp"
>>>>>>> 0f18090e8f7bddf39123306abf466af425290d60

namespace Models
{
    struct SafetyStatus
    {
<<<<<<< HEAD
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
=======
        SystemState systemState         { SystemState::INIT };

        bool        imuHealthy          { false };
        bool        emgHealthy          { false };

        bool        inEmergencyLockout  { false };
        uint32_t    emergencyLockoutRemainingMs { 0 };

        // Contadores acumulados
        uint32_t    totalCommandsValidated  { 0 };
        uint32_t    totalCommandsRejected   { 0 };
        uint32_t    totalLockoutRejections  { 0 };
        uint32_t    totalDeadtimeRejections { 0 };
        uint32_t    totalStateInvalidations { 0 };

        uint32_t    imuTimeouts        { 0 };
        uint32_t    emgTimeouts        { 0 };

        uint32_t    lastStateChangeMs  { 0 };
        AlarmCode   lastAlarm          { AlarmCode::NONE };
>>>>>>> 0f18090e8f7bddf39123306abf466af425290d60
    };
}