#include "drivers/MPU6050Driver.hpp"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

namespace Drivers
{
    static constexpr const char* TAG = "MPU6050";

    // Registros
    static constexpr uint8_t REG_SMPLRT_DIV   = 0x19;
    static constexpr uint8_t REG_CONFIG       = 0x1A;
    static constexpr uint8_t REG_GYRO_CONFIG  = 0x1B;
    static constexpr uint8_t REG_ACCEL_CONFIG = 0x1C;
    static constexpr uint8_t REG_ACCEL_XOUT_H = 0x3B;
    static constexpr uint8_t REG_PWR_MGMT_1   = 0x6B;
    static constexpr uint8_t REG_WHO_AM_I     = 0x75;

    static constexpr uint8_t WHO_AM_I_VALUE   = 0x68;

    MPU6050Driver::MPU6050Driver(
        HAL::I2CHal& i2cHal
    )
        : i2cHal_(i2cHal)
    {
    }

    bool MPU6050Driver::writeReg(
        uint8_t reg,
        uint8_t value
    )
    {
        return i2cHal_.writeRegister(
            deviceHandle_,
            reg,
            value
        );
    }

    bool MPU6050Driver::readReg(
        uint8_t reg,
        uint8_t& value
    )
    {
        return i2cHal_.readRegister(
            deviceHandle_,
            reg,
            value
        );
    }

    bool MPU6050Driver::initialize(
        const Models::IMUConfig& config
    )
    {
        config_ = config;

        if (!i2cHal_.isInitialized())
        {
            ESP_LOGE(TAG, "I2C bus not initialized");

            return false;
        }

        // Probe antes de registrar el device
        if (!i2cHal_.probe(config_.deviceAddress))
        {
            ESP_LOGE(
                TAG,
                "MPU6050 not found at 0x%02X",
                config_.deviceAddress
            );

            return false;
        }

        if (!i2cHal_.addDevice(
                config_.deviceAddress,
                config_.busFrequencyHz,
                deviceHandle_
            ))
        {
            ESP_LOGE(TAG, "Failed to add device on I2C bus");

            return false;
        }

        // Verificar WHO_AM_I
        uint8_t whoAmI = 0;

        if (!readReg(REG_WHO_AM_I, whoAmI))
        {
            ESP_LOGE(TAG, "WHO_AM_I read failed");

            return false;
        }

        if (whoAmI != WHO_AM_I_VALUE)
        {
            ESP_LOGE(
                TAG,
                "WHO_AM_I mismatch: got 0x%02X, expected 0x%02X",
                whoAmI,
                WHO_AM_I_VALUE
            );

            return false;
        }

        // Wake up: PWR_MGMT_1 = 0x00 (sale de sleep, clock interno).
        // Tras un primer write para salir de sleep, configuramos clock source = PLL X-axis
        // para mayor precisión (0x01).
        if (!writeReg(REG_PWR_MGMT_1, 0x00))
        {
            return false;
        }

        vTaskDelay(pdMS_TO_TICKS(50));

        if (!writeReg(REG_PWR_MGMT_1, 0x01))
        {
            return false;
        }

        // Sample rate divisor
        if (!writeReg(REG_SMPLRT_DIV, config_.sampleRateDiv))
        {
            return false;
        }

        // DLPF (Digital Low-Pass Filter)
        if (!writeReg(REG_CONFIG, static_cast<uint8_t>(config_.dlpf)))
        {
            return false;
        }

        // Gyro full-scale
        const uint8_t gyroCfg =
            static_cast<uint8_t>(
                static_cast<uint8_t>(config_.gyroFs) << 3
            );

        if (!writeReg(REG_GYRO_CONFIG, gyroCfg))
        {
            return false;
        }

        // Accel full-scale
        const uint8_t accelCfg =
            static_cast<uint8_t>(
                static_cast<uint8_t>(config_.accelFs) << 3
            );

        if (!writeReg(REG_ACCEL_CONFIG, accelCfg))
        {
            return false;
        }

        // Calcular factores de escala
        switch (config_.accelFs)
        {
            case Models::AccelFullScale::FS_2G:  accelScale_ = 16384.0f; break;
            case Models::AccelFullScale::FS_4G:  accelScale_ =  8192.0f; break;
            case Models::AccelFullScale::FS_8G:  accelScale_ =  4096.0f; break;
            case Models::AccelFullScale::FS_16G: accelScale_ =  2048.0f; break;
        }

        switch (config_.gyroFs)
        {
            case Models::GyroFullScale::FS_250_DPS:  gyroScale_ = 131.0f; break;
            case Models::GyroFullScale::FS_500_DPS:  gyroScale_ =  65.5f; break;
            case Models::GyroFullScale::FS_1000_DPS: gyroScale_ =  32.8f; break;
            case Models::GyroFullScale::FS_2000_DPS: gyroScale_ =  16.4f; break;
        }

        initialized_ = true;

        ESP_LOGI(TAG, "MPU6050 initialized OK");

        return true;
    }

