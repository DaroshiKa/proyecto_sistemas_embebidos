#pragma once

#include <cstdint>
#include <cstddef>

namespace Models
{
    // Snapshot inmutable de métricas del sistema. Producido por
    // TaskDiagnostics y consumido por CLI / futuras integraciones
    // (Nextion, BLE, ROS2). Es POD para evitar costos de copia y
    // alocación dinámica.

    struct TaskMetric
    {
        const char* name             { nullptr };
        uint32_t    stackHighWater   { 0 };    // palabras (4 bytes c/u)
        uint32_t    runtimePercent   { 0 };    // % CPU acumulado (si disponible)
        bool        alive            { false };
    };

    static constexpr size_t DIAG_MAX_TASKS = 8;

    struct DiagnosticsSnapshot
    {
        // ---------- Uptime / heap ----------
        uint32_t uptimeMs              { 0 };
        uint32_t freeHeapBytes         { 0 };
        uint32_t minFreeHeapBytes      { 0 };
        uint32_t largestBlockBytes     { 0 };

        // ---------- Tasks ----------
        TaskMetric tasks[DIAG_MAX_TASKS] {};
        size_t     taskCount             { 0 };

        // ---------- Dispatcher ----------
        uint32_t dispatched            { 0 };
        uint32_t rejected              { 0 };
        uint32_t dropped               { 0 };

        // ---------- Safety ----------
        uint8_t  safetyState           { 0 };  // SafetyState as u8
        uint16_t activeFaults          { 0 };
        uint16_t latchedFaults         { 0 };
        uint32_t totalEmergencyStops   { 0 };
        uint32_t totalRecoveries       { 0 };

        // ---------- Sensores ----------
        bool     imuHealthy            { false };
        bool     emgHealthy            { false };
        uint32_t imuTotalSamples       { 0 };
        uint32_t emgTotalSamples       { 0 };

        // ---------- Watermark global ----------
        uint32_t minStackHighWater     { 0xFFFFFFFFu };
    };
}