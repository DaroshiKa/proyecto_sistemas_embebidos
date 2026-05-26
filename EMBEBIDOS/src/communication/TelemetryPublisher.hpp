#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include "interfaces/IEventListener.hpp"
#include "interfaces/IEventNotifier.hpp"
#include "interfaces/IDataProvider.hpp"

#include "models/TelemetryFrame.hpp"

namespace Communication
{
    class TelemetryPublisher final :
        public Interfaces::IEventListener,
        public Interfaces::IEventNotifier
    {
    public:
        TelemetryPublisher(
            QueueHandle_t outputQueue,
            Interfaces::IDataProvider* dataProvider = nullptr
        );

        // ----- IEventListener -----
        void onEvent(const Models::EventMessage& event) override;

        // ----- IEventNotifier -----
        void notify(const Models::TelemetryFrame& frame) override;

        // Publicación periódica (llamada por timer o por una task lenta):
        // construye frames TLM_IMU, TLM_EMG, TLM_SERVOS, TLM_SYSTEM a partir
        // del data provider y los encola.
        void publishPeriodic(uint32_t nowMs);

        // Stats
        uint32_t totalPushed() const { return totalPushed_; }
        uint32_t totalDropped() const { return totalDropped_; }

    private:
        bool enqueue(const Models::TelemetryFrame& frame);

        QueueHandle_t              outQueue_;
        Interfaces::IDataProvider* dataProvider_;

        uint32_t totalPushed_  { 0 };
        uint32_t totalDropped_ { 0 };

        // Para no saturar: cada cuántos ms publicamos cada canal
        uint32_t lastImuMs_    { 0 };
        uint32_t lastEmgMs_    { 0 };
        uint32_t lastServosMs_ { 0 };
        uint32_t lastSystemMs_ { 0 };

        static constexpr uint32_t IMU_INTERVAL_MS    = 100;  // 10 Hz
        static constexpr uint32_t EMG_INTERVAL_MS    = 100;  // 10 Hz
        static constexpr uint32_t SERVOS_INTERVAL_MS = 200;  // 5 Hz
        static constexpr uint32_t SYSTEM_INTERVAL_MS = 1000; // 1 Hz
    };
}