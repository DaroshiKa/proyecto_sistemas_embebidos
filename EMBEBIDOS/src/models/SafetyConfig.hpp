#pragma once

#include <cstdint>

namespace Models
{
    struct SafetyConfig
    {
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
    };
}