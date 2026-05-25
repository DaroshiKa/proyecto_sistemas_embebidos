#pragma once
#include "driver/gpio.h"
#include "driver/uart.h"

namespace HAL
{
    class UARTHal
    {
    public:
        bool initialize(
            uart_port_t port,
            int baudRate,
            gpio_num_t tx,
            gpio_num_t rx
        );

        int write(
            uart_port_t port,
            const uint8_t* data,
            size_t length
        );

        int read(
            uart_port_t port,
            uint8_t* buffer,
            size_t length,
            TickType_t timeout
        );
    };
}