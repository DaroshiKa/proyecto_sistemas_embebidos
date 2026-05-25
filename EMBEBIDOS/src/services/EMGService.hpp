#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include "interfaces/IService.hpp"
#include "interfaces/IEMGSource.hpp"
#include "interfaces/ICommandDispatcher.hpp"

#include "drivers/EMGDriver.hpp"

#include "core/EventBus.hpp"

#include "utils/EMGProcessor.hpp"
#include "utils/ThresholdDetector.hpp"
#include "utils/GestureFSM.hpp"

#include "models/EMGConfig.hpp"
#include "models/EMGStatus.hpp"
#include "models/SensorData.hpp"

namespace Services
{
    class EMGService final :
        public Interfaces::IService,
        public Interfaces::IEMGSource
    {
    public:
        EMGService(
            Drivers::EMGDriver& driver,
            Core::EventBus& eventBus,
            const Models::EMGConfig& config
        );

        void attachCommandDispatcher(
            Interfaces::ICommandDispatcher* dispatcher
        );

        // IService
        bool initialize() override;
        void update() override;

        // IEMGSource
        bool getLatestData(Models::EMGData& out) const override;
        Models::EMGStatus getStatus() const override;
        bool startCalibration() override;
        bool isCalibrated() const override;

        // Reconfiguración runtime (CLI / Nextion)
        void setThresholds(float onLevel, float offLevel);

    private:
        void processOneSample(uint16_t raw, uint32_t nowMs);
        void publishSensorUpdated();
        void emitMotionCommandIfGesture(Models::EMGGesture gesture, uint32_t nowMs);
        void setState(Models::EMGState state);

        Drivers::EMGDriver&        driver_;
        Core::EventBus&            eventBus_;
        Models::EMGConfig          config_;

        Utils::EMGProcessor        processor_;
        Utils::ThresholdDetector   detector_;
        Utils::GestureFSM          gestureFsm_;

        Interfaces::ICommandDispatcher* dispatcher_ { nullptr };

        mutable SemaphoreHandle_t  mutex_ { nullptr };
        Models::EMGData            lastData_ {};
        Models::EMGStatus          status_   {};
    };
}