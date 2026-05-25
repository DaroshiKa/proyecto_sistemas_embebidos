#include "hal/I2CHal.hpp"

namespace HAL
{
    bool I2CHal::initialize(
        gpio_num_t sda,
        gpio_num_t scl,
        uint32_t frequency
    )
    {
        i2c_master_bus_config_t config {};

        config.i2c_port = I2C_NUM_0;

        config.sda_io_num = sda;

        config.scl_io_num = scl;

        config.clk_source =
            I2C_CLK_SRC_DEFAULT;

        config.glitch_ignore_cnt = 7;

        return i2c_new_master_bus(
            &config,
            &busHandle_
        ) == ESP_OK;
    }

    bool I2CHal::writeRegister(
        uint8_t deviceAddress,
        uint8_t reg,
        uint8_t value
    )
    {
        uint8_t buffer[2] = { reg, value };

        i2c_device_config_t devConfig {};

        devConfig.dev_addr_length =
            I2C_ADDR_BIT_LEN_7;

        devConfig.device_address =
            deviceAddress;

        devConfig.scl_speed_hz = 400000;

        if (i2c_master_bus_add_device(
                busHandle_,
                &devConfig,
                &deviceHandle_
            ) != ESP_OK)
        {
            return false;
        }

        return i2c_master_transmit(
            deviceHandle_,
            buffer,
            sizeof(buffer),
            -1
        ) == ESP_OK;
    }

    bool I2CHal::readRegister(
        uint8_t deviceAddress,
        uint8_t reg,
        uint8_t* data,
        size_t length
    )
    {
        i2c_device_config_t devConfig {};

        devConfig.dev_addr_length =
            I2C_ADDR_BIT_LEN_7;

        devConfig.device_address =
            deviceAddress;

        devConfig.scl_speed_hz = 400000;

        if (i2c_master_bus_add_device(
                busHandle_,
                &devConfig,
                &deviceHandle_
            ) != ESP_OK)
        {
            return false;
        }

        return i2c_master_transmit_receive(
            deviceHandle_,
            &reg,
            1,
            data,
            length,
            -1
        ) == ESP_OK;
    }
}