#pragma once

#include <cstdint>
<<<<<<< HEAD
#include "MotionTypes.hpp"
=======
>>>>>>> 0f18090e8f7bddf39123306abf466af425290d60

namespace Models
{
    struct SafetyConfig
    {
<<<<<<< HEAD
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
=======
        // ============ Watchdogs de sensores ============
        // Tiempo máximo desde la última muestra válida antes de declarar
        // el sensor como "timeout".
        uint32_t imuTimeoutMs           { 500 };
        uint32_t emgTimeoutMs           { 500 };

        // ============ Lockout post-emergencia ============
        // Tras un EMERGENCY_STOP, durante este tiempo se rechazan TODOS
        // los comandos excepto CLEAR_EMERGENCY.
        uint32_t emergencyLockoutMs     { 5000 };

        // ============ Deadtime entre comandos ============
        // Tiempo mínimo entre dos comandos del mismo tipo y misma fuente.
        // Anti-rebote para EMG/IMU que pueden disparar varias veces seguidas.
        uint32_t commandDeadtimeMs      { 200 };

        // ============ Inactividad de movimiento ============
        // Tras este tiempo sin comandos en estado ACTIVE, vuelve a IDLE.
        uint32_t motionIdleTimeoutMs    { 3000 };

        // ============ Task Watchdog ============
        // Si TaskSafety detecta que una task no hizo heartbeat en este tiempo,
        // publica WATCHDOG_WARNING antes de que el TWDT haga reset.
        // Debe ser menor que CONFIG_ESP_TASK_WDT_TIMEOUT_S (5000 ms).
        uint32_t taskWatchdogWarningMs  { 3000 };

        // ============ Habilitación de comprobaciones ============
        // Permite deshabilitar checks específicos para bring-up/testing.
        bool checkImuTimeout            { true };
        bool checkEmgTimeout            { true };
        bool checkLockout               { true };
        bool checkDeadtime              { true };
        bool checkIdleTimeout           { true };
>>>>>>> 0f18090e8f7bddf39123306abf466af425290d60
    };
}