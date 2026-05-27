#pragma once

#include <cstdint>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

namespace Services
{
    using WatchdogTriggerCallback = void (*)(void* userArg);

    class WatchdogManager
    {
    public:
        WatchdogManager() = default;

        // Inicializa el TWDT con el timeout indicado.
        // panicOnTrigger: si true, el firmware aborta (sólo en debug).
        // En producción se usa false: el callback decide la respuesta.
        bool initialize(
            uint32_t timeoutMs,
            bool panicOnTrigger,
            WatchdogTriggerCallback cb = nullptr,
            void* userArg = nullptr
        );

        // Subscribir la tarea actual al TWDT.
        // Debe llamarse desde la propia task.
        bool subscribeCurrentTask();

        // Desubscribir la tarea actual.
        bool unsubscribeCurrentTask();

        // Alimentar el watchdog desde la tarea actual.
        // Debe llamarse al menos una vez por timeoutMs/2.
        bool feed();

        bool isInitialized() const { return initialized_; }

        uint32_t timeoutMs() const { return timeoutMs_; }

    private:
        bool       initialized_     { false };
        uint32_t   timeoutMs_       { 3000 };
        bool       panicOnTrigger_  { false };
        WatchdogTriggerCallback cb_ { nullptr };
        void*      userArg_         { nullptr };
    };
}