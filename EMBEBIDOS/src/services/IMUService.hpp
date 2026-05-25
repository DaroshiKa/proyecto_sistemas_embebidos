#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include "interfaces/IService.hpp"
#include "interfaces/IIMUSource.hpp"
#include "interfaces/ICommandDispatcher.hpp"

#include "drivers/MPU6050Driver.hpp"
#include "core/EventBus.hpp"
#include "utils/ComplementaryFilter.hpp"
#include "utils/PlaneDetector.hpp"

#include "models/IMUConfig.hpp"
#include "models/IMUStatus.hpp"
#include "models/SensorData.hpp"

namespace Services
{
    // El IMUService NO conoce servos. Opcionalmente puede emitir
    // MotionCommand a un ICommandDispatcher, pero esa dependencia
    // es inyectada y puede ser nullptr (modo solo-telemetría/testing).
    class IMUService final :
        public Interfaces::IService,
        public Interfaces::IIMUSource
    {
    public:
        IMUService(
            Drivers::MPU6050Driver& driver,
            Core::EventBus& eventBus,
            const Models::IMUConfig& config
        );

        // Inyección opcional del dispatcher para emitir comandos de movimiento
        // derivados de la orientación (ej: detección de plano XY/XZ/YZ).
        void attachCommandDispatcher(
            Interfaces::ICommandDispatcher* dispatcher
        );

        // IService
        bool initialize() override;

        void update() override;

        // IIMUSource
        bool getLatestData(
            Models::IMUData& outData
        ) const override;

        Models::IMUStatus getStatus() const override;

        bool startCalibration() override;

        bool isCalibrated() const override;

    private:
        void publishSensorUpdated();

        void emitMotionCommandIfPlaneChanged(
            Models::OrientationPlane newPlane
        );

        void setState(
            Models::IMUState state
        );

        Drivers::MPU6050Driver&  driver_;
        Core::EventBus&          eventBus_;
        Models::IMUConfig        config_;

        Utils::ComplementaryFilter filter_;
        Utils::PlaneDetector       planeDetector_;

        Interfaces::ICommandDispatcher* dispatcher_ { nullptr };

        // Estado protegido por mutex (lectura desde otras tasks: CLI, Nextion, etc.)
        mutable SemaphoreHandle_t mutex_ { nullptr };

        Models::IMUData   lastData_  {};
        Models::IMUStatus status_    {};
        Models::OrientationPlane lastPublishedPlane_ {
            Models::OrientationPlane::UNKNOWN
        };

        uint32_t lastUpdateTickUs_ { 0 };
    };
}