    bool MPU6050Driver::isPresent() const
    {
        return initialized_;
    }

    bool MPU6050Driver::readRaw(
        MPU6050Raw& outRaw
    )
    {
        if (!initialized_)
        {
            return false;
        }

        uint8_t buffer[14] = { 0 };

        if (!i2cHal_.readBytes(
                deviceHandle_,
                REG_ACCEL_XOUT_H,
                buffer,
                sizeof(buffer)
            ))
        {
            return false;
        }

        outRaw.ax   = static_cast<int16_t>((buffer[0]  << 8) | buffer[1]);
        outRaw.ay   = static_cast<int16_t>((buffer[2]  << 8) | buffer[3]);
        outRaw.az   = static_cast<int16_t>((buffer[4]  << 8) | buffer[5]);
        outRaw.temp = static_cast<int16_t>((buffer[6]  << 8) | buffer[7]);
        outRaw.gx   = static_cast<int16_t>((buffer[8]  << 8) | buffer[9]);
        outRaw.gy   = static_cast<int16_t>((buffer[10] << 8) | buffer[11]);
        outRaw.gz   = static_cast<int16_t>((buffer[12] << 8) | buffer[13]);

        return true;
    }

    void MPU6050Driver::convertToPhysical(
        const MPU6050Raw& raw,
        float& ax_g,
        float& ay_g,
        float& az_g,
        float& gx_dps,
        float& gy_dps,
        float& gz_dps,
        float& tempC
    ) const
    {
        ax_g = static_cast<float>(raw.ax) / accelScale_;
        ay_g = static_cast<float>(raw.ay) / accelScale_;
        az_g = static_cast<float>(raw.az) / accelScale_;

        gx_dps = static_cast<float>(raw.gx) / gyroScale_;
        gy_dps = static_cast<float>(raw.gy) / gyroScale_;
        gz_dps = static_cast<float>(raw.gz) / gyroScale_;

        // Datasheet: TempC = TEMP_OUT/340 + 36.53
        tempC = (static_cast<float>(raw.temp) / 340.0f) + 36.53f;
    }

    void MPU6050Driver::applyOffsets(
        float& ax,
        float& ay,
        float& az,
        float& gx,
        float& gy,
        float& gz
    ) const
    {
        ax -= offsetAx_;
        ay -= offsetAy_;
        az -= offsetAz_;
        gx -= offsetGx_;
        gy -= offsetGy_;
        gz -= offsetGz_;
    }

    void MPU6050Driver::resetCalibration()
    {
        offsetAx_ = offsetAy_ = offsetAz_ = 0.0f;
        offsetGx_ = offsetGy_ = offsetGz_ = 0.0f;
        calibrated_ = false;
    }

    bool MPU6050Driver::calibrate(
        uint16_t samples,
        uint32_t intervalMs
    )
    {
        if (!initialized_)
        {
            return false;
        }

        if (samples == 0)
        {
            return false;
        }

        ESP_LOGI(
            TAG,
            "Starting calibration: %u samples, %lu ms interval",
            samples,
            static_cast<unsigned long>(intervalMs)
        );

        // Reset previo a recalibración
        resetCalibration();

        double sumAx = 0.0;
        double sumAy = 0.0;
        double sumAz = 0.0;
        double sumGx = 0.0;
        double sumGy = 0.0;
        double sumGz = 0.0;

        uint16_t valid = 0;

        for (uint16_t i = 0; i < samples; ++i)
        {
            MPU6050Raw raw {};

            if (!readRaw(raw))
            {
                vTaskDelay(pdMS_TO_TICKS(intervalMs));
                continue;
            }

            float ax, ay, az, gx, gy, gz, t;

            convertToPhysical(raw, ax, ay, az, gx, gy, gz, t);

            sumAx += ax;
            sumAy += ay;
            sumAz += az;
            sumGx += gx;
            sumGy += gy;
            sumGz += gz;

            ++valid;

            vTaskDelay(pdMS_TO_TICKS(intervalMs));
        }

        if (valid < samples / 2)
        {
            ESP_LOGE(
                TAG,
                "Calibration failed: only %u/%u samples valid",
                valid,
                samples
            );

            return false;
        }

        offsetAx_ = static_cast<float>(sumAx / valid);
        offsetAy_ = static_cast<float>(sumAy / valid);
        // En reposo y nivelado, AZ debe medir +1g (gravedad), no offset cero.
        offsetAz_ = static_cast<float>(sumAz / valid) - 1.0f;

        offsetGx_ = static_cast<float>(sumGx / valid);
        offsetGy_ = static_cast<float>(sumGy / valid);
        offsetGz_ = static_cast<float>(sumGz / valid);

        calibrated_ = true;

        ESP_LOGI(
            TAG,
            "Calibration OK | "
            "AO=[%.4f, %.4f, %.4f]g  GO=[%.3f, %.3f, %.3f]dps",
            offsetAx_, offsetAy_, offsetAz_,
            offsetGx_, offsetGy_, offsetGz_
        );

        return true;
    }
}