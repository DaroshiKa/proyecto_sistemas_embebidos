#pragma once

#include <cstdint>
#include "MotionTypes.hpp"

namespace Models
{
    struct SafetyConfig
    {
        // ---------- Watchdog ----------
        uint32_t watchdogTimeoutMs       { 3000 };
        bool     watchdogPanicOnTrigger  { false };   // true => esp_restart()
        bool     watchdogTriggerEStop    { true };    // si una tarea no alimenta,
                                                      // ¿ir a E-Stop?

        // ---------- Sensor health ----------
        // El SafetyService considera que un sensor está sano si
        // pasaron menos de N ms desde su última muestra válida.
        uint32_t imuStaleThresholdMs     { 250 };
        uint32_t emgStaleThresholdMs     { 250 };

        // ---------- Recovery ----------
        // Tiempo sin faltas activas que se requiere antes de pasar de
        // RECOVERING → NOMINAL
        uint32_t recoveryStabilityMs     { 3000 };

        // ---------- Heap ----------
        uint32_t minFreeHeapBytes        { 16 * 1024 };  // 16 KB
        bool     heapLowTriggerDegraded  { true };

        // ---------- Rate limit (anti-spam) ----------
        // Por cada CommandSource: máximo N comandos en ventana de M ms.
        uint16_t rateLimitMaxCommands    { 20 };
        uint32_t rateLimitWindowMs       { 1000 };

        // ---------- Queue overflow ----------
        // Si el dispatcher reporta más de N dropped en un periodo,
        // se cuenta como QUEUE_OVERFLOW.
        uint32_t queueOverflowThreshold  { 5 };

        // ---------- Política de comandos ----------
        // En estado EMERGENCY_STOP_HOLD, ¿qué fuentes pueden enviar
        // un comando CRITICAL para forzar recovery?
        // Por defecto solo CLI (operador físico).
        bool acceptRecoveryFromCli       { true };
        bool acceptRecoveryFromEmg       { false };
        bool acceptRecoveryFromImu       { false };
        bool acceptRecoveryFromNextion   { true };
        bool acceptRecoveryFromBle       { false };
        bool acceptRecoveryFromRos2      { false };

        // ---------- E-Stop policy ----------
        // ¿Permitir que comandos NORMAL/HIGH lleguen al motion service
        // cuando el sistema está en DEGRADED?
        bool allowNormalCommandsInDegraded { true };

        // ---------- Periodo de la TaskSafety ----------
        uint32_t taskPeriodMs            { 50 };  // 20 Hz
    };
}