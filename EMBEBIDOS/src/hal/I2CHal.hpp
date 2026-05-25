#pragma once

#include "driver/i2c_master.h"

namespace HAL
{
    class I2CHal
    {
    public:
        bool initialize(
            gpio_num_t sda,
            gpio_num_t scl,
            uint32_t frequency
        );

        bool writeRegister(
            uint8_t deviceAddress,
            uint8_t reg,
            uint8_t value
        );

        bool readRegister(
            uint8_t deviceAddress,
            uint8_t reg,
            uint8_t* data,
            size_t length
        );

    private:
        i2c_master_bus_handle_t busHandle_ { nullptr };

        i2c_master_dev_handle_t deviceHandle_ { nullptr };
    };
}