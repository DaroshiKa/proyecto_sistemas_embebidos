#pragma once

#include <cstdint>
#include "driver/gpio.h"

namespace Models
{
    enum class GyroFullScale : uint8_t
    {
        FS_250_DPS  = 0,    // ±250 °/s   - 131 LSB/(°/s)
        FS_500_DPS  = 1,
        FS_1000_DPS = 2,
        FS_2000_DPS = 3
    };

    enum class AccelFullScale : uint8_t
    {
        FS_2G  = 0,         // ±2g  - 16384 LSB/g
        FS_4G  = 1,
        FS_8G  = 2,
        FS_16G = 3
    };

    enum class DLPFMode : uint8_t
    {
        BW_260HZ = 0,
        BW_184HZ = 1,
        BW_94HZ  = 2,
        BW_44HZ  = 3,       // recomendado para robótica humana
        BW_21HZ  = 4,
        BW_10HZ  = 5,
        BW_5HZ   = 6
    };

    struct IMUConfig
    {
        // Bus I2C
        gpio_num_t   sdaPin          { GPIO_NUM_21 };
        gpio_num_t   sclPin          { GPIO_NUM_22 };
        uint32_t     busFrequencyHz  { 400000 };
        uint8_t      deviceAddress   { 0x68 };

        // Sensibilidades
        GyroFullScale  gyroFs        { GyroFullScale::FS_250_DPS };
        AccelFullScale accelFs       { AccelFullScale::FS_2G };
        DLPFMode       dlpf          { DLPFMode::BW_44HZ };
        uint8_t        sampleRateDiv { 9 };    // 1kHz / (1+9) = 100 Hz

        // Filtro complementario
        float       complementaryAlpha { 0.98f };

        // Calibración
        uint16_t    calibrationSamples { 1000 };  // 10 s a 100 Hz

        // Detección de plano
        float       planeThresholdG    { 0.7f };  // |a| dominante > 0.7g
        float       planeHysteresisG   { 0.1f };

        // Watchdog del sensor
        uint32_t    timeoutMs          { 200 };   // sin datos válidos
        uint8_t     maxConsecutiveFails { 5 };
    };
}