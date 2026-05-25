#include "hal/UARTHal.hpp"

namespace HAL
{
    bool UARTHal::initialize(
        uart_port_t port,
        int baudRate,
        gpio_num_t tx,
        gpio_num_t rx
    )
    {
        uart_config_t config {};

        config.baud_rate = baudRate;

        config.data_bits =
            UART_DATA_8_BITS;

        config.parity =
            UART_PARITY_DISABLE;

        config.stop_bits =
            UART_STOP_BITS_1;

        config.flow_ctrl =
            UART_HW_FLOWCTRL_DISABLE;

        config.source_clk =
            UART_SCLK_DEFAULT;

        if (uart_driver_install(
                port,
                2048,
                2048,
                0,
                nullptr,
                0
            ) != ESP_OK)
        {
            return false;
        }

        if (uart_param_config(
                port,
                &config
            ) != ESP_OK)
        {
            return false;
        }

        return uart_set_pin(
            port,
            tx,
            rx,
            UART_PIN_NO_CHANGE,
            UART_PIN_NO_CHANGE
        ) == ESP_OK;
    }

    int UARTHal::write(
        uart_port_t port,
        const uint8_t* data,
        size_t length
    )
    {
        return uart_write_bytes(
            port,
            reinterpret_cast<const char*>(data),
            length
        );
    }

    int UARTHal::read(
        uart_port_t port,
        uint8_t* buffer,
        size_t length,
        TickType_t timeout
    )
    {
        return uart_read_bytes(
            port,
            buffer,
            length,
            timeout
        );
    }
}