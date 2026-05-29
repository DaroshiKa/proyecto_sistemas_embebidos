#pragma once

#include "driver/i2c_master.h"

#include "hal/I2CHal.hpp"
#include "models/IMUConfig.hpp"
#include "models/SensorData.hpp"

namespace Drivers
{
    struct MPU6050Raw
    {
        int16_t ax;
        int16_t ay;
        int16_t az;
        int16_t temp;
        int16_t gx;
        int16_t gy;
        int16_t gz;
    };

    class MPU6050Driver
    {
    public:
        explicit MPU6050Driver(
            HAL::I2CHal& i2cHal
        );

        // Aplica la configuración del chip y registra el device en el bus.
        // El bus debe haberse inicializado externamente (responsabilidad
        // del bootstrap del sistema, no del driver).
        bool initialize(
            const Models::IMUConfig& config
        );

        bool isPresent() const;

        // Lectura cruda atómica de los 14 bytes (AX..GZ).
        bool readRaw(
            MPU6050Raw& outRaw
        );

        // Convierte raw a magnitudes físicas usando los FS configurados.
        // No aplica offsets de calibración (eso es del calibrador).
        void convertToPhysical(
            const MPU6050Raw& raw,
            float& ax_g,
            float& ay_g,
            float& az_g,
            float& gx_dps,
            float& gy_dps,
            float& gz_dps,
            float& tempC
        ) const;

        // Calibración bloqueante: promedia N muestras en reposo y guarda offsets.
        // El usuario debe garantizar que el dispositivo está inmóvil y nivelado.
        bool calibrate(
            uint16_t samples,
            uint32_t intervalMs = 5
        );

        bool isCalibrated() const { return calibrated_; }

        void resetCalibration();

        // Aplica offsets in-place sobre lecturas físicas.
        void applyOffsets(
            float& ax,
            float& ay,
            float& az,
            float& gx,
            float& gy,
            float& gz
        ) const;

        // Inyecta offsets calibrados desde una fuente externa (ej: NVS).
        // No realiza muestreo; sólo guarda y marca calibrated = true.
        void setManualOffsets(
            float ax, float ay, float az,
            float gx, float gy, float gz
        );

        void getOffsets(
            float& ax, float& ay, float& az,
            float& gx, float& gy, float& gz
        ) const;

    private:
        bool writeReg(
            uint8_t reg,
            uint8_t value
        );

        bool readReg(
            uint8_t reg,
            uint8_t& value
        );

        HAL::I2CHal& i2cHal_;
        i2c_master_dev_handle_t deviceHandle_ { nullptr };

        Models::IMUConfig config_ {};

        // Factores de escala (calculados en initialize según FS)
        float accelScale_ { 16384.0f };  // LSB / g
        float gyroScale_  { 131.0f };    // LSB / (°/s)

        // Offsets de calibración (en unidades físicas)
        float offsetAx_ { 0.0f };
        float offsetAy_ { 0.0f };
        float offsetAz_ { 0.0f };
        float offsetGx_ { 0.0f };
        float offsetGy_ { 0.0f };
        float offsetGz_ { 0.0f };

        bool initialized_ { false };
        bool calibrated_  { false };
    };
}