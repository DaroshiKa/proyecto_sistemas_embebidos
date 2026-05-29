#pragma once

#include <cstdint>

namespace Models
{
    static constexpr uint16_t CALIBRATION_VERSION = 0x0001;
    static constexpr uint32_t CALIBRATION_MAGIC   = 0x43414C42; // 'CALB'

    struct IMUOffsets
    {
        float ax { 0.0f };
        float ay { 0.0f };
        float az { 0.0f };
        float gx { 0.0f };
        float gy { 0.0f };
        float gz { 0.0f };
    };

    struct EMGCalibration
    {
        float baseline { 0.5f };
        float peakNorm { 0.5f };
    };

    struct CalibrationData
    {
        uint32_t       magic    { CALIBRATION_MAGIC };
        uint16_t       version  { CALIBRATION_VERSION };
        uint16_t       _pad     { 0 };

        IMUOffsets     imu      {};
        EMGCalibration emg      {};

        bool           imuValid { false };
        bool           emgValid { false };
        uint8_t        _pad2[2] { 0, 0 };

        uint32_t       crc      { 0 };
    };
}