#include "hal/I2CHal.hpp"

namespace HAL
{
    static constexpr int I2C_TIMEOUT_MS = 50;

    bool I2CHal::initialize(
        gpio_num_t sda,
        gpio_num_t scl,
        uint32_t frequency
    )
    {
        (void)frequency;

        if (busHandle_ != nullptr)
        {
            return true;
        }

        i2c_master_bus_config_t config {};

        config.i2c_port = I2C_NUM_0;
        config.sda_io_num = sda;
        config.scl_io_num = scl;
        config.clk_source = I2C_CLK_SRC_DEFAULT;
        config.glitch_ignore_cnt = 7;
        config.flags.enable_internal_pullup = true;

        return i2c_new_master_bus(
            &config,
            &busHandle_
        ) == ESP_OK;
    }

    bool I2CHal::addDevice(
        uint8_t deviceAddress,
        uint32_t sclSpeedHz,
        i2c_master_dev_handle_t& outHandle
    )
    {
        if (busHandle_ == nullptr)
        {
            return false;
        }

        i2c_device_config_t devConfig {};

        devConfig.dev_addr_length = I2C_ADDR_BIT_LEN_7;
        devConfig.device_address = deviceAddress;
        devConfig.scl_speed_hz = sclSpeedHz;

        return i2c_master_bus_add_device(
            busHandle_,
            &devConfig,
            &outHandle
        ) == ESP_OK;
    }

    bool I2CHal::writeRegister(
        i2c_master_dev_handle_t device,
        uint8_t reg,
        uint8_t value
    )
    {
        if (device == nullptr)
        {
            return false;
        }

        uint8_t buffer[2] = { reg, value };

        return i2c_master_transmit(
            device,
            buffer,
            sizeof(buffer),
            I2C_TIMEOUT_MS
        ) == ESP_OK;
    }

    bool I2CHal::readRegister(
        i2c_master_dev_handle_t device,
        uint8_t reg,
        uint8_t& outValue
    )
    {
        if (device == nullptr)
        {
            return false;
        }

        return i2c_master_transmit_receive(
            device,
            &reg,
            1,
            &outValue,
            1,
            I2C_TIMEOUT_MS
        ) == ESP_OK;
    }

    bool I2CHal::readBytes(
        i2c_master_dev_handle_t device,
        uint8_t startReg,
        uint8_t* buffer,
        size_t length
    )
    {
        if (device == nullptr || buffer == nullptr || length == 0)
        {
            return false;
        }

        return i2c_master_transmit_receive(
            device,
            &startReg,
            1,
            buffer,
            length,
            I2C_TIMEOUT_MS
        ) == ESP_OK;
    }

    bool I2CHal::probe(
        uint8_t deviceAddress
    )
    {
        if (busHandle_ == nullptr)
        {
            return false;
        }

        return i2c_master_probe(
            busHandle_,
            deviceAddress,
            I2C_TIMEOUT_MS
        ) == ESP_OK;
    }
}