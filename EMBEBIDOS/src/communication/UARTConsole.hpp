#pragma once

#include <cstdarg>
#include <cstdint>
#include <cstddef>

#include "driver/uart.h"

#include "hal/UARTHal.hpp"

namespace Communication
{
    class UARTConsole
    {
    public:
        static constexpr size_t LINE_BUFFER_SIZE = 128;

        UARTConsole(
            HAL::UARTHal& uartHal,
            uart_port_t port = UART_NUM_0
        );

        // El bus se asume ya inicializado externamente.
        // Si quieres que la consola lo inicialice, llamar a `bind()`.
        bool bind(int baudRate, gpio_num_t tx, gpio_num_t rx);

        // Escribe una línea ASCII (sin agregar \n; el caller lo controla).
        void write(const char* s);
        void writeLine(const char* s);
        void printf(const char* fmt, ...);

        // Lee bytes del UART hasta encontrar '\n' o llenar `LINE_BUFFER_SIZE`.
        // Retorna true cuando una línea completa está disponible en `outLine`.
        // Llamar repetidamente; mantiene estado interno entre llamadas.
        bool readLine(
            char* outLine,
            size_t outLineSize,
            uint32_t timeoutMs
        );

    private:
        HAL::UARTHal&   uart_;
        uart_port_t     port_;
        char            lineBuf_[LINE_BUFFER_SIZE] {};
        size_t          lineIdx_ { 0 };
    };
}