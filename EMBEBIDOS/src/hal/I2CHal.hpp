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

        bool addDevice(
            uint8_t deviceAddress,
            uint32_t sclSpeedHz,
            i2c_master_dev_handle_t& outHandle
        );

        bool writeRegister(
            i2c_master_dev_handle_t device,
            uint8_t reg,
            uint8_t value
        );

        bool readRegister(
            i2c_master_dev_handle_t device,
            uint8_t reg,
            uint8_t& outValue
        );

        bool readBytes(
            i2c_master_dev_handle_t device,
            uint8_t startReg,
            uint8_t* buffer,
            size_t length
        );

        bool probe(
            uint8_t deviceAddress
        );

        bool isInitialized() const
        {
            return busHandle_ != nullptr;
        }

    private:
        i2c_master_bus_handle_t busHandle_ { nullptr };
    };
